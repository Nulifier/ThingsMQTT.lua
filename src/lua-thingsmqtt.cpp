#include "lua-thingsmqtt.h"
#include <nlohmann/json.hpp>
#include "controller.hpp"
#include "lauxlib.h"
#include "lua-thingsmqtt-private.hpp"

static const char* thingsmqtt_meta = "ThingsMqtt";

luaL_Reg library_methods[] = {{"new", lua_thingsmqtt_new}, {NULL, NULL}};
luaL_Reg thingsmqtt_methods[] = {{"connect", lua_thingsmqtt_connect},
								 {"telemetry", lua_thingsmqtt_telemetry},
								 {"send", lua_thingsmqtt_send},
								 {"loop", lua_thingsmqtt_loop},
								 {"is_connected", lua_thingsmqtt_is_connected},
								 {NULL, NULL}};

int luaopen_thingsmqtt(lua_State* L) {
	STACK_START(luaopen_thingsmqtt, 1);

	// Pop module name
	lua_pop(L, 1);

	// Create the ThingsMqtt metatable
	if (luaL_newmetatable(L, thingsmqtt_meta)) {
		luaL_register(L, nullptr, thingsmqtt_methods);
		lua_setfield(L, -1, "__index");
	} else {
		lua_pop(L, 1);
	}

	// Create the ThingsMqtt library
	lua_newtable(L);
	luaL_register(L, nullptr, library_methods);

	STACK_END(luaopen_thingsmqtt, 1);

	return 1;
}

int lua_thingsmqtt_new(lua_State* L) {
	STACK_START(lua_thingsmqtt_new, 0);

	// Create a new Controller
	void* ud = lua_newuserdata(L, sizeof(Controller*));
	Controller** controller = static_cast<Controller**>(ud);

	*controller = new Controller();

	luaL_getmetatable(L, thingsmqtt_meta);
	lua_setmetatable(L, -2);

	STACK_END(lua_thingsmqtt_new, 1);

	return 1;
}

int lua_thingsmqtt_connect(lua_State* L) {
	STACK_START(lua_thingsmqtt_connect, 2);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));

	// Read config
	ControllerConfig config;
	lua_getfield(L, 2, "host");
	if (auto host = lua_tostring(L, -1)) {
		config.host = host;
	}
	lua_getfield(L, 2, "port");
	if (lua_isnumber(L, -1)) {
		config.port = lua_tointeger(L, -1);
	}
	lua_getfield(L, 2, "bind_address");
	if (auto bindAddr = lua_tostring(L, -1)) {
		config.bind_address = bindAddr;
	}
	lua_getfield(L, 2, "keepalive");
	if (lua_isnumber(L, -1)) {
		config.keepalive = lua_tointeger(L, -1);
	}
	lua_getfield(L, 2, "client_id");
	if (auto clientId = lua_tostring(L, -1)) {
		config.client_id = clientId;
	}
	lua_getfield(L, 2, "username");
	if (auto username = lua_tostring(L, -1)) {
		config.username = username;
	}
	lua_getfield(L, 2, "password");
	if (auto password = lua_tostring(L, -1)) {
		config.password = password;
	}
	lua_pop(L, 7);

	// Get SSL config
	lua_getfield(L, 2, "ssl_config");
	if (lua_istable(L, -1)) {
		lua_getfield(L, 3, "ca_file");
		if (auto caFile = lua_tostring(L, -1)) {
			config.ssl_config.ca_file = caFile;
		}
		lua_getfield(L, 3, "cert_file");
		if (auto certFile = lua_tostring(L, -1)) {
			config.ssl_config.cert_file = certFile;
		}
		lua_getfield(L, 3, "key_file");
		if (auto keyFile = lua_tostring(L, -1)) {
			config.ssl_config.key_file = keyFile;
		}
		lua_getfield(L, 3, "verify_peer");
		if (lua_isboolean(L, -1)) {
			config.ssl_config.verify_peer = lua_toboolean(L, -1);
		}
		lua_getfield(L, 3, "verify_hostname");
		if (lua_isboolean(L, -1)) {
			config.ssl_config.verify_hostname = lua_toboolean(L, -1);
		}
		lua_pop(L, 5);
	}
	lua_pop(L, 3);	// Pop userdata, config, and ssl_config

	controller->connect(config);

	STACK_END(lua_thingsmqtt_connect, 0);

	return 0;
}

int lua_thingsmqtt_telemetry(lua_State* L) {
	STACK_START(lua_thingsmqtt_telemetry, 3);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));

	// Get key
	const char* key = luaL_checkstring(L, 2);

	// Get value
	switch (lua_type(L, 3)) {
		case LUA_TNIL: {
			controller->publishTelemetry(key,
										 std::move(nlohmann::json(nullptr)));
			break;
		}
		case LUA_TNUMBER: {
			double value = luaL_checknumber(L, 3);

			// Check if value is an integer or a float
			double int_part;
			if (std::modf(value, &int_part) == 0.0) {
				controller->publishTelemetry(
					key,
					std::move(nlohmann::json(static_cast<int64_t>(int_part))));
			} else {
				controller->publishTelemetry(key,
											 std::move(nlohmann::json(value)));
			}
			break;
		}
		case LUA_TBOOLEAN: {
			bool value = lua_toboolean(L, 3);
			controller->publishTelemetry(key, std::move(nlohmann::json(value)));
			break;
		}
		case LUA_TSTRING: {
			const char* value = luaL_checkstring(L, 3);
			controller->publishTelemetry(key, std::move(nlohmann::json(value)));
			break;
		}
		case LUA_TTABLE: {
			nlohmann::json table = nlohmann::json::object();
			lua_pushnil(L);
			while (lua_next(L, 3) != 0) {
				const char* key = luaL_checkstring(L, -2);
				switch (lua_type(L, -1)) {
					case LUA_TNUMBER: {
						double value = luaL_checknumber(L, -1);
						table[key] = value;
						break;
					}
					case LUA_TBOOLEAN: {
						bool value = lua_toboolean(L, -1);
						table[key] = value;
						break;
					}
					case LUA_TSTRING: {
						const char* value = luaL_checkstring(L, -1);
						table[key] = value;
						break;
					}
					default:
						break;
				}
				lua_pop(L, 1);
			}
			controller->publishTelemetry(key, std::move(table));
			break;
		}
	}

	lua_pop(L, 3);

	STACK_END(lua_thingsmqtt_telemetry, 0);

	return 0;
}

int lua_thingsmqtt_send(lua_State* L) {
	STACK_START(lua_thingsmqtt_send, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	controller->send();

	STACK_END(lua_thingsmqtt_send, 0);

	return 0;
}

int lua_thingsmqtt_loop(lua_State* L) {
	STACK_START(lua_thingsmqtt_loop, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	controller->loop();

	STACK_END(lua_thingsmqtt_loop, 0);

	return 0;
}

static int lua_thingsmqtt_is_connected(lua_State* L) {
	STACK_START(lua_thingsmqtt_is_connected, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	lua_pushboolean(L, controller->isConnected());

	STACK_END(lua_thingsmqtt_is_connected, 1);

	return 1;
}
