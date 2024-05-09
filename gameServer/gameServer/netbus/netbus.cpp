#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"
#include "session.h"
#include "uv_session.h"
#include "udp_session.h"

#include "netbus.h"

#include "ws_protocol.h"
#include "tp_protocol.h"
#include "proto_man.h"
#include "service_man.h"

extern "C"{

	//获得客户端得数据
	static void on_recv_client_cmd(session* s, unsigned char* body, int len){
		printf("client command !!!!\n");
		//test
		//websocket 发哦那个消息的 s->send_data(body, len);
		struct cmd_msg* msg = NULL;
		if (proto_man::decode_cmd_msg(body, len, &msg)) {
			if (!service_man::on_recv_cmd_msg((session*)s, msg)) {
				s->close();
			}
			proto_man::cmd_msg_free(msg);
		}
		//end
	}

	//tcp接受数据得函数
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
			on_recv_client_cmd((session*)s, raw_data, pkg_size - head_size);

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

	//ws得相关数据获取
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
			on_recv_client_cmd((session*)s, raw_data, pkg_size - head_size);

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

	//创建数据的缓存
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

	//彻底关闭
	static void on_close(uv_handle_t* handle){
		uv_session* s = (uv_session*)handle->data;
		uv_session::destroy(s);
	}

	//关闭
	static void on_shutdown(uv_shutdown_t* req, int status){
		uv_close((uv_handle_t*)req->handle, on_close);
	}

	//读取对应的数据
	static void after_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf){
		//创建缓存对象
		uv_session* s = (uv_session*)stream->data;
		if (nread < 0){
			//5.7号修改
			s->close();
			return;
		}

		//websocket得操作
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

	//链接成功
	static void uv_connection(uv_stream_t* server, int status){
		//创建对象
		uv_session* s = uv_session::create();
		//获得客户端的事件
		uv_tcp_t* client = &s->tcp_handler;
		memset(client, 0, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client);
		client->data = (void*)s;
		uv_accept(server, (uv_stream_t*)client);
		//地址
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
/* 启动tcp的服务器 参数 port 为端口                                     */
/************************************************************************/
void netbus::start_tcp_server(int port) {
	printf("start_tcp_server\n");
	//创建tcp服务器的基础
	uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));//创建tcp的
	memset(listen, 0, sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), listen);
	//end

	//网络的相关的
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	//end

	//绑定客户端
	int ret = uv_tcp_bind(listen, (const struct sockaddr*)&addr, 0);
	if (ret != 0){
		printf("bind is error\n");
		free(listen);
		return;
	}

	uv_listen((uv_stream_t*)listen,SOMAXCONN,uv_connection);
	listen->data = (void*)TCP_SOCKET;
}

//启动websocket
void netbus::start_ws_server(int port){
	printf("启动websocket\n");
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

//udp的信息结构
struct udp_recv_buf
{
	char* recv_buf;
	size_t max_recv_len;
};

static void udp_uv_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	suggested_size = (suggested_size < 8096) ? 8096 : suggested_size;
	struct udp_recv_buf* udp_buf = (struct udp_recv_buf*)handle->data;
	if (udp_buf->max_recv_len < suggested_size) {
		if (udp_buf->recv_buf) {
			free(udp_buf->recv_buf);
			udp_buf->recv_buf = NULL;
		}

		udp_buf->recv_buf = (char*)malloc(suggested_size);
		udp_buf->max_recv_len = suggested_size;
	}

	buf->base = udp_buf->recv_buf;
	buf->len = suggested_size;
}

static void
after_uv_udp_recv(uv_udp_t* handle,
	ssize_t nread,
	const uv_buf_t* buf,
	const struct sockaddr* addr,
	unsigned flags) {

	udp_session udp_s;
	udp_s.udp_handler = handle;
	udp_s.addr = addr;
	uv_ip4_name((struct sockaddr_in*)addr, udp_s.c_address, 32);
	udp_s.c_port = ntohs(((struct sockaddr_in*)addr)->sin_port);

	on_recv_client_cmd((session*)&udp_s, (unsigned char*)buf->base, nread);
}

void
netbus::start_upd_server(int port) {
	printf("启动udp服务器");
	uv_udp_t* server = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	memset(server, 0, sizeof(uv_udp_t));

	uv_udp_init(uv_default_loop(), server);
	struct udp_recv_buf* udp_buf = (struct udp_recv_buf*)malloc(sizeof(struct udp_recv_buf));
	memset(udp_buf, 0, sizeof(struct udp_recv_buf));
	server->data = (struct udp_recv_buf*)udp_buf;

	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	uv_udp_bind(server, (const struct sockaddr*)&addr, 0);

	uv_udp_recv_start(server, udp_uv_alloc_buf, after_uv_udp_recv);
}

//启动uv默认循环的方法
void netbus::run() {
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

//初始化的方法
void netbus::init(){
	printf("初始化内存相关\n");
	service_man::init();
	init_session_allocer();
}