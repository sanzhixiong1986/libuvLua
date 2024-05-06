#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"

#include "session.h"
#include "uv_session.h"

//2024.5.6新添加的缓存记录
#include "../utils/cache_alloc.h"

//5.6
#define SESSION_CACHE_CAPACITY 6000
#define WQ_CACHE_CAPCITY 4096
//end

struct cache_allocer* session_allocer = NULL;
struct cache_allocer* wr_allocer = NULL;//写和读的相关操作

//5.6添加一个session初始化的方法
void init_session_allocer(){
	if (session_allocer == NULL){
		session_allocer = create_cache_allocer(SESSION_CACHE_CAPACITY, sizeof(uv_session));
	}

	if (wr_allocer == NULL){
		wr_allocer = create_cache_allocer(WQ_CACHE_CAPCITY, sizeof(uv_write_t));
	}
}

extern "C"{

	//返回函数之发送成功以后
	static void after_write(uv_write_t* req, int status){
		if (status == 0){
			printf("write success\n");
		}
		//5.6添加缓存的方法
		cache_free(wr_allocer, req);
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
//v_session* uv_s = new uv_session();
	//5.6 
	uv_session* uv_s = (uv_session*)cache_alloc(session_allocer, sizeof(uv_session));
	uv_s->uv_session::uv_session();
	//end
	uv_s->init();
	return uv_s;
}

/************************************************************************/
/* 销毁的方法                                                                     */
/************************************************************************/
void uv_session::destroy(uv_session* s){
	s->exit();

	//5.6 
	s->uv_session::~uv_session();
//elete s;
	cache_free(session_allocer, s);
	//end
}

/************************************************************************/
/* 初始化方法                                                                     */
/************************************************************************/
void uv_session::init(){
	//创建地址内存
	memset(this->c_address, 0, sizeof(c_address));
	//5.6 revise
	this->c_port = 0;
	this->recved = 0;
	this->is_shutdown = false;
	//end
}

//退出方法
void uv_session::exit(){}

/************************************************************************/
/* 关闭的方法，继承下来的                                                                     */
/************************************************************************/
void uv_session::close(){
	//5.6 edit
	if (this->is_shutdown){
		return;
	}
	this->is_shutdown = true;
	//end
	uv_shutdown_t* req = &this->shutdown;
	memset(req, 0, sizeof(uv_shutdown_t));
	uv_shutdown(req, (uv_stream_t*)&this->tcp_handler, on_shutdown);
}


/************************************************************************/
/* 发送数据方法                                                                     */
/************************************************************************/
void uv_session::send_data(unsigned char* body,int len){
	//_write_t* w_req = &this->w_req;
	//_buf_t* w_buf = &this->w_buf;
	//5.6修改属性
	uv_write_t* w_req = (uv_write_t*)cache_alloc(wr_allocer, sizeof(uv_write_t));
	uv_buf_t w_buf;
	//end
	w_buf = uv_buf_init((char*)body, len);
	uv_write(w_req, (uv_stream_t*)&this->tcp_handler, &w_buf, 1, after_write);
}


//获得当前地址
const char*
uv_session::get_address(int* port){
	*port = this->c_port;
	return this->c_address;
}