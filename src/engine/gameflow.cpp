#include "gameflow.h"

#include "engine/engine.h"
#include "gui/gui.h"
#include "script/script.h"
#include "world/world.h"

namespace engine
{
Gameflow Gameflow::instance{};

void Gameflow::init()
{
    m_actions = std::queue<GameflowAction>();
}

void Gameflow::execute()
{
    while(!m_actions.empty())
    {
        switch(m_actions.front().opcode)
        {
            case Opcode::LevelComplete:
                // Switch level only when fade is complete AND all streams / sounds are unloaded!
                if(gui::Gui::instance->faders.getStatus(gui::FaderType::LoadScreen) == gui::FaderStatus::Complete && !Engine::instance.m_world.m_audioEngine.isTrackPlaying())
                {
                    lua::tie(m_currentLevelPath, m_currentLevelName, m_currentLevelID) = engine_lua["getNextLevel"](m_currentGameID, m_currentLevelID, m_actions.front().operand);
                    engine::Engine::instance.loadMap(m_currentLevelPath);
                    m_actions.pop();
                }
                else
                {
                    ///@FIXME Gameflow has NOTHING to do with faders! this should all be done elsewhere!
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    return;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;
            default:
                m_actions.pop();
        }   // end switch(gameflow_manager.Operand)
    }
}

/*
    Send opcode and operand to gameflow manager.
    Note: It will be added at the end of actions list.
*/

///@CRITICAL - The order MUST BE MAINTAINED.
void Gameflow::send(Opcode opcode, int operand)
{
    m_actions.emplace(opcode, operand);
}
} // namespace engine
