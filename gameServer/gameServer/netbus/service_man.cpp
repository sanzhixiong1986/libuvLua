#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "session.h"
#include "proto_man.h"
#include "service.h"
#include "service_man.h"

#define MAX_SERVICE 512

static service* g_service_set[MAX_SERVICE];


/// <summary>
/// 注册服务活动
/// </summary>
/// <param name="stype"></param>
/// <param name="s"></param>
/// <returns></returns>
bool service_man::register_service(int stype, service* s) {
	//判断是否注册的数据不对和注册过了
	if (stype < 0 || stype >= MAX_SERVICE) {
		return false;
	}

	//注册已经存在
	if (g_service_set[stype]) {
		return false;
	}

	g_service_set[stype] = s;
	return true;
}

/// <summary>
/// 获得数据
/// </summary>
/// <param name="s">客户端</param>
/// <param name="msg">收到的数据</param>
/// <returns>是否成功</returns>
bool service_man::on_recv_cmd_msg(session* s, struct cmd_msg* msg) {
	//判断服务是不是不存在
	if (g_service_set[msg->stype] == NULL) {
		return false;
	}
	
	return g_service_set[msg->stype]->on_session_recv_cmd(s, msg);
}

/// <summary>
/// 登录
/// </summary>
/// <param name="s"></param>
void service_man::on_session_disconnect(session* s) {
	if (s == NULL) {
		return;
	}

	for (int i=0;i<MAX_SERVICE;i++)
	{
		if (g_service_set[i] == NULL) {
			continue;
		}
		g_service_set[i]->on_session_disconnect(s);
	}
}

/// <summary>
/// 初始化
/// </summary>
void service_man::init() {
	memset(g_service_set, 0, sizeof(g_service_set));
}