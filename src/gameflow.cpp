extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/gl_text.h"
#include "core/console.h"
#include "script/script.h"
#include "gui/gui.h"
#include "engine.h"
#include "gameflow.h"
#include "game.h"
#include "audio.h"
#include "world.h"

#include <assert.h>
#include <string.h>
#include <vector>

typedef struct gameflow_action_s
{
    int16_t      m_opcode;
    uint16_t     m_operand;
} gameflow_action_t;

struct gameflow_s
{
    int                             m_currentGameID;
    int                             m_currentLevelID;

    int                             m_nextGameID;
    int                             m_nextLevelID;

    char                            m_currentLevelName[LEVEL_NAME_MAX_LEN];
    char                            m_currentLevelPath[MAX_ENGINE_PATH];
    char                            m_secretsTriggerMap[GF_MAX_SECRETS];

    std::vector<gameflow_action_t>    m_actions;
} global_gameflow;

typedef struct level_info_s
{
    int num_levels = 0;
    char name[LEVEL_NAME_MAX_LEN];
    char path[MAX_ENGINE_PATH];
    char pic[MAX_ENGINE_PATH];
}level_info_t, *level_info_p;


bool Gameflow_GetLevelInfo(level_info_p info, int game_id, int level_id);
bool Gameflow_SetGameInternal(int game_id, int level_id);


void Gameflow_Init()
{
    global_gameflow.m_nextGameID = -1;
    global_gameflow.m_nextLevelID = -1;
    memset(global_gameflow.m_currentLevelName, 0, sizeof(global_gameflow.m_currentLevelName));
    memset(global_gameflow.m_currentLevelPath, 0, sizeof(global_gameflow.m_currentLevelPath));
    memset(global_gameflow.m_secretsTriggerMap, 0, sizeof(global_gameflow.m_secretsTriggerMap));
    global_gameflow.m_actions.clear();
}


bool Gameflow_Send(int opcode, int operand)
{
    gameflow_action_t act;

    act.m_opcode = opcode;
    act.m_operand = operand;
    global_gameflow.m_actions.push_back(act);

    return true;
}


void Gameflow_ProcessCommands()
{
    for(; !Engine_IsVideoPlayed() && !global_gameflow.m_actions.empty(); global_gameflow.m_actions.pop_back())
    {
        gameflow_action_t &it = global_gameflow.m_actions.back();
        switch(it.m_opcode)
        {
            case GF_OP_LEVELCOMPLETE:
                if(World_GetPlayer())
                {
                    luaL_dostring(engine_lua, "saved_inventory = getItems(player);");
                }
                if(Gameflow_SetGameInternal(global_gameflow.m_currentGameID, global_gameflow.m_currentLevelID + 1) && World_GetPlayer())
                {
                    luaL_dostring(engine_lua, "if(saved_inventory ~= nil) then\n"
                                                  "removeAllItems(player);\n"
                                                  "for k, v in pairs(saved_inventory) do\n"
                                                      "addItem(player, k, v);\n"
                                                  "end;\n"
                                                  "saved_inventory = nil;\n"
                                              "end;");
                }
                break;

            case GF_OP_SETTRACK:
                Audio_StreamPlay(it.m_operand);
                break;

            case GF_NOENTRY:
                continue;

            default:
                //Con_Printf("Unimplemented gameflow opcode: %i", global_gameflow.m_actions[i].m_opcode);
                break;
        };   // end switch(gameflow_manager.Operand)
    }

    if(global_gameflow.m_nextGameID >= 0)
    {
        Gameflow_SetGameInternal(global_gameflow.m_nextGameID, global_gameflow.m_nextLevelID);
        global_gameflow.m_nextGameID = -1;
        global_gameflow.m_nextLevelID = -1;
    }
}


bool Gameflow_SetMap(const char* filePath, int game_id, int level_id)
{
    level_info_t info;
    if(Gameflow_GetLevelInfo(&info, game_id, level_id))
    {
        if(!Gui_LoadScreenAssignPic(info.pic))
        {
            Gui_LoadScreenAssignPic("resource/graphics/legal");
        }
    }

    strncpy(global_gameflow.m_currentLevelPath, filePath, MAX_ENGINE_PATH);
    global_gameflow.m_currentGameID = game_id;
    global_gameflow.m_currentLevelID = level_id;

    return Engine_LoadMap(filePath);
}


bool Gameflow_SetGame(int game_id, int level_id)
{
    global_gameflow.m_nextGameID = game_id;
    global_gameflow.m_nextLevelID = level_id;
    return true;
}


bool Gameflow_GetLevelInfo(level_info_p info, int game_id, int level_id)
{
    int top = lua_gettop(engine_lua);

    lua_getglobal(engine_lua, "gameflow_params");
    if(!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return false;
    }

    lua_rawgeti(engine_lua, -1, game_id);
    if(!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return false;
    }

    lua_getfield(engine_lua, -1, "title");
    strncpy(info->pic, lua_tostring(engine_lua, -1), MAX_ENGINE_PATH);
    lua_pop(engine_lua, 1);

    lua_getfield(engine_lua, -1, "numlevels");
    info->num_levels = lua_tointeger(engine_lua, -1);
    lua_pop(engine_lua, 1);

    lua_getfield(engine_lua, -1, "levels");
    if(!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return false;
    }

    level_id = (level_id <= info->num_levels) ? (level_id) : (0);
    lua_rawgeti(engine_lua, -1, level_id);
    if(!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return false;
    }

    lua_getfield(engine_lua, -1, "name");
    strncpy(info->name, lua_tostring(engine_lua, -1), LEVEL_NAME_MAX_LEN);
    lua_pop(engine_lua, 1);

    lua_getfield(engine_lua, -1, "filepath");
    strncpy(info->path, lua_tostring(engine_lua, -1), MAX_ENGINE_PATH);
    lua_pop(engine_lua, 1);

    lua_getfield(engine_lua, -1, "picpath");
    strncpy(info->pic, lua_tostring(engine_lua, -1), MAX_ENGINE_PATH);
    lua_pop(engine_lua, 1);

    lua_pop(engine_lua, 1);   // level_id
    lua_pop(engine_lua, 1);   // levels

    lua_pop(engine_lua, 1);   // game_id
    lua_settop(engine_lua, top);

    return true;
}


bool Gameflow_SetGameInternal(int game_id, int level_id)
{
    level_info_t info;
    if(Gameflow_GetLevelInfo(&info, game_id, level_id))
    {
        if(!Gui_LoadScreenAssignPic(info.pic))
        {
            Gui_LoadScreenAssignPic("resource/graphics/legal");
        }

        global_gameflow.m_currentGameID = game_id;
        global_gameflow.m_currentLevelID = level_id;
        strncpy(global_gameflow.m_currentLevelName, info.name, LEVEL_NAME_MAX_LEN);
        strncpy(global_gameflow.m_currentLevelPath, info.path, MAX_ENGINE_PATH);
        return Engine_LoadMap(info.path);
    }

    return false;
}


const char *Gameflow_GetCurrentLevelPathLocal()
{
    return global_gameflow.m_currentLevelPath + strlen(Engine_GetBasePath());
}


uint8_t Gameflow_GetCurrentGameID()
{
    return global_gameflow.m_currentGameID;
}


uint8_t Gameflow_GetCurrentLevelID()
{
    return global_gameflow.m_currentLevelID;
}


void Gameflow_ResetSecrets()
{
    memset(global_gameflow.m_secretsTriggerMap, 0, GF_MAX_SECRETS * sizeof(*global_gameflow.m_secretsTriggerMap));
}


void Gameflow_SetSecretStateAtIndex(int index, int value)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
    global_gameflow.m_secretsTriggerMap[index] = (char)value; ///@FIXME should not cast.
}


int Gameflow_GetSecretStateAtIndex(int index)
{
    assert((index >= 0) && index <= (GF_MAX_SECRETS));
    return global_gameflow.m_secretsTriggerMap[index];
}