#include "lua-utils.hpp"

nlohmann::json lua_value_to_json(lua_State* L, int index) {
	switch (lua_type(L, index)) {
		case LUA_TNIL:
			return nullptr;
		case LUA_TBOOLEAN:
			return lua_toboolean(L, index);
		case LUA_TNUMBER:
			// Distinguish between integer and float
			// Since this is Lua 5.1, we check if the number is an integer by checking if modf is 0
			{
				lua_Number num = lua_tonumber(L, index);
				lua_Number intpart;
				if (modf(num, &intpart) == 0.0) {
					return static_cast<nlohmann::json::number_integer_t>(num);
				} else {
					return num;
				}
			}
		case LUA_TSTRING:
			return lua_tostring(L, index);
		case LUA_TTABLE: {
			// The value can be a table representing either an array or an
			// object. If index 1 exists, we treat it as an array; otherwise, as
			// an object.

			lua_pushinteger(L, 1);
			lua_gettable(L, index);
			if (lua_isnil(L, -1)) {
				// It's an object
				nlohmann::json obj = nlohmann::json::object();
				lua_pop(L, 1);	 // Pop nil
				lua_pushnil(L);	 // First key
				while (lua_next(L, index) != 0) {
					const char* key = luaL_checkstring(L, -2);
					nlohmann::json value = lua_value_to_json(L, -1);
					obj[key] = value;
					lua_pop(L, 1);	// Pop value, keep key for next iteration
				}
				return obj;
			} else {
				// It's an array
				lua_pop(L, 1);
				nlohmann::json json = nlohmann::json::array();
				lua_pushnil(L);
				while (lua_next(L, index) != 0) {
					json.push_back(lua_value_to_json(L, -1));
					lua_pop(L, 1);
				}
				return json;
			}
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
