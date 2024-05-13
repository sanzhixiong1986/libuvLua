#define WIN32_INTEROP_TYPES_H 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//redis的相关
#include "deps/hiredis/hiredis.h"
#include "src/Win32_Interop/win32fixes.h"
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"hiredis.lib")
#pragma comment(lib,"Win32_Interop.lib")
//end

#include "uv.h"
#include "redis_wrapper.h"

#define my_malloc malloc
#define my_free free

struct connect_req {

	char* ip;
	int port;

	void(*open_cb)(const char* err, void* context);

	char* err;
	void* context;
};

struct redis_context {
	void* pConn;
	uv_mutex_t lock;
	int is_closed;
};

struct query_req {
	void* context;
	char* cmd;
	void(*query_cb)(const char* err, redisReply* result);

	char* err;
	redisReply* result;
};

static void connect_work(uv_work_t* req) {
	struct connect_req* r = (struct connect_req*)req->data;

	struct timeval timeout = { 5,0 };
	redisContext* rc = redisConnectWithTimeout((char*)r->ip, r->port, timeout);
	if (rc->err) {
		printf("Connection error: %s\n", rc->errstr);
		r->err = strdup(rc->errstr);
		r->context = NULL;
		redisFree(rc);
	}
	else {
		struct redis_context* c = (struct redis_context*)my_malloc(sizeof(struct redis_context));
		memset(c, 0, sizeof(struct redis_context));
		c->pConn = rc;
		uv_mutex_init(&c->lock);
		r->err = NULL;
		r->context = c;
	}
}

static void on_connect_complete(uv_work_t* req, int status) {
	struct connect_req* r = (struct connect_req*)req->data;
	r->open_cb(r->err, r->context);

	if (r->ip) {
		free(r->ip);
	}

	if (r->err) {
		free(r->err);
	}
	my_free(r);
	my_free(req);
}

/// <summary>
/// 链接
/// </summary>
/// <param name="ip">ip</param>
/// <param name="port">端口</param>
/// <param name="open_cb">返回函数</param>
void
redis_wrapper::connect(char* ip, int port,
	void(*open_cb)(const char* err, void* context)) {
	uv_work_t* w = (uv_work_t*)my_malloc(sizeof(uv_work_t));
	memset(w, 0, sizeof(uv_work_t));

	struct connect_req* r = (struct connect_req*)my_malloc(sizeof(struct connect_req));
	memset(r, 0, sizeof(struct connect_req));

	r->ip = strdup(ip);
	r->port = port;

	r->open_cb = open_cb;

	w->data = (void*)r;
	int result = uv_queue_work(uv_default_loop(), w, connect_work, on_connect_complete);
	if (result) {
		fprintf(stderr, "uv_queue_work failed with error %s\n", uv_strerror(result));
		// 进行错误处理，如释放内存等
	}
}

static void on_close_complete(uv_work_t* req, int status) {
	struct redis_context* r = (struct redis_context*)(req->data);
	my_free(r);
	my_free(req);
}

static void close_work(uv_work_t* req) {
	struct redis_context* r = (struct redis_context*)(req->data);
	uv_mutex_unlock(&r->lock);

	redisContext* c = (redisContext*)r->pConn;
	redisFree(c);
	r->pConn = NULL;
	uv_mutex_unlock(&r->lock);
}

/// <summary>
/// 关闭redis的方法
/// </summary>
/// <param name="context">客户端的链接对象</param>
void redis_wrapper::close_redis(void* context) {
	struct redis_context* c = (struct redis_context*)context;
	if (c->is_closed) {
		return;
	}

	uv_work_t* w = (uv_work_t*)my_malloc(sizeof(uv_work_t));
	memset(w, 0, sizeof(uv_work_t));
	w->data = (context);

	c->is_closed = 1;
	uv_queue_work(uv_default_loop(), w, close_work, on_close_complete);
}

static void query_work(uv_work_t* req) {
	query_req* r = (query_req*)req->data;

	struct redis_context* my_conn = (struct redis_context*)(r->context);
	redisContext* rc = (redisContext*)my_conn->pConn;

	uv_mutex_lock(&my_conn->lock);
	r->err = NULL;

	redisReply* replay = (redisReply*)redisCommand(rc, r->cmd);

	if (replay) {
		r->result = replay;
	}
	uv_mutex_unlock(&my_conn->lock);
}

static void on_query_complete(uv_work_t* req, int status) {
	query_req* r = (query_req*)req->data;
	r->query_cb(r->err, r->result);

	if (r->cmd) {
		free(r->cmd);
	}

	if (r->result) {
		freeReplyObject(r->result);
	}

	if (r->err) {
		free(r->err);
	}

	my_free(r);
	my_free(req);
}


/// <summary>
/// 执行redis的相关操作
/// </summary>
/// <param name="context">地址</param>
/// <param name="cmd">命令</param>
/// <param name="query_cb">返回</param>
void redis_wrapper::query(void* context, char* cmd, void(*query_cb)(const char* err, redisReply* result)) {
	struct redis_context* c = (struct redis_context*)context;
	if (c->is_closed) {
		return;
	}

	uv_work_t* w = (uv_work_t*)my_malloc(sizeof(uv_work_t));
	memset(w, 0, sizeof(uv_work_t));

	query_req* r = (query_req*)my_malloc(sizeof(query_req));
	memset(r, 0, sizeof(query_req));

	r->context = context;
	r->cmd = strdup(cmd);
	r->query_cb = query_cb;

	w->data = r;
	uv_queue_work(uv_default_loop(), w, query_work, on_query_complete);
}