#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "session.h"
#include "proto_man.h"

#include "service.h"

/// <summary>
/// ����û��յ�������
/// </summary>
/// <param name="s">�ͻ��˵�����</param>
/// <param name="msg">�ͻ��˻�õ���Ϣ</param>
/// <returns>�Ƿ��ȡ�ɹ�</returns>
bool service::on_session_recv_cmd(session* s, struct cmd_msg* msg) {
	return false;
}

/// <summary>
/// �����ͻ��˵���ز���
/// </summary>
/// <param name="s"></param>
void service::on_session_disconnect(session* s) {

}