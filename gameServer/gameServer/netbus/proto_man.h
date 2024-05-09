#ifndef __PROTO_MAN_H__
#define __PROTO_MAN_H__

/**
 * 数据的编译和使用相关
 */
enum {
	PROTO_JSON = 0,
	PROTO_BUF = 1,
};

//命令的msg
struct cmd_msg{
	int stype;
	int ctype;
	unsigned int utag;//用户的id
	void* body; //如果是json的话就是body，如果是二进制的话就是对应的字符串数组
};

class proto_man {

public:
	//初始化现在要使用的是那个操作
	static void init(int proto_type);
	static void register_pf_cmd_map(char** pf_map, int len);
	static int proto_type();
	static bool decode_cmd_msg(unsigned char* cmd, int cmd_len, struct cmd_msg** ouy_msg);
	static void cmd_msg_free(struct cmd_msg* msg);
	static unsigned char* encode_msg_to_raw(const struct cmd_msg* msg, int* out_len);
	static void msg_raw_free(unsigned char* raw);
};




#endif // !__PROTO_MAN_H__
