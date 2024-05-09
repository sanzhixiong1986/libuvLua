#ifndef __SESSION_H__
#define __SESSION_H__

/************************************************************************/
/* ������                                                                     */
/************************************************************************/
class session {
public:
	virtual void close() = 0;//������
	virtual void send_data(unsigned char* body, int len) = 0;//�������ݺ���
	virtual const char* get_address(int* client_port) = 0;//��õ�ǰ�ĵ�ַ
	virtual void send_msg(struct cmd_msg* msg) = 0; //�������ݵķ���
};

#endif // !__SESSION_H__
