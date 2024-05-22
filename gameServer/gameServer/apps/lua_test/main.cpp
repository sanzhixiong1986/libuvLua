#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;


#include "../../netbus/proto_man.h"
#include "../../netbus/netbus.h"
#include "../../utils/logger.h"
#include "../../utils/time_list.h"
#include "../../utils/timestamp.h"
#include "../../database/mysql_wrapper.h"
#include "../../database/redis_wrapper.h"
#include "../../lua_wrapper/lua_wrapper.h"

void start_auth() {
	std::string auth_path = "C:/work/c/libuvLua/gameServer/gameServer/apps/lua_test/scripts/auth_server/";
	lua_wrapper::add_search_path(auth_path);
	std::string auth_file = auth_path + "main.lua";
	lua_wrapper::do_file(auth_file);
}

int main(int argc, char** argv) {
	netbus::instance()->init();
	lua_wrapper::init();

	if (argc != 3) { // ²âÊÔ
		//C:/work/c/libuvLua/
		std::string search_path = "C:/work/c/libuvLua/gameServer/gameServer/apps/lua_test/scripts/";
		lua_wrapper::add_search_path(search_path);
		std::string lua_file = search_path + "gateway/main.lua";
		lua_wrapper::do_file(lua_file);
		// end 
	
		start_auth();
	}
	else {
		std::string search_path = argv[1];
		if (*(search_path.end() - 1) != '/') {
			search_path += "/";
		}
		lua_wrapper::add_search_path(search_path);

		std::string lua_file = search_path + argv[2];
		lua_wrapper::do_file(lua_file);
	}

	netbus::instance()->run();
	lua_wrapper::exit();

	system("pause");
	return 0;
}