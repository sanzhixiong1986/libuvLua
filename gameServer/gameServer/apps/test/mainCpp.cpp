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

static
void on_logger_timer(void* udata) {
	log_debug("on_logger_timer");
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
	netbus::instance()->init();
	netbus::instance()->start_tcp_server(6080);
	netbus::instance()->start_ws_server(8001);
	netbus::instance()->start_upd_server(8002);
	netbus::instance()->run();
	return 0;
}