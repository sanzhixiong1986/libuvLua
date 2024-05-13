#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;
#include "../../netbus/proto_man.h"
#include "../../netbus/netbus.h"
#include "../../utils/logger.h"
#include "../../utils/logger.h"
#include "../../utils/time_list.h"
#include "../../utils/timestamp.h"

#include "proto/pf_cmd_map.h"

#include "../../database/mysql_wrapper.h"

#include "../../database/redis_wrapper.h"

#include "lua.hpp"



static
void on_logger_timer(void* udata) {
	log_debug("on_logger_timer");
}

static void on_query_cb(const char* err, std::vector<std::vector<std::string>>* result) {
	if (err) {
		printf("err");
		return;
	}

	/*if (result) {  // 检查指针是否非空
		for (const auto& row : *result) {  // 遍历每个内部向量
			for (const auto& item : row) {  // 遍历内部向量中的每个字符串
				std::cout << item << " ";
			}
			std::cout << std::endl;
		}
	}*/

	printf("success");
	std::cout << result;
}

static void on_open_cb(const char* err, void* context) {
	if (err != NULL) {
		printf("%s\n", err);
		return;
	}
	printf("connect success\n");

	mysql_wrapper::query(context, "select * from note", on_query_cb);
}

static void test_db() {
	mysql_wrapper::connect("127.0.0.1", 3306, "games", "root", "sanzhixiong", on_open_cb);
}

static void
on_redis_query(const char* err, redisReply* result) {
	if (err) {
		printf("%s\n", err);
		return;
	}
}

static void
on_redis_open(const char* err, void* context) {
	if (err != NULL) {
		printf("%s\n", err);
		return;
	}
	printf("connect success\n");

	redis_wrapper::query(context, "get hello", on_redis_query);
	// redis_wrapper::close_redis(context);
}

static void
test_redis() {
	redis_wrapper::connect("127.0.0.1", 6379, on_redis_open);
}

/// <summary>
/// 测试lua的代码
/// </summary>
static void text_lua() {
	printf("启动lua的相关\n");
	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	luaL_dofile(lua, "main.lua");
	lua_close(lua);
}

int main(int argc, char** argv){

	proto_man::init(PROTO_BUF);
	init_pf_cmd_map();
	//时间戳的和log的相关操作
	/*logger::init("logger/gateway/", "gateway", true);

	log_error("%d", timestamp());
	log_error("%d", timestamp_today());
	log_debug("%d", date2timestamp("%Y-%m-%d %H:%M:%S", "2018-02-01 00:00:00"));

	unsigned long yesterday = timestamp_yesterday();
	char out_buf[64];
	timestamp2date(yesterday, "%Y-%m-%d %H:%M:%S", out_buf, sizeof(out_buf));
	log_debug("%s", out_buf);
	schedule(on_logger_timer, NULL, 3000, -1);
	*/

	//test_db();
	//test_redis();
	text_lua();
	netbus::instance()->init();
	netbus::instance()->start_tcp_server(6080);
	netbus::instance()->start_ws_server(8001);
	netbus::instance()->start_upd_server(8002);
	netbus::instance()->run();
	return 0;
}