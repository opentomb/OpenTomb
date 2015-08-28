#include "gameflow.h"

#include <cstdio>
#include <cstdlib>

#include "engine.h"
#include "gui.h"
#include "script.h"
#include "world.h"

Gameflow Gameflow_Manager;

void Gameflow::Init()
{
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        Actions[i].opcode = GF_NOENTRY;
    }
}

void Gameflow::Do()
{
    if(!m_nextAction)
    {
        return;
    }

    bool completed = true;

    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        if(Actions[i].opcode == GF_NOENTRY) continue;
        completed = false;

        switch(Actions[i].opcode)
        {
            case GF_OP_LEVELCOMPLETE:
                // Switch level only when fade is complete AND all streams / sounds are unloaded!
                if((Gui_FadeCheck(FaderType::LoadScreen) == FaderStatus::Complete) && (!Audio_IsTrackPlaying()))
                {
                    const char* levelName;
                    const char* levelPath;
                    lua::tie(levelPath, levelName, m_currentLevelID) = engine_lua["getNextLevel"](int(m_currentGameID), int(m_currentLevelID), int(Actions[i].operand));
                    m_currentLevelName = levelName;
                    m_currentLevelPath = levelPath;
                    Engine_LoadMap(m_currentLevelPath);
                    Actions[i].opcode = GF_NOENTRY;
                }
                else
                {
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    continue;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;

            default:
                Actions[i].opcode = GF_NOENTRY;
                break;
        }   // end switch(gameflow_manager.Operand)
    }

    if(completed) m_nextAction = false;    // Reset action marker!
}

bool Gameflow::Send(int opcode, int operand)
{
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        if(Actions[i].opcode == opcode) return false;

        if(Actions[i].opcode == GF_NOENTRY)
        {
            Actions[i].opcode = opcode;
            Actions[i].operand = operand;
            m_nextAction = true;
            return true;
        }
    }
    return false;
}

