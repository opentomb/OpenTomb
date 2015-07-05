#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <lua.hpp>

#include <AL/al.h>
#include <AL/alc.h>

#include "console.h"
#include "engine.h"
#include "gameflow.h"
#include "engine.h"
#include "gui.h"
#include "anim_state_control.h"
#include "world.h"

#include "LuaState.h"
#include "luastate_extra.h"

gameflow_manager_s gameflow_manager;

void Gameflow_Init()
{
    for(int i=0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
    }
}

void Gameflow_Do()
{
    if(!gameflow_manager.NextAction)
        return;

    bool completed = true;

    for(int i=0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        if(gameflow_manager.Actions[i].opcode == TR_GAMEFLOW_NOENTRY) continue;
        completed = false;

        switch(gameflow_manager.Actions[i].opcode)
        {
            case TR_GAMEFLOW_OP_LEVELCOMPLETE:
                if(Gui_FadeCheck(FADER_LOADSCREEN) == GUI_FADER_STATUS_COMPLETE)     // Switch level only when fade is complete!
                {
                    lua::tie(gameflow_manager.CurrentLevelID, gameflow_manager.CurrentLevelName, gameflow_manager.CurrentLevelPath) = engine_lua["getNextLevel"](int(gameflow_manager.CurrentGameID), int(gameflow_manager.CurrentLevelID), int(gameflow_manager.Actions[i].operand));
                    Engine_LoadMap(gameflow_manager.CurrentLevelPath);
                    gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
                }
                else
                {
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    continue;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;

            default:
                gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
                break;  ///@FIXME: Implement all other gameflow opcodes here!

        }   // end switch(gameflow_manager.Operand)
    }

    if(completed) gameflow_manager.NextAction = false;    // Reset action marker!
}

bool Gameflow_Send(int opcode, int operand)
{
    for(int i=0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        if(gameflow_manager.Actions[i].opcode == opcode) return false;

        if(gameflow_manager.Actions[i].opcode == TR_GAMEFLOW_NOENTRY)
        {
            gameflow_manager.Actions[i].opcode  = opcode;
            gameflow_manager.Actions[i].operand = operand;
            gameflow_manager.NextAction         = true;
            return true;
        }
    }
    return false;
}
