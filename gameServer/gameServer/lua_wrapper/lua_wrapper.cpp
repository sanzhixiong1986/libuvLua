#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

#include "lua_wrapper.h"

lua_State* g_lua_State = NULL;

/// <summary>
/// ³õÊ¼»¯
/// </summary>
void lua_wrapper::init() {
	g_lua_State = luaL_newstate();
	luaL_openlibs(g_lua_State);
}


/// <summary>
/// ÍË³ö
/// </summary>
void lua_wrapper::exit() {
	if (g_lua_State != NULL) {
		lua_close(g_lua_State);
		g_lua_State = NULL;
	}
}

bool lua_wrapper::exe_lua_file(const char* lua_file) {
	if (lua_file == NULL) {
		return false;
	}

	if (luaL_dofile(g_lua_State, lua_file)) {
		return false;
	}
	return true;
}