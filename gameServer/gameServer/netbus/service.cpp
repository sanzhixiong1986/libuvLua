#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "session.h"
#include "proto_man.h"

#include "service.h"

/// <summary>
/// 获得用户收到的数据
/// </summary>
/// <param name="s">客户端的索引</param>
/// <param name="msg">客户端获得的消息</param>
/// <returns>是否获取成功</returns>
bool service::on_session_recv_cmd(session* s, struct cmd_msg* msg) {
	return false;
}

/// <summary>
/// 消除客户端的相关操作
/// </summary>
/// <param name="s"></param>
void service::on_session_disconnect(session* s) {

}