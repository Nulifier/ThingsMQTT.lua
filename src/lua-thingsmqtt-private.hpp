#pragma once

#include <lua.hpp>
#include "thingsmqtt-config.hpp"

static int lua_thingsmqtt_new(lua_State* L);
static int lua_thingsmqtt_connect(lua_State* L);
static int lua_thingsmqtt_telemetry(lua_State* L);
static int lua_thingsmqtt_send(lua_State* L);
static int lua_thingsmqtt_loop(lua_State* L);
static int lua_thingsmqtt_is_connected(lua_State* L);

static int lua_thingsmqtt_json_stringify(lua_State* L);
static int lua_thingsmqtt_json_parse(lua_State* L);

#ifdef THINGSMQTT_STACK_CHECK
#define STACK_START(fn_name, nargs)                             \
	int thingsmqtt_stack_top_##fn_name = lua_gettop(L) - nargs; \
	enum {}
#define STACK_END(fn_name, nresults)                                    \
	if (lua_gettop(L) != thingsmqtt_stack_top_##fn_name + nresults) {   \
		fprintf(stderr,                                                 \
				"Stack imbalance in %s: expected %d results, got %d\n", \
				#fn_name, nresults,                                     \
				lua_gettop(L) - thingsmqtt_stack_top_##fn_name);        \
	}                                                                   \
	enum {}
#else
#define STACK_START(name, nargs) enum {}
#define STACK_END(name, nresults) enum {}
#endif
