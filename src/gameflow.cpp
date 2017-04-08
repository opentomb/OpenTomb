extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/gl_text.h"
#include "core/console.h"
#include "script/script.h"
#include "gameflow.h"

#include <assert.h>
#include <vector>

struct gameflow_s
{
    uint8_t                         m_currentGameID;
    uint8_t                         m_currentLevelID;

    char                            m_currentLevelName[LEVEL_NAME_MAX_LEN];
    char                            m_currentLevelPath[MAX_ENGINE_PATH];
    char                            m_secretsTriggerMap[GF_MAX_SECRETS];

    std::vector<gameflow_action>    m_actions;
} global_gameflow;


void Gameflow_Init()
{
    memset(global_gameflow.m_currentLevelName, 0, sizeof(global_gameflow.m_currentLevelName));
    memset(global_gameflow.m_currentLevelPath, 0, sizeof(global_gameflow.m_currentLevelPath));
    memset(global_gameflow.m_secretsTriggerMap, 0, sizeof(global_gameflow.m_secretsTriggerMap));
    global_gameflow.m_actions.clear();
}


bool Gameflow_Send(int opcode, int operand)
{
    gameflow_action act;

    act.m_opcode = opcode;
    act.m_operand = operand;
    global_gameflow.m_actions.push_back(act);

    return true;
}


void Gameflow_ProcessCommands()
{
    for(; !global_gameflow.m_actions.empty(); global_gameflow.m_actions.pop_back())
    {
        gameflow_action &it = global_gameflow.m_actions.back();
        switch(it.m_opcode)
        {
            case GF_OP_LEVELCOMPLETE:
            {
                int top = lua_gettop(engine_lua);

                //Must be loaded from gameflow script!
                lua_getglobal(engine_lua, "getNextLevel");

                //Push the 3 arguments require dby getNextLevel();
                lua_pushnumber(engine_lua, global_gameflow.m_currentGameID);
                lua_pushnumber(engine_lua, global_gameflow.m_currentLevelID);
                lua_pushnumber(engine_lua, it.m_operand);

                if (lua_CallAndLog(engine_lua, 3, 3, 0))
                {
                    //First value in stack is level id
                    global_gameflow.m_currentLevelID = lua_tonumber(engine_lua, -1);
                    //Second value in stack is level name
                    strncpy(global_gameflow.m_currentLevelName, lua_tostring(engine_lua, -2), LEVEL_NAME_MAX_LEN);
                    //Third value in stack is level path
                    strncpy(global_gameflow.m_currentLevelPath, lua_tostring(engine_lua, -3), MAX_ENGINE_PATH);
                    Engine_LoadMap(global_gameflow.m_currentLevelPath);
                }
                else
                {
                    Con_AddLine("Fatal Error: Failed to call GetNextLevel()", FONTSTYLE_CONSOLE_WARNING);
                }
                lua_settop(engine_lua, top);
                it.m_opcode = GF_NOENTRY;
            }
            break;

            case GF_NOENTRY:
                continue;

            default:
                //Con_Printf("Unimplemented gameflow opcode: %i", global_gameflow.m_actions[i].m_opcode);
                break;
        };   // end switch(gameflow_manager.Operand)
    }
}

void Gameflow_ResetSecrets()
{
    memset(global_gameflow.m_secretsTriggerMap, 0, GF_MAX_SECRETS * sizeof(*global_gameflow.m_secretsTriggerMap));
}


const char* Gameflow_GetCurrentLevelPathLocal()
{
    return global_gameflow.m_currentLevelPath + strlen(Engine_GetBasePath());
}


void Gameflow_SetCurrentLevelPath(const char* filePath)
{
    assert(strlen(filePath) < LEVEL_NAME_MAX_LEN);
    strncpy(global_gameflow.m_currentLevelPath, filePath, MAX_ENGINE_PATH);
}


void Gameflow_SetCurrentGameID(uint8_t id)
{
    assert(id >= 0 && id <= GAME_5);
    global_gameflow.m_currentGameID = id;
}


void Gameflow_SetCurrentLevelID(uint8_t id)
{
    assert(id >= 0);///@TODO pull max level ID from script?
    global_gameflow.m_currentLevelID = id;
}


void Gameflow_SetSecretStateAtIndex(int index, int value)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
    global_gameflow.m_secretsTriggerMap[index] = (char)value; ///@FIXME should not cast.
}


uint8_t Gameflow_GetCurrentGameID()
{
    return global_gameflow.m_currentGameID;
}


uint8_t Gameflow_GetCurrentLevelID()
{
    return global_gameflow.m_currentLevelID;
}


int Gameflow_GetSecretStateAtIndex(int index)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
    return global_gameflow.m_secretsTriggerMap[index];
}