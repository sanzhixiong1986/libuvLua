#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;


#include "netbus.h"

static netbus g_netbus;
netbus* netbus::instance() {
	return &g_netbus;
}

void netbus::start_tcp_server(int port) {
	printf("start_tcp_server\n");
}


void netbus::run() {
	printf("run\n");
}