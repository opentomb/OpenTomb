#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/lstate.h"
#include "al/AL/al.h"
#include "al/AL/alc.h"
}

#include "console.h"
#include "engine.h"
#include "gameflow.h"
#include "engine.h"
#include "anim_state_control.h"
#include "world.h"

gameflow_manager_s gameflow_manager;

void Gameflow_Do()
{
    if(gameflow_manager.NextAction == true)//If we need to load the next level
    {
        gameflow_manager.Script = CVAR_get_val_s("game_script");//Get our script filepath from game_script cvar!

        if(Engine_FileFound(gameflow_manager.Script))//If the lua file exists
        {
            luaL_dofile(engine_lua, gameflow_manager.Script);
            lua_getglobal(engine_lua, "GetNextLevel");//Define the function to be called from LUA (GetNextLevel)
            lua_pushnumber(engine_lua, gameflow_manager.CurrentLevelID);//Push the first argument (gameflow_manager.CurrentLevelID)
            lua_pushnumber(engine_lua, gameflow_manager.Operand);//Push the second argument (gameflow_manager.Operand)

            if (!lua_pcall(engine_lua, 2, 3, 0) != 0)//Call function from lua, 2 arguments 3 return values
            {
                gameflow_manager.CurrentLevelID = lua_tonumber(engine_lua, -1);//First value in stack is level id
                lua_pop(engine_lua, 1);//Pop stack to get next value
                gameflow_manager.CurrentLevelName = lua_tostring(engine_lua, -1);//Second value in stack is level name
                lua_pop(engine_lua, 1);//Pop stack to get next value
                Engine_LoadMap(lua_tostring(engine_lua, -1));
                gameflow_manager.NextAction = false;
            }
            else
            {
                Con_Printf("Fatal Error: Failed to call GetNextLevel(); from LUA script: %s", gameflow_manager.Script);
                gameflow_manager.NextAction = false;//An error has been detected, continuing is not possible! If this happens, check the LUA syntax!
            }
        }
        else
        {
            Con_Printf("Fatal Error: Cannot find script file at path: %s", gameflow_manager.Script);
            gameflow_manager.NextAction = false; //We want to stop cause yet again, an error has been encountered.
        }
    }
}
