extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/gl_text.h"
#include "core/console.h"
#include "gameflow.h"
#include "script.h"

#include <assert.h>

class CGameflow gameflow;

CGameflow::CGameflow()
{
    memset(this->m_currentLevelName, 0, LEVEL_NAME_MAX_LEN);
    memset(this->m_currentLevelPath, 0, MAX_ENGINE_PATH);
    this->m_currentGameID = 0;
    this->m_currentLevelID = 0;
    this->m_nextAction = false;
    memset(this->m_actions, TR_GAMEFLOW_NOENTRY, TR_GAMEFLOW_MAX_SECRETS);
    memset(this->m_secretsTriggerMap, 0, TR_GAMEFLOW_MAX_SECRETS);
}


CGameflow::~CGameflow()
{
    memset(this->m_currentLevelName, 0, LEVEL_NAME_MAX_LEN);
    memset(this->m_currentLevelPath, 0, MAX_ENGINE_PATH);
    this->m_currentGameID = 0;
    this->m_currentLevelID = 0;
    this->m_nextAction = false;
    memset(this->m_actions, TR_GAMEFLOW_NOENTRY, TR_GAMEFLOW_MAX_SECRETS);
    memset(this->m_secretsTriggerMap, 0, TR_GAMEFLOW_MAX_SECRETS);
}


void CGameflow::Do()
{
    if(!this->m_nextAction)
        return;

    bool completed = true;

    for(int i = 0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        switch(this->m_actions[i].m_opcode)
        {
            case TR_GAMEFLOW_OP_LEVELCOMPLETE:
                {
                    int top = lua_gettop(engine_lua);
                    lua_getglobal(engine_lua, "getNextLevel");                       // Must be loaded from gameflow script!
                    lua_pushnumber(engine_lua, this->m_currentGameID);      // Push the 1st argument
                    lua_pushnumber(engine_lua, this->m_currentLevelID);     // Push the 2nd argument
                    lua_pushnumber(engine_lua, this->m_actions[i].m_operand); // Push the 3rd argument

                    if (lua_CallAndLog(engine_lua, 3, 3, 0))
                    {
                        this->m_currentLevelID = lua_tonumber(engine_lua, -1);   // First value in stack is level id
                        strncpy(this->m_currentLevelName, lua_tostring(engine_lua, -2), LEVEL_NAME_MAX_LEN); // Second value in stack is level name
                        strncpy(this->m_currentLevelPath, lua_tostring(engine_lua, -3), MAX_ENGINE_PATH); // Third value in stack is level path
                        Engine_LoadMap(this->m_currentLevelPath);
                    }
                    else
                    {
                        Con_AddLine("Fatal Error: Failed to call GetNextLevel()", FONTSTYLE_CONSOLE_WARNING);
                    }
                    lua_settop(engine_lua, top);
                    this->m_actions[i].m_opcode = TR_GAMEFLOW_NOENTRY;
                }
                break;

            case TR_GAMEFLOW_NOENTRY:
                continue;

            default:
                this->m_actions[i].m_opcode = TR_GAMEFLOW_NOENTRY;
                break;  ///@FIXME: Implement all other gameflow opcodes here!
        };   // end switch(gameflow_manager.Operand)
        completed = false;
    }

    if(completed) this->m_nextAction = false;    // Reset action marker!
}


bool CGameflow::Send(int opcode, int operand)
{
    for(int i = 0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        if(this->m_actions[i].m_opcode == opcode) return false;

        if(this->m_actions[i].m_opcode == TR_GAMEFLOW_NOENTRY)
        {
            this->m_actions[i].m_opcode   = opcode;
            this->m_actions[i].m_operand  = operand;
            this->m_nextAction            = true;
            return true;
        }
    }
    return false;
}


uint8_t CGameflow::getCurrentGameID()
{
    return this->m_currentGameID;
}


uint8_t CGameflow::getCurrentLevelID()
{
    return this->m_currentLevelID;
}


const char* CGameflow::getCurrentLevelName()
{
    return this->m_currentLevelName;
}


const char* CGameflow::getCurrentLevelPath()
{
    return this->m_currentLevelPath;
}


void CGameflow::setCurrentLevelName(const char* levelName)
{
    assert(strlen(levelName) < LEVEL_NAME_MAX_LEN);
    strncpy(this->m_currentLevelName, levelName, LEVEL_NAME_MAX_LEN);
}


void CGameflow::setCurrentLevelPath(const char* filePath)
{
    assert(strlen(filePath) < LEVEL_NAME_MAX_LEN);
    strncpy(this->m_currentLevelPath, filePath, MAX_ENGINE_PATH);
}


void CGameflow::resetSecrets()
{
    memset(this->m_secretsTriggerMap, 0, TR_GAMEFLOW_MAX_SECRETS);
}


char CGameflow::getSecretStateAtIndex(int index)
{
    assert((index >= 0) && index <= (TR_GAMEFLOW_MAX_SECRETS));
    return this->m_secretsTriggerMap[index];
}

void CGameflow::setSecretStateAtIndex(int index, int value)
{
    assert((index >= 0) && index <= (TR_GAMEFLOW_MAX_SECRETS));
    this->m_secretsTriggerMap[index] = (char)value; ///@FIXME should not cast.
}

void CGameflow::setCurrentGameID(int gameID)
{
    assert(gameID >= 0 && gameID <= GAME_5);
    this->m_currentGameID = gameID;
}

void CGameflow::setCurrentLevelID(int levelID)
{
    assert(levelID >= 0);///@TODO pull max level ID from script?
    this->m_currentLevelID = levelID;
}
