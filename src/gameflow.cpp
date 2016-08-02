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
    memset(this->m_actions, GF_NOENTRY, GF_MAX_ACTIONS);
    memset(this->m_secretsTriggerMap, 0, GF_MAX_SECRETS);
}


CGameflow::~CGameflow()
{
    memset(this->m_currentLevelName, 0, LEVEL_NAME_MAX_LEN);
    memset(this->m_currentLevelPath, 0, MAX_ENGINE_PATH);
    this->m_currentGameID = 0;
    this->m_currentLevelID = 0;
    memset(this->m_actions, 0, GF_MAX_ACTIONS);
    memset(this->m_secretsTriggerMap, 0, GF_MAX_SECRETS);
}


void CGameflow::Do()
{
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        switch(this->m_actions[i].m_opcode)
        {
            case GF_OP_LEVELCOMPLETE:
            {
                int top = lua_gettop(engine_lua);

                //Must be loaded from gameflow script!
                lua_getglobal(engine_lua, "getNextLevel");

                //Push the 3 arguments require dby getNextLevel();
                lua_pushnumber(engine_lua, this->m_currentGameID);
                lua_pushnumber(engine_lua, this->m_currentLevelID);
                lua_pushnumber(engine_lua, this->m_actions[i].m_operand);

                if (lua_CallAndLog(engine_lua, 3, 3, 0))
                {
                    //First value in stack is level id
                    this->m_currentLevelID = lua_tonumber(engine_lua, -1);
                    //Second value in stack is level name
                    strncpy(this->m_currentLevelName, lua_tostring(engine_lua, -2), LEVEL_NAME_MAX_LEN);
                    //Third value in stack is level path
                    strncpy(this->m_currentLevelPath, lua_tostring(engine_lua, -3), MAX_ENGINE_PATH);
                    Engine_LoadMap(this->m_currentLevelPath);
                }
                else
                {
                    Con_AddLine("Fatal Error: Failed to call GetNextLevel()", FONTSTYLE_CONSOLE_WARNING);
                }
                lua_settop(engine_lua, top);
                this->m_actions[i].m_opcode = GF_NOENTRY;
            }
            break;
            case GF_NOENTRY:
                continue;
            default:
                Con_Printf("Unimplemented gameflow opcode: %i", this->m_actions[i].m_opcode);
                break;
        };   // end switch(gameflow_manager.Operand)
    }
}


bool CGameflow::Send(int opcode, int operand)
{
    //Iterate through our action list until we have a free space (TR_GAMEFLOW_NOENTRY) to add the next action.
    for(int i = 0; i < GF_MAX_ACTIONS; i++)
    {
        if(this->m_actions[i].m_opcode == GF_NOENTRY)
        {
            this->m_actions[i].m_opcode = opcode;
            this->m_actions[i].m_operand = operand;
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
    memset(this->m_secretsTriggerMap, 0, GF_MAX_SECRETS);
}


char CGameflow::getSecretStateAtIndex(int index)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
    return this->m_secretsTriggerMap[index];
}

void CGameflow::setSecretStateAtIndex(int index, int value)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
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
