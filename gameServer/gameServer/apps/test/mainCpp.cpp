#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "../../netbus/netbus.h"

int main(int argc, char** argv){

	netbus::instance()->start_tcp_server(6080);

	netbus::instance()->run();

	getchar();
	return 0;
}