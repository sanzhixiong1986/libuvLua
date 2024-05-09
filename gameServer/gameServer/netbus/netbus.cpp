#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"
#include "session.h"
#include "uv_session.h"

#include "netbus.h"

#include "ws_protocol.h"
#include "tp_protocol.h"
#include "proto_man.h"
#include "service_man.h"

extern "C"{

	//��ÿͻ��˵�����
	static void on_recv_client_cmd(uv_session* s, unsigned char* body, int len){
		printf("client command !!!!\n");
		//test
		//websocket ��Ŷ�Ǹ���Ϣ�� s->send_data(body, len);
		struct cmd_msg* msg = NULL;
		if (proto_man::decode_cmd_msg(body, len, &msg)) {
			if (!service_man::on_recv_cmd_msg((session*)s, msg)) {
				s->close();
			}
			proto_man::cmd_msg_free(msg);
		}
		//end
	}

	//tcp�������ݵú���
	static void on_recv_tcp_data(uv_session* s){
		unsigned char* pkg_data = (unsigned char*)((s->long_pkg != NULL) ? s->long_pkg : s->recv_buf);

		while (s->recved > 0)
		{
			int pkg_size = 0;
			int head_size = 0;

			if (!tp_protocol::read_header(pkg_data, s->recved, &pkg_size, &head_size)){
				break;
			}

			if (s->recved < pkg_size){
				break;
			}

			unsigned char* raw_data = pkg_data + head_size;
			on_recv_client_cmd(s, raw_data, pkg_size - head_size);

			if (s->recved > pkg_size){
				memmove(pkg_data, pkg_data + pkg_size, s->recved - pkg_size);
			}
			s->recved -= pkg_size;

			if (s->recved == 0 && s->long_pkg != NULL){
				free(s->long_pkg);
				s->long_pkg = NULL;
				s->long_pkg_size = 0;
			}
		}
	}

	//ws��������ݻ�ȡ
	static void on_recv_ws_data(uv_session* s){
		unsigned char* pkg_data = (unsigned char*)((s->long_pkg != NULL) ? s->long_pkg : s->recv_buf);
		while (s->recved > 0)
		{
			int pkg_size = 0;
			int head_size = 0;

			if (pkg_data[0] == 0x88){
				s->close();
				break;
			}

			if (!ws_protocol::read_ws_header(pkg_data, s->recved, &pkg_size, &head_size)){
				break;
			}

			if (s->recved < pkg_size){
				break;
			}

			unsigned char* raw_data = pkg_data + head_size;
			unsigned char* mask = raw_data - 4;
			ws_protocol::parser_ws_recv_data(raw_data, mask, pkg_size - head_size);
			on_recv_client_cmd(s, raw_data, pkg_size - head_size);

			if (s->recved > pkg_size){
				memmove(pkg_data, pkg_data + pkg_size, s->recved - pkg_size);
			}
			s->recved -= pkg_size;

			if (s->recved == 0 && s->long_pkg != NULL){
				free(s->long_pkg);
				s->long_pkg = NULL;
				s->long_pkg_size = 0;
			}
		}
	}

	//�������ݵĻ���
	static void uv_alloc_buf(uv_handle_t* handler, size_t suggested, uv_buf_t* buf){
		uv_session* s = (uv_session*)handler->data;

		if (s->recved < RECV_LEN){
			*buf = uv_buf_init(s->recv_buf + s->recved, RECV_LEN - s->recved);
		}
		else{
			if (s->long_pkg == NULL){
				if (s->socket_type == WS_SOCKET && s->is_ws_shake){
					int pkg_size;
					int head_size;
					ws_protocol::read_ws_header((unsigned char*)s->recv_buf, s->recved, &pkg_size, &head_size);
					s->long_pkg_size = pkg_size;
					s->long_pkg = (char*)malloc(pkg_size);
					memcpy(s->long_pkg, s->recv_buf, s->recved);
				}
				else{
					//tcp
				}
			}
		}
	}

	//���׹ر�
	static void on_close(uv_handle_t* handle){
		uv_session* s = (uv_session*)handle->data;
		uv_session::destroy(s);
	}

	//�ر�
	static void on_shutdown(uv_shutdown_t* req, int status){
		uv_close((uv_handle_t*)req->handle, on_close);
	}

	//��ȡ��Ӧ������
	static void after_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf){
		//�����������
		uv_session* s = (uv_session*)stream->data;
		if (nread < 0){
			//5.7���޸�
			s->close();
			return;
		}

		//websocket�ò���
		s->recved += nread;
		if (s->socket_type == WS_SOCKET){
			if (s->is_ws_shake == 0){
				if (ws_protocol::ws_shake_hand((session*)s, s->recv_buf, s->recved)){
					s->is_ws_shake = 1;
					s->recved = 0;
				}
			}
			else{
				on_recv_ws_data(s);
			}
		}
		else{
			//tcp
			on_recv_tcp_data(s);
		}
		//end
	}

	//���ӳɹ�
	static void uv_connection(uv_stream_t* server, int status){
		//��������
		uv_session* s = uv_session::create();
		//��ÿͻ��˵��¼�
		uv_tcp_t* client = &s->tcp_handler;
		memset(client, 0, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client);
		client->data = (void*)s;
		uv_accept(server, (uv_stream_t*)client);
		//��ַ
		struct sockaddr_in addr;
		int len = sizeof(addr);
		uv_tcp_getpeername(client, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, (char*)s->c_address, 64);
		s->c_port = ntohs(addr.sin_port);
		s->socket_type = (int)(server->data);
		printf("new client comming %s:%d\n", s->c_address, s->c_port);
		uv_read_start((uv_stream_t*)client, uv_alloc_buf, after_read);
	}
}

static netbus g_netbus;
netbus* netbus::instance() {
	return &g_netbus;
}

/************************************************************************/
/* ����tcp�ķ����� ���� port Ϊ�˿�                                     */
/************************************************************************/
void netbus::start_tcp_server(int port) {
	printf("start_tcp_server\n");
	//����tcp�������Ļ���
	uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));//����tcp��
	memset(listen, 0, sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), listen);
	//end

	//�������ص�
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	//end

	//�󶨿ͻ���
	int ret = uv_tcp_bind(listen, (const struct sockaddr*)&addr, 0);
	if (ret != 0){
		printf("bind is error\n");
		free(listen);
		return;
	}

	uv_listen((uv_stream_t*)listen,SOMAXCONN,uv_connection);
	listen->data = (void*)TCP_SOCKET;
}

//����websocket
void netbus::start_ws_server(int port){
	printf("����websocket\n");
	uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(listen, 0, sizeof(uv_tcp_t));

	uv_tcp_init(uv_default_loop(), listen);
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);

	int ret = uv_tcp_bind(listen, (const struct sockaddr*)&addr, 0);
	if (ret != 0){
		printf("bind error\n");
		free(listen);
		return;
	}

	uv_listen((uv_stream_t*)listen, SOMAXCONN, uv_connection);
	listen->data = (void*)WS_SOCKET;
}

//����uvĬ��ѭ���ķ���
void netbus::run() {
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

//��ʼ���ķ���
void netbus::init(){
	printf("��ʼ���ڴ����\n");
	service_man::init();
	init_session_allocer();
}