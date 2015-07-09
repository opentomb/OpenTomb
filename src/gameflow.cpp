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
                    int top = lua_gettop(engine_lua);
                    lua_getglobal(engine_lua, "getNextLevel");                       // Must be loaded from gameflow script!
                    lua_pushnumber(engine_lua, gameflow_manager.CurrentGameID);      // Push the 1st argument
                    lua_pushnumber(engine_lua, gameflow_manager.CurrentLevelID);     // Push the 2nd argument
                    lua_pushnumber(engine_lua, gameflow_manager.Actions[i].operand); // Push the 3rd argument

                    if (lua_CallAndLog(engine_lua, 3, 3, 0))
                    {
                        gameflow_manager.CurrentLevelID = lua_tonumber(engine_lua, -1);   // First value in stack is level id
                        strncpy(gameflow_manager.CurrentLevelName, lua_tostring(engine_lua, -2), LEVEL_NAME_MAX_LEN); // Second value in stack is level name
                        strncpy(gameflow_manager.CurrentLevelPath, lua_tostring(engine_lua, -3), MAX_ENGINE_PATH); // Third value in stack is level path

                        // Now, load the level! + if character exists then save inventory
                        /*if((engine_world.character != NULL) && (engine_world.character->character != NULL))
                        {
                            inventory_node_p i = engine_world.character->character->inventory;
                            engine_world.character->character->inventory = NULL;
                            Engine_LoadMap(gameflow_manager.CurrentLevelPath);
                            engine_world.character->character->inventory = i;
                        }
                        else*/
                        {
                            Engine_LoadMap(gameflow_manager.CurrentLevelPath);
                        }
                    }
                    else
                    {
                        Gui_FadeStop(FADER_LOADSCREEN);
                        ConsoleInfo::instance().addLine("Fatal Error: Failed to call GetNextLevel()", FONTSTYLE_CONSOLE_WARNING);
                    }
                    lua_settop(engine_lua, top);
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
