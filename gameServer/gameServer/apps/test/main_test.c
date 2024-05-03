#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../../netbus/netbus.h"
int main__11(int argc, char** argv){

	printf("helloworld\n");

	NetBus::instance()->start_tcp_server(6080);
	NetBus::instance()->start_tcp_server(6081);

	NetBus::instance()->run();

	system("pause");
	return 0;
}