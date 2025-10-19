#include "lua-utils.hpp"

nlohmann::json lua_value_to_json(lua_State* L, int index) {
	switch (lua_type(L, index)) {
		case LUA_TNIL:
			return nullptr;
		case LUA_TBOOLEAN:
			return lua_toboolean(L, index);
		case LUA_TNUMBER:
			return lua_tonumber(L, index);
		case LUA_TSTRING:
			return lua_tostring(L, index);
		case LUA_TTABLE: {
			nlohmann::json json = nlohmann::json::object();
			lua_pushnil(L);
			while (lua_next(L, index) != 0) {
				// Key is at -2, value is at -1
				json[lua_value_to_json(L, -2)] = lua_value_to_json(L, -1);
				lua_pop(L, 1);
			}
			return json;
		}
		default:
			return nullptr;
	}
}

void lua_json_to_value(lua_State* L, const nlohmann::json& json) {
	switch (json.type()) {
		case nlohmann::json::value_t::null:
			lua_pushnil(L);
			break;
		case nlohmann::json::value_t::boolean:
			lua_pushboolean(L, json.get<bool>());
			break;
		case nlohmann::json::value_t::number_integer:
			lua_pushinteger(L, json.get<int>());
			break;
		case nlohmann::json::value_t::number_unsigned:
			lua_pushinteger(L, json.get<unsigned int>());
			break;
		case nlohmann::json::value_t::number_float:
			lua_pushnumber(L, json.get<float>());
			break;
		case nlohmann::json::value_t::string:
			lua_pushstring(L, json.get<std::string>().c_str());
			break;
		case nlohmann::json::value_t::object: {
			lua_newtable(L);
			for (auto& [key, value] : json.items()) {
				lua_pushstring(L, key.c_str());
				lua_json_to_value(L, value);
				lua_settable(L, -3);
			}
			break;
		}
		case nlohmann::json::value_t::array: {
			lua_newtable(L);
			for (size_t i = 0; i < json.size(); ++i) {
				lua_pushinteger(L, i + 1);
				lua_json_to_value(L, json[i]);
				lua_settable(L, -3);
			}
			break;
		}
		default:
			lua_pushnil(L);
			break;
	}
}
