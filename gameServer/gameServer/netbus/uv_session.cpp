#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"

#include "session.h"
#include "uv_session.h"

extern "C"{

	//返回函数之发送成功以后
	static void after_write(uv_write_t* req, int status){
		if (status == 0){
			printf("write success\n");
		}
	}

	//关闭
	static void on_close(uv_handle_t* handle){
		uv_session* s = (uv_session*)handle->data;
		uv_session::destroy(s);
	}

	//关闭执行的方法
	static void on_shutdown(uv_shutdown_t* req,int status){
		uv_close((uv_handle_t*)req->handle, on_close);
	}
}

/************************************************************************/
/* 创建session的方法                                                                     */
/************************************************************************/
uv_session* uv_session::create(){
	uv_session* uv_s = new uv_session();
	uv_s->init();
	return uv_s;
}

/************************************************************************/
/* 销毁的方法                                                                     */
/************************************************************************/
void uv_session::destroy(uv_session* s){
	s->exit();
	delete s;
}

/************************************************************************/
/* 初始化方法                                                                     */
/************************************************************************/
void uv_session::init(){
	//创建地址内存
	memset(this->c_address, 0, sizeof(c_address));
}

//退出方法
void uv_session::exit(){}

/************************************************************************/
/* 关闭的方法，继承下来的                                                                     */
/************************************************************************/
void uv_session::close(){
	uv_shutdown_t* req = &this->shutdown;
	memset(req, 0, sizeof(uv_shutdown_t));
	uv_shutdown(req, (uv_stream_t*)&this->tcp_handler, on_shutdown);
}


/************************************************************************/
/* 发送数据方法                                                                     */
/************************************************************************/
void uv_session::send_data(unsigned char* body,int len){
	uv_write_t* w_req = &this->w_req;
	uv_buf_t* w_buf = &this->w_buf;

	*w_buf = uv_buf_init((char*)body, len);
	uv_write(w_req, (uv_stream_t*)&this->tcp_handler, w_buf, 1, after_write);
}


//获得当前地址
const char*
uv_session::get_address(int* port){
	*port = this->c_port;
	return this->c_address;
}