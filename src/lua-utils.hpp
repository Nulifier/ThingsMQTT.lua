#pragma once

#include <lua.hpp>
#include <nlohmann/json.hpp>

/**
 * Converts a Lua value to a JSON value.
 * @param L The Lua state.
 * @param index The index of the Lua value to convert.
 */
nlohmann::json lua_value_to_json(lua_State* L, int index);

/**
 * Converts a JSON value to a Lua value.
 * Pushes the resulting value onto the Lua stack.
 * @param L The Lua state.
 * @param json The JSON value to convert.
 */
void lua_json_to_value(lua_State* L, const nlohmann::json& json);
