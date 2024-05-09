#ifndef __SESSION_H__
#define __SESSION_H__

/************************************************************************/
/* 抽象类                                                                     */
/************************************************************************/
class session {
public:
	virtual void close() = 0;//抽象函数
	virtual void send_data(unsigned char* body, int len) = 0;//发送数据函数
	virtual const char* get_address(int* client_port) = 0;//获得当前的地址
	virtual void send_msg(struct cmd_msg* msg) = 0; //发送数据的方法
};

#endif // !__SESSION_H__
