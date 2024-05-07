#ifndef __SESSION_UV_H__
#define __SESSION_UV_H__

#define RECV_LEN 4096//读取的内存长度

//准备只做json操作
enum 
{
	TCP_SOCKET, //使用tcp制作操作
	WS_SOCKET	//网页端的websocket的相关操作
};

class uv_session :session{

public:
	uv_tcp_t tcp_handler;
	char c_address[32];		//地址的存储地址
	int c_port;

	//2024.5.6修改对象的属性删除原来的属性，添加了两个新的属性
	//_shutdown_t shutdown;	//关闭的应用
	//_write_t w_req;		//写的相关操作
	//_buf_t w_buf;			//写入数据的相关二进制
	uv_shutdown_t shutdown;
	bool is_shutdown;
public:
	char recv_buf[RECV_LEN];//获得数据的二进制buf
	int recved;				//当前的数据操作
	int socket_type;		//socket的类型我现在只有一个类型json类型
	//5.7添加
	char* long_pkg;
	int long_pkg_size;
	//end
public:
	//5.7
	int is_ws_shake;
	//end
private:
	void init();			//初始化session
	void exit();			//退出

public:
	static uv_session* create();//创建一个session对象
	static void destroy(uv_session* s);		//清理数据的一些操作

public:
	virtual void close();
	virtual void send_data(unsigned char* body, int len);
	virtual const char* get_address(int* client_port);
};

//2024.5.6新添加一个初始化的方法
void init_session_allocer();

#endif
