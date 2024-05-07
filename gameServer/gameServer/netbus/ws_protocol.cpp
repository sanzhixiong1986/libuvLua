#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "../3rd/http_parser/http_parser.h"
#include "../3rd/crypto/base64_encoder.h"
#include "../3rd/crypto/sha1.h"

#include "session.h"
#include "ws_protocol.h"
#include "../utils/cache_alloc.h"

extern cache_allocer* wbuf_allocer; //昨天出现了一个问题，导出的时候没有加上=nullprt就会报错

static char* wb_migic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//base64(sha1(key+web_migic))生成
//websocket的头部文件生成
static char* wb_accept = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade:websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n"
"WebSocket-Protocol:chat\r\n\r\n";

static char filed_sec_key[512];
static char value_sec_key[512];
static int is_sec_key = 0;
static int has_sec_key = 0;
static int is_shaker_ended = 0;

extern "C" {
	int on_message_end(http_parser*p){
		is_shaker_ended = 1;
		return 0;
	}
}

//对应的接受事件
static int on_ws_header_field(http_parser* p, const char *at, size_t length){
	if (strncmp(at, "Sec-WebSocket-Key", length) == 0){
		is_sec_key = 1;
	}
	else{
		is_sec_key = 0;
	}
	return 0;
}

static int on_ws_header_value(http_parser* p, const char *at, size_t length){
	if (!is_sec_key){
		return 0;
	}
	strncpy(value_sec_key, at, length);
	value_sec_key[length] = 0;//给一个结束的操作
	has_sec_key = 1;
	return 0;
}

//重点 数据的相关操作
bool ws_protocol::ws_shake_hand(session* s, char* body, int len){
	//利用http进行创建对象
	http_parser_settings settings;
	http_parser_settings_init(&settings);
	//end

	settings.on_header_field = on_ws_header_field;
	settings.on_header_value = on_ws_header_value;
	settings.on_message_complete = on_message_end;
	http_parser p;
	http_parser_init(&p, HTTP_REQUEST); //初始化http的操作
	//所有的数据都清空
	is_sec_key = 0;
	has_sec_key = 0;
	is_shaker_ended = 0;

	printf("body=%s\n",body);
	http_parser_execute(&p, &settings, body, len); //执行方法
	
	//判断数据的正确定
	if (has_sec_key && is_shaker_ended){
		printf("Sec-WebSocket-Key: %s\n", value_sec_key);
		//key+migic
		static char key_migic[512];
		static char shal_key_migic[SHA1_DIGEST_SIZE];
		static char send_client[512];

		int sha1_size;
		sprintf(key_migic, "%s%s", value_sec_key, wb_migic);
		crypt_sha1((unsigned char*)key_migic, strlen(key_migic), (unsigned char*)&shal_key_migic, &sha1_size);
		
		int base64_len;
		char* base_buf = base64_encode((uint8_t*)shal_key_migic, sha1_size, &base64_len);
		sprintf(send_client, wb_accept, base_buf);
		base64_encode_free(base_buf);

		s->send_data((unsigned char*)send_client, strlen(send_client));
		return true;
	}
	return false;
}

//重点头的组成操作
bool ws_protocol::read_ws_header(unsigned char* recv_data, int recv_len, int* pkg_size, int* out_header_size){
	if (recv_data[0] != 0x81 && recv_data[0] != 0x82){
		return false;
	}

	//数据长度小于2，连头文件都不到，肯定是失败的
	if (recv_len < 2){
		return false;
	}

	unsigned int data_len = recv_data[1] & 0x0000007f;
	int head_size = 2;
	if (data_len == 126){
		head_size += 2;
		if (recv_len < head_size){
			return false;
		}
		data_len = recv_data[3] | (recv_data[2] << 8);
	}
	else if (data_len == 127)
	{
		head_size += 8;
		if (recv_len < head_size){
			return false;
		}

		unsigned int low = recv_data[5] | (recv_data[4] << 8) | (recv_data[3] << 16) | (recv_data[2] << 24);
		unsigned int hight = recv_data[9] | (recv_data[8] << 8) | (recv_data[7] << 16) | (recv_data[6] << 24);

		data_len = low;
	}

	head_size += 4;
	*pkg_size = data_len + head_size;
	*out_header_size = head_size;
	
	return true;
}

//接受数据
void ws_protocol::parser_ws_recv_data(unsigned char* rew_data, unsigned char* mask, int rew_len){
	for (int i = 0; i < rew_len; i++){
		rew_data[i] = rew_data[i] ^ mask[i % 4];
	}
}

//发送数据
unsigned char* ws_protocol::package_ws_send_data(const unsigned char* raw_data, int len, int* ws_data_len){
	//头字节的大小进行操作
	int head_size = 2;
	if (len > 125 && len < 65536){
		head_size += 2;
	}
	else if (len >= 65536){
		head_size += 8;
		return NULL;
	}
	//cache malloc
	//unsigned char* data_buf = (unsigned char*)malloc(head_size + len);
	unsigned char* data_buf = (unsigned char*)cache_alloc(wbuf_allocer, head_size + len);
	data_buf[0] = 0x81;
	if (len <= 125){
		data_buf[1] = len;
	}else if (len>25&&len<65536)
	{
		data_buf[1] = 126;
		data_buf[2] = (len & 0x0000ff00) >> 8;
		data_buf[3] = (len & 0x000000ff);
	}
	memset(data_buf + head_size, (int)raw_data, len);
	*ws_data_len = (head_size + len);
	return data_buf;
}

//清理发送后的数据
void ws_protocol::free_ws_send_pkg(unsigned char* ws_pkg){
	cache_free(wbuf_allocer, ws_pkg);
}