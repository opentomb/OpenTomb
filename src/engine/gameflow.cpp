#include "gameflow.h"

#include <cstdio>
#include <cstdlib>

#include "engine/engine.h"
#include "gui/gui.h"
#include "script/script.h"
#include "world/world.h"

namespace engine
{

Gameflow Gameflow_Manager;

void Gameflow::init()
{
    for(GameflowAction& action : m_actions)
        action.opcode = Opcode::Sentinel;
}

void Gameflow::execute()
{
    if(!m_nextAction)
    {
        return;
    }

    bool completed = true;

    for(GameflowAction& action : m_actions)
    {
        if(action.opcode == Opcode::Sentinel)
            continue;
        completed = false;

        switch(action.opcode)
        {
            case Opcode::LevelComplete:
                // Switch level only when fade is complete AND all streams / sounds are unloaded!
                if(gui::getFaderStatus(gui::FaderType::LoadScreen) == gui::FaderStatus::Complete && !audio::isTrackPlaying())
                {
                    const char* levelName;
                    const char* levelPath;
                    lua::tie(levelPath, levelName, m_currentLevelID) = engine_lua["getNextLevel"](int(m_currentGameID), int(m_currentLevelID), int(action.operand));
                    m_currentLevelName = levelName;
                    m_currentLevelPath = levelPath;
                    engine::loadMap(m_currentLevelPath);
                    action.opcode = Opcode::Sentinel;
                }
                else
                {
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    continue;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;

            default:
                action.opcode = Opcode::Sentinel;
                break;
        }   // end switch(gameflow_manager.Operand)
    }

    if(completed)
        m_nextAction = false;    // Reset action marker!
}

bool Gameflow::send(Opcode opcode, int operand)
{
    for(GameflowAction& action : m_actions)
    {
        if(action.opcode == opcode)
            return false;

        if(action.opcode == Opcode::Sentinel)
        {
            action.opcode = opcode;
            action.operand = operand;
            m_nextAction = true;
            return true;
        }
    }
    return false;
}

} // namespace engine

