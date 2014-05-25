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


const char* GF_Script = "scripts/tr1/gameflow.lua";
bool GF_NextAction;
const char* GF_CurrentLevelName;
uint8_t GF_CurrentLevelID;
uint8_t GF_Operand;

void Gameflow_Do()
{
    if(GF_NextAction == true)//If we need to load the next level
    {
        GF_Script = CVAR_get_val_s("game_script");//Get our script filepath from game_script cvar!

        if(Engine_FileFound(GF_Script))//If the lua file exists
        {
            luaL_dofile(engine_lua, GF_Script);
            lua_getglobal(engine_lua, "GetNextLevel");//Define the function to be called from LUA (GetNextLevel)
            lua_pushnumber(engine_lua, GF_CurrentLevelID);//Push the first argument (GF_CurrentLevelID)
            lua_pushnumber(engine_lua, GF_Operand);//Push the second argument (GF_Operand)

            if (!lua_pcall(engine_lua, 2, 3, 0) != 0)//Call function from lua, 2 arguments 3 return values
            {
                GF_CurrentLevelID = lua_tonumber(engine_lua, -1);//First value in stack is level id
                lua_pop(engine_lua, 1);//Pop stack to get next value
                GF_CurrentLevelName = lua_tostring(engine_lua, -1);//Second value in stack is level name
                lua_pop(engine_lua, 1);//Pop stack to get next value
                Engine_LoadMap(lua_tostring(engine_lua, -1));
                GF_NextAction = false;
            }
            else
            {
                Con_Printf("Fatal Error: Failed to call GetNextLevel(); from LUA script: %s", GF_Script);
                GF_NextAction = false;//An error has been detected, continuing is not possible! If this happens, check the LUA syntax!
            }
        }
        else
        {
            Con_Printf("Fatal Error: Cannot find script file at path: %s", GF_Script);
            GF_NextAction = false; //We want to stop cause yet again, an error has been encountered.
        }
    }
}


