#pragma once

#include <cstdint>
#include <string>

#include <lua.hpp>

#include "LuaStack.h"

namespace lua {
namespace stack {

template<>
inline bool check<size_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<uint16_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<int16_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<uint32_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<uint8_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<int8_t> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<char> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<float> (lua_State* state, int n) {
    return lua_isnumber(state, n);
}

template<>
inline bool check<std::string> (lua_State* state, int n) {
    return lua_isstring(state, n);
}



template<>
inline std::string read(lua_State* luaState, int index) {
    return lua_tostring(luaState, index);
}

template<>
inline uint8_t read(lua_State* luaState, int index) {
    return lua_tounsigned(luaState, index);
}

template<>
inline int8_t read(lua_State* luaState, int index) {
    return lua_tointeger(luaState, index);
}

}
}
