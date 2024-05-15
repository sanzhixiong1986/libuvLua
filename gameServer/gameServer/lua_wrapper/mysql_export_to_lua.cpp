#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../lua_wrapper/lua_wrapper.h"
#include "mysql_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "tolua_fix.h"

static void on_open_cb(const char* err, void* context, void* udata) {
	if (err) {
		lua_pushstring(lua_wrapper::lua_state(), err);
		lua_pushnil(lua_wrapper::lua_state());
	}
	else {
		lua_pushnil(lua_wrapper::lua_state());
		tolua_pushuserdata(lua_wrapper::lua_state(), context);
	}

	lua_wrapper::execute_script_handler((int)udata, 2);
	lua_wrapper::remove_script_handler((int)udata);
}

static int lua_mysql_connect(lua_State* tolua_S) {
	char* ip = (char*)tolua_tostring(tolua_S, 1, 0);
	if (ip == NULL)
	{
		goto lua_failed;
	}

	int port = (int)tolua_tonumber(tolua_S, 2, 0);

	char* db_name = (char*)tolua_tostring(tolua_S, 3, 0);
	if (db_name == NULL) {
		goto lua_failed;
	}

	char* uname = (char*)tolua_tostring(tolua_S, 4, 0);
	if (uname == NULL) {
		goto lua_failed;
	}

	char* upwd = (char*)tolua_tostring(tolua_S, 5, 0);
	if (upwd == NULL) {
		goto lua_failed;
	}

	int handler = toluafix_ref_function(tolua_S, 6, 0);
	mysql_wrapper::connect(ip, port, db_name, uname, upwd, on_open_cb, (void*)handler);
lua_failed:
	return 0;
}

static int lua_mysql_close(lua_State* tolua_S) {
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (context)
	{
		mysql_wrapper::close(context);
	}
	return 0;
}

static int lua_mysql_query(lua_State* tolua_S) {
	return 0;
}

int register_mysql_export(lua_State* tolua_S) {
	lua_getglobal(tolua_S,"_G");

	if (lua_istable(tolua_S, -1))
	{
		tolua_open(tolua_S);
		tolua_module(tolua_S, "mysql_wrapper", 0);
		tolua_beginmodule(tolua_S, "mysql_wrapper");

		tolua_function(tolua_S, "connect", lua_mysql_connect);
		tolua_function(tolua_S, "close", lua_mysql_close);
		tolua_function(tolua_S, "query", lua_mysql_query);
	}
	lua_pop(tolua_S, 1);
	return 0;
}