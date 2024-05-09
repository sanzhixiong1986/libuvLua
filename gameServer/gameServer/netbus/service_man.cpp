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
/// ע�����
/// </summary>
/// <param name="stype"></param>
/// <param name="s"></param>
/// <returns></returns>
bool service_man::register_service(int stype, service* s) {
	//�ж��Ƿ�ע������ݲ��Ժ�ע�����
	if (stype < 0 || stype >= MAX_SERVICE) {
		return false;
	}

	//ע���Ѿ�����
	if (g_service_set[stype]) {
		return false;
	}

	g_service_set[stype] = s;
	return true;
}

/// <summary>
/// �������
/// </summary>
/// <param name="s">�ͻ���</param>
/// <param name="msg">�յ�������</param>
/// <returns>�Ƿ�ɹ�</returns>
bool service_man::on_recv_cmd_msg(session* s, struct cmd_msg* msg) {
	//�жϷ����ǲ��ǲ�����
	if (g_service_set[msg->stype] == NULL) {
		return false;
	}
	
	return g_service_set[msg->stype]->on_session_recv_cmd(s, msg);
}

/// <summary>
/// ��¼
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
/// ��ʼ��
/// </summary>
void service_man::init() {
	memset(g_service_set, 0, sizeof(g_service_set));
}