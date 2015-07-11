/* 
 * File:   engine_lua.h
 * Author: nTesla
 *
 * Created on July 11, 2015, 1:28 PM
 */

#ifndef ENGINE_LUA_H
#define	ENGINE_LUA_H

struct lua_State;

int lua_print(lua_State * lua);
int lua_BindKey(lua_State *lua);

bool Engine_LuaInit();
void Engine_LuaClearTasks();
void Engine_LuaRegisterFuncs(lua_State *lua);

#endif	/* ENGINE_LUA_H */

