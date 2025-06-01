#pragma once

#include <lua.h>
#include "lua-thingsmqtt-export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register the ThingsMQTT library with Lua.
 */
LUA_THINGSMQTT_EXPORT int luaopen_thingsmqtt(lua_State* L);

#ifdef __cplusplus
}
#endif
