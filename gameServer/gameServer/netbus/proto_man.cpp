#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proto_man.h"
#include "google/protobuf/message.h"

#define MAX_PF_MAP_SIZE 1024
#define CMD_HEADER 8

static int g_proto_type = PROTO_BUF;
static char* g_pf_map[MAX_PF_MAP_SIZE];
static int g_cmd_count = 0;

//初始化
void proto_man::init(int proto_type){

}

//获得当前类型
int proto_man::proto_type(){
	return g_proto_type;
}

//pf的相关操作
void proto_man::register_pf_cmd_map(char** pf_map, int len){
	len = (MAX_PF_MAP_SIZE - g_cmd_count) < len ? ((MAX_PF_MAP_SIZE - g_cmd_count)) : len;
	for (int i = 0; i < len;i++)
	{
		g_pf_map[g_cmd_count + i] = strdup(pf_map[i]);
	}
	g_cmd_count += len;
}

//创建protobuf
static google::protobuf::Message* create_message(const char* type_name){
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor =
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);

	if (descriptor) {
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			message = prototype->New();
		}
	}
	return message;
}

static void release_message(google::protobuf::Message* m){
	delete m;
}

// stype(2) stype(2) utag(4) body
bool proto_man::decode_cmd_msg(unsigned char* cmd, int cmd_len, struct cmd_msg** ouy_msg){
	return true;
}

//清楚命令行
void proto_man::cmd_msg_free(struct cmd_msg* msg){
	if (msg->body){
		if (g_proto_type == PROTO_JSON){
			free(msg->body);
			msg->body = NULL;
		}
		else{
			//google的相关
			google::protobuf::Message* p_m = (google::protobuf::Message*)msg->body;
			delete p_m;
			msg->body = NULL;
			//end
		}
	}
	free(msg);
}

unsigned char* proto_man::encode_msg_to_raw(const struct cmd_msg* msg, int* out_len){
	//长度
	int raw_len = 0;
	unsigned char* raw_data = NULL;
	//给这个指针对应的对象赋值
	*out_len = 0;
	if (g_proto_type == PROTO_JSON){
		char* json_str = (char*)msg->body;//把body解析出来
		int len = strlen(json_str) + 1;
		raw_data = (unsigned char*)malloc(CMD_HEADER + len);
		memcpy(raw_data + CMD_HEADER, json_str, len - 1);
		raw_data[8 + len] = 0;
		*out_len = (len + CMD_HEADER);
	}
	else{
		google::protobuf::Message* p_m = (google::protobuf::Message*)msg->body;
		int pf_len = p_m->ByteSize();
		raw_data = (unsigned char*)malloc(CMD_HEADER + pf_len);
		if (!p_m->SerializePartialToArray(raw_data + CMD_HEADER, pf_len)) {
			free(raw_data);
			return NULL;
		}
		*out_len = (pf_len + CMD_HEADER);
	}

	//数据的编写出来
	raw_data[0] = (msg->stype & 0x000000ff);
	raw_data[1] = ((msg->stype & 0x0000ff00) >> 8);
	raw_data[2] = (msg->ctype & 0x000000ff);
	raw_data[3] = ((msg->ctype & 0x0000ff00) >> 8);
	memcpy(raw_data + 4, &msg->utag, 4);
	//end
	return raw_data;
}

void proto_man::msg_raw_free(unsigned char* raw){
	free(raw);
}


