#ifndef __SESSION_UV_H__
#define __SESSION_UV_H__

#define RECV_LEN 4096//��ȡ���ڴ泤��

//׼��ֻ��json����
enum 
{
	TCP_SOCKET, //ʹ��tcp��������
	WS_SOCKET	//��ҳ�˵�websocket����ز���
};

class uv_session :session{

public:
	uv_tcp_t tcp_handler;
	char c_address[32];		//��ַ�Ĵ洢��ַ
	int c_port;

	uv_shutdown_t shutdown;	//�رյ�Ӧ��
	uv_write_t w_req;		//д����ز���
	uv_buf_t w_buf;			//д�����ݵ���ض�����
public:
	char recv_buf[RECV_LEN];//������ݵĶ�����buf
	int recved;
	int socket_type;		//socket������������ֻ��һ������json����
private:
	void init();			//��ʼ��session
	void exit();			//�˳�

public:
	static uv_session* create();//����һ��session����
	static void destroy(uv_session* s);		//�������ݵ�һЩ����

public:
	virtual void close();
	virtual void send_data(unsigned char* body, int len);
	virtual const char* get_address(int* client_port);
};

#endif
