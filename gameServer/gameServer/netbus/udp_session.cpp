#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"
#include "session.h"
#include "udp_session.h"
#include "proto_man.h"

void udp_session::close() {

}

/// <summary>
/// udp接受数据
/// </summary>
/// <param name="req"></param>
/// <param name="status"></param>
static void on_uv_udp_send_end(uv_udp_send_t* req,int status) {
	if (status == 0) {
		printf("send sucess\n");
	}
	free(req);
}

/// <summary>
/// udp发送数据部分
/// </summary>
/// <param name="body"></param>
/// <param name="len"></param>
void udp_session::send_data(unsigned char* body, int len) {
	uv_buf_t w_buf;
	w_buf = uv_buf_init((char*)body, len);
	uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
	uv_udp_send(req, this->udp_handler, &w_buf, 1, this->addr, on_uv_udp_send_end);
}

/// <summary>
/// 获得客户端的地址
/// </summary>
/// <param name="port"></param>
/// <returns></returns>
const char* udp_session::get_address(int* port) {
	*port = this->c_port;
	return this->c_address;
}

/// <summary>
/// 发送数据
/// </summary>
/// <param name="msg"></param>
void udp_session::send_msg(struct cmd_msg* msg) {
	unsigned char* encode_pkg = NULL;
	int encode_len = 0;
	encode_pkg = proto_man::encode_msg_to_raw(msg, &encode_len);
	if (encode_pkg) {
		this->send_data(encode_pkg, encode_len);
		proto_man::msg_raw_free(encode_pkg);
	}
}

