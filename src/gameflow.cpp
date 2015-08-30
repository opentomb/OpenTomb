#include "gameflow.h"

#include <cstdio>
#include <cstdlib>

#include "engine.h"
#include "gui.h"
#include "script.h"
#include "world.h"

Gameflow Gameflow_Manager;

/*
    Initialisation, sets all actions to GF_NOENTRY (-1)
*/
void Gameflow::Init()
{
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        m_actions[i].opcode = GF_NOENTRY;
    }
}

/*
    Process next gameflow action from action list if m_nextAction true
*/

void Gameflow::Do()
{
    if(!m_nextAction)
    {
        return;
    }

    bool completed = true;

    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        completed = false;

        switch(m_actions[i].opcode)
        {
            case GF_OP_LEVELCOMPLETE:
                // Switch level only when fade is complete AND all streams / sounds are unloaded!
                if((Gui_FadeCheck(FaderType::LoadScreen) == FaderStatus::Complete) && (!Audio_IsTrackPlaying()))
                {
                    lua::tie(m_currentLevelPath, m_currentLevelName, m_currentLevelID) = engine_lua["getNextLevel"](int(m_currentGameID), int(m_currentLevelID), int(m_actions[i].operand));
                    Engine_LoadMap(m_currentLevelPath);
                    m_actions[i].opcode = GF_NOENTRY;
                }
                else
                {
                    ///@FIXME Gameflow has NOTHING to do with faders! this should all be done elsewhere!
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    continue;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;

            default:
                m_actions[i].opcode = GF_NOENTRY;///?
                break;
        }   // end switch(gameflow_manager.Operand)
    }

    if(completed) m_nextAction = false;    // Reset action marker!
}

/*
    Send opcode and operand to gameflow manager.
    Note: It will be added at the end of actions list.
*/

///@CRITICAL - The order MUST BE MAINTAINED.
bool Gameflow::Send(int opcode, int operand)
{
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        if(m_actions[i].opcode == GF_NOENTRY)///@FIXME But what if [ -1, 2, 3 -1]? We're essentially favouring the first -1 which is WRONG.
        {
            m_actions[i].opcode = opcode;
            m_actions[i].operand = operand;
            m_nextAction = true;///@FIXME No, we shouldn't need to modify this here.
            return true;
        }
    }
    return false;
}
