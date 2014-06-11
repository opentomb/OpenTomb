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
#include "gui.h"
#include "anim_state_control.h"
#include "world.h"

gameflow_manager_s gameflow_manager;

void Gameflow_Do()
{
    if(!gameflow_manager.NextAction)
        return;

    switch(gameflow_manager.Opcode)
    {
        case TR_GAMEFLOW_OP_LEVELCOMPLETE:
            if(Gui_IsFading(FADER_BLACK) == TR_FADER_STATUS_COMPLETE)   // Switch level only when fade is complete!
            {
                // Get our script filepath from game_script cvar!
                gameflow_manager.Script = CVAR_get_val_s("game_script");

                if(Engine_FileFound(gameflow_manager.Script))           // If the lua file exists...
                {
                    luaL_dofile(engine_lua, gameflow_manager.Script);
                    lua_getglobal(engine_lua, "GetNextLevel");
                    lua_pushnumber(engine_lua, gameflow_manager.CurrentLevelID); // Push the first argument
                    lua_pushnumber(engine_lua, gameflow_manager.Operand);        // Push the second argument

                    if (!lua_pcall(engine_lua, 2, 3, 0) != 0)
                    {
                        gameflow_manager.CurrentLevelID = lua_tonumber(engine_lua, -1);   // First value in stack is level id
                        lua_pop(engine_lua, 1); // Pop stack to get next value
                        gameflow_manager.CurrentLevelName = lua_tostring(engine_lua, -1); // Second value in stack is level name
                        lua_pop(engine_lua, 1); // Pop stack to get next value
                        gameflow_manager.CurrentLevelPath = lua_tostring(engine_lua, -1); // Third value in stack is level path
                        // Now, load the level!
                        Engine_LoadMap(gameflow_manager.CurrentLevelPath);

                    }
                    else
                    {
                        Con_Printf("Fatal Error: Failed to call GetNextLevel(); from LUA script: %s", gameflow_manager.Script);
                    }
                }
                else
                {
                    Con_Printf("Fatal Error: Cannot find script file at path: %s", gameflow_manager.Script);
                }
            }
            else
            {
                // If fadeout is in the process, we block level loading until it is complete.
                // It is achieved by not resetting action marker and exiting the function instead.
                return;

            }   // end if(!Gui_Fade(FADER_BLACK))
            break;

        default:
            break;  ///@FIXME: Implement all other gameflow opcodes here!

    }   // end switch(gameflow_manager.Operand)

    gameflow_manager.NextAction = false;    // Reset action marker!
}

void Gameflow_Send(int opcode, int operand)
{
    gameflow_manager.Operand    = operand;
    gameflow_manager.Opcode     = opcode;
    gameflow_manager.NextAction = true;
}
