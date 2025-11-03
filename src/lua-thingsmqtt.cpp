#include "lua-thingsmqtt.h"
#include <nlohmann/json.hpp>
#include "controller.hpp"
#include "lauxlib.h"
#include "lua-thingsmqtt-private.hpp"
#include "lua-utils.hpp"

static const char* thingsmqtt_meta = "ThingsMqtt";

luaL_Reg library_methods[] = {{"new", lua_thingsmqtt_new},
							  {"json_stringify", lua_thingsmqtt_json_stringify},
							  {"json_parse", lua_thingsmqtt_json_parse},
							  {NULL, NULL}};
luaL_Reg thingsmqtt_methods[] = {
	{"connect", lua_thingsmqtt_connect},
	{"telemetry", lua_thingsmqtt_telemetry},
	{"set_attribute", lua_thingsmqtt_set_attribute},
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
	nlohmann::json value = lua_value_to_json(L, 3);
	controller->publishTelemetry(key, std::move(value));

	lua_pop(L, 3);

	STACK_END(lua_thingsmqtt_telemetry, 0);

	return 0;
}

int lua_thingsmqtt_set_attribute(lua_State* L) {
	STACK_START(lua_thingsmqtt_set_attribute, 3);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));

	// Get key
	const char* key = luaL_checkstring(L, 2);

	// Get value
	nlohmann::json value = lua_value_to_json(L, 3);
	controller->setAttribute(key, std::move(value));

	lua_pop(L, 3);

	STACK_END(lua_thingsmqtt_set_attribute, 0);

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

int lua_thingsmqtt_is_connected(lua_State* L) {
	STACK_START(lua_thingsmqtt_is_connected, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	lua_pushboolean(L, controller->isConnected());

	STACK_END(lua_thingsmqtt_is_connected, 1);

	return 1;
}

int lua_thingsmqtt_add_rpc_handler(lua_State* L) {
	STACK_START(lua_thingsmqtt_add_rpc_handler, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	// Get the Lua function at index 2
	if (!lua_isfunction(L, 2)) {
		luaL_error(L, "Expected a function");
		return 0;
	}

	// Get the function reference
	lua_pushvalue(L, 2);
	int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Create a new RPC handler
	auto handler = [L, func_ref](const std::string& method,
								 const nlohmann::json& params) {
		lua_push_error_func(L);

		// Push the method name and parameters onto the Lua stack
		lua_pushstring(L, method.c_str());
		lua_json_to_value(L, params);

		// STACK: traceback, method, params

		// Call the Lua function
		lua_pcall(L, 2, 1, 0);

		// STACK: traceback, result

		// Get the result
		nlohmann::json result = lua_value_to_json(L, -1);
		lua_pop(L, 2);	// Pop the result and traceback

		return result;
	};

	// Add the RPC handler to the controller
	size_t handler_id = controller->addRpcHandler(std::move(handler));
	lua_pushinteger(L, handler_id);

	STACK_END(lua_thingsmqtt_add_rpc_handler, 1);

	return 1;
}

int lua_thingsmqtt_remove_rpc_handler(lua_State* L) {
	STACK_START(lua_thingsmqtt_remove_rpc_handler, 1);

	Controller* controller =
		*static_cast<Controller**>(luaL_checkudata(L, 1, thingsmqtt_meta));
	lua_pop(L, 1);

	int handler_id = luaL_checkinteger(L, 2);
	lua_pushboolean(L, controller->removeRpcHandler(handler_id));

	STACK_END(lua_thingsmqtt_remove_rpc_handler, 1);

	return 1;
}

int lua_thingsmqtt_json_stringify(lua_State* L) {
	STACK_START(lua_thingsmqtt_json_stringify, 1);

	nlohmann::json json = lua_value_to_json(L, 1);
	std::string json_str = json.dump();

	lua_pop(L, 1);	// Pop input value
	lua_pushstring(L, json_str.c_str());

	STACK_END(lua_thingsmqtt_json_stringify, 1);

	return 1;
}

int lua_thingsmqtt_json_parse(lua_State* L) {
	STACK_START(lua_thingsmqtt_json_parse, 1);

	const char* json_str = luaL_checkstring(L, 1);
	nlohmann::json json = nlohmann::json::parse(json_str, nullptr, false);
	if (json.is_discarded()) {
		lua_pop(L, 1);	// Pop input string
		lua_pushnil(L);
		STACK_END(lua_thingsmqtt_json_parse, 1);
		return 1;
	}

	lua_pop(L, 1);	// Pop input string
	lua_json_to_value(L, json);

	STACK_END(lua_thingsmqtt_json_parse, 1);

	return 1;
}

void lua_push_error_func(lua_State* L) {
	STACK_START(lua_push_error_func, 0);

	// Push debug.traceback function
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);	// Remove 'debug' table, leaving only the function

	STACK_END(lua_push_error_func, 1);
}
