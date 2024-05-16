﻿#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../netbus/proto_man.h"

#include "lua_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "tolua_fix.h"

#include "proto_man_export_to_lua.h"
// local cmd_name_map = {"Name2", "Name1", "Name3"}... Lua 1¿ªÊ¼, cmd 1¿ªÊ¼
static int
lua_register_protobuf_cmd_map(lua_State* L) {
	std::map<int, std::string> cmd_map;
	int n = luaL_len(L, 1);
	if (n <= 0) {
		goto lua_failed;
	}
	for (int i = 1; i <= n; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, 1);
		const char* name = luaL_checkstring(L, -1);
		if (name) {
			cmd_map[i] = name;
		}
		lua_pop(L, 1);
	}
	proto_man::register_protobuf_cmd_map(cmd_map);
lua_failed:
	return 0;
}

static int
lua_proto_type(lua_State* tolua_S) {
	lua_pushinteger(tolua_S, proto_man::proto_type());
lua_failed:
	return 1;
}

static int
lua_proto_man_init(lua_State* tolua_S) {
	int argc = lua_gettop(tolua_S);
	if (argc != 1) {
		goto lua_failed;
	}
	int proto_type = (int)lua_tointeger(tolua_S, 1);
	if (proto_type != PROTO_JSON && proto_type != PROTO_BUF) {
		goto lua_failed;
	}

	proto_man::init(proto_type);
lua_failed:
	return 0;
}

int
register_proto_man_export(lua_State* tolua_S) {
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "proto_man", 0);
		tolua_beginmodule(tolua_S, "proto_man");

		tolua_function(tolua_S, "init", lua_proto_man_init);
		tolua_function(tolua_S, "proto_type", lua_proto_type);
		tolua_function(tolua_S, "register_protobuf_cmd_map", lua_register_protobuf_cmd_map);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);

	return 0;
}