
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "script.h"

#include "../core/system.h"
#include "../core/gl_text.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "../render/camera.h"
#include "../render/render.h"
#include "../gui/gui.h"
#include "../physics/physics.h"
#include "../vt/tr_versions.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../inventory.h"
#include "../trigger.h"
#include "../room.h"
#include "../entity.h"
#include "../world.h"
#include "../engine.h"
#include "../controls.h"
#include "../game.h"
#include "../gameflow.h"
#include "../character_controller.h"
#include "../engine_string.h"


void Script_DoFlipEffect(lua_State *lua, int id_effect, int id_object, int param)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, "doFlipEffect");
    if(lua_isfunction(lua, -1))
    {
        lua_pushinteger(lua, id_effect);
        lua_pushinteger(lua, id_object);
        lua_pushinteger(lua, param);
        if(lua_pcall(lua, 3, 0, 0) == LUA_OK)
        {
           //
        }
    }
    lua_settop(lua, top);
}


size_t Script_GetFlipEffectsSaveData(lua_State *lua, char *buf, size_t buf_size)
{
    int top = lua_gettop(lua);
    size_t ret = 0;

    lua_getglobal(lua, "onSaveFlipEffects");
    if(lua_isfunction(lua, -1))
    {
        if((lua_pcall(lua, 0, 1, 0) == LUA_OK) && lua_isstring(lua, -1))
        {
            strncpy(buf, lua_tolstring(lua, -1, &ret), buf_size);
        }
    }
    lua_settop(lua, top);

    return ret;
}


int lua_GetLevelVersion(lua_State *lua)
{
    lua_pushinteger(lua, World_GetVersion());
    return 1;
}


/*
 * Load level functions
 */
int lua_SetSectorFloorConfig(lua_State * lua)
{
    if(lua_gettop(lua) >= 10)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs)
        {
            if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
            if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
            if(!lua_isnil(lua, 6))  rs->floor = lua_tonumber(lua, 6);
            rs->floor_corners[0][2] = lua_tonumber(lua, 7);
            rs->floor_corners[1][2] = lua_tonumber(lua, 8);
            rs->floor_corners[2][2] = lua_tonumber(lua, 9);
            rs->floor_corners[3][2] = lua_tonumber(lua, 10);
        }
        else
        {
            Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("Wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, floor, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SetSectorCeilingConfig(lua_State * lua)
{
    if(lua_gettop(lua) >= 10)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs)
        {
            if(!lua_isnil(lua, 4))  rs->ceiling_penetration_config = lua_tointeger(lua, 4);
            if(!lua_isnil(lua, 5))  rs->ceiling_diagonal_type = lua_tointeger(lua, 5);
            if(!lua_isnil(lua, 6))  rs->ceiling = lua_tonumber(lua, 6);
            rs->ceiling_corners[0][2] = lua_tonumber(lua, 7);
            rs->ceiling_corners[1][2] = lua_tonumber(lua, 8);
            rs->ceiling_corners[2][2] = lua_tonumber(lua, 9);
            rs->ceiling_corners[3][2] = lua_tonumber(lua, 10);
        }
        else
        {
            Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, ceiling, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SetSectorPortal(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs)
        {
            rs->portal_to_room = World_GetRoomByID(lua_tointeger(lua, 4));
        }
        else
        {
            Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, portal_room_id)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SetSectorFlags(lua_State * lua)
{
    if(lua_gettop(lua) >= 7)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs)
        {
            if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
            if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
            if(!lua_isnil(lua, 6))  rs->ceiling_penetration_config = lua_tointeger(lua, 6);
            if(!lua_isnil(lua, 7))  rs->ceiling_diagonal_type = lua_tointeger(lua, 7);
        }
        else
        {
            Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, fp_flag, ft_flag, cp_flag, ct_flag)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SameRoom(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent1 = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p ent2 = World_GetEntityByID(lua_tointeger(lua, 2));
        lua_pushboolean(lua, ent1 && ent2 && (ent1->self->room == ent2->self->room));
        return 1;
    }
    else
    {
        Con_Warning("sameRoom: expecting arguments (ent_id1, ent_id2)");
    }

    return 0;
}


int lua_SimilarSector(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 5)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            float dx = lua_tonumber(lua, 2);
            float dy = lua_tonumber(lua, 3);
            float dz = lua_tonumber(lua, 4);

            float next_pos[3];

            next_pos[0] = ent->transform[12 + 0] + (dx * ent->transform[0 + 0] + dy * ent->transform[4 + 0] + dz * ent->transform[8 + 0]);
            next_pos[1] = ent->transform[12 + 1] + (dx * ent->transform[0 + 1] + dy * ent->transform[4 + 1] + dz * ent->transform[8 + 1]);
            next_pos[2] = ent->transform[12 + 2] + (dx * ent->transform[0 + 2] + dy * ent->transform[4 + 2] + dz * ent->transform[8 + 2]);

            room_sector_p next_sector = Room_GetSectorRaw(ent->self->sector->owner_room, next_pos);
            next_sector = Sector_GetPortalSectorTargetRaw(next_sector);

            bool ignore_doors = lua_toboolean(lua, 5);

            if((top >= 6) && lua_toboolean(lua, 6))
            {
                lua_pushboolean(lua, Sectors_SimilarCeiling(ent->self->sector, next_sector, ignore_doors));
            }
            else
            {
                lua_pushboolean(lua, Sectors_SimilarFloor(ent->self->sector, next_sector, ignore_doors));
            }

            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("similarSector: expecting arguments (entity_id, dx, dy, dz, ignore_doors, (ceiling))");
    }

    return 0;
}


int lua_GetSectorHeight(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            bool ceiling = (top > 1) ? (lua_toboolean(lua, 2)) : (false);
            float pos[3] = {ent->transform[12 + 0], ent->transform[12 + 1], ent->transform[12 + 2]};

            if(top >= 5)
            {
                float dx = lua_tonumber(lua, 3);
                float dy = lua_tonumber(lua, 4);
                float dz = lua_tonumber(lua, 5);

                pos[0] += dx * ent->transform[0 + 0] + dy * ent->transform[4 + 0] + dz * ent->transform[8 + 0];
                pos[1] += dx * ent->transform[0 + 1] + dy * ent->transform[4 + 1] + dz * ent->transform[8 + 1];
                pos[2] += dx * ent->transform[0 + 2] + dy * ent->transform[4 + 2] + dz * ent->transform[8 + 2];
            }

            (ceiling) ? (Sector_LowestCeilingCorner(ent->self->sector, pos)) : (Sector_HighestFloorCorner(ent->self->sector, pos));

            lua_pushnumber(lua, pos[2]);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getSectorHeight: expecting arguments (entity_id, (ceiling), (dx, dy, dz))");
    }

    return 0;
}


int lua_SectorTriggerClear(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs)
        {
            if(rs->trigger)
            {
                for(trigger_command_p current_command = rs->trigger->commands; current_command; )
                {
                    trigger_command_p next_command = current_command->next;
                    current_command->next = NULL;
                    free(current_command);
                    current_command = next_command;
                }
                free(rs->trigger);
                rs->trigger = NULL;
            }
        }
        else
        {
            Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("sectorTriggerClear: wrong arguments number, must be (room_id, index_x, index_y)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SectorAddTrigger(lua_State * lua)
{
    if(lua_gettop(lua) >= 8)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);

        if(rs && !rs->trigger)
        {
            rs->trigger = (trigger_header_p)malloc(sizeof(trigger_header_t));
            rs->trigger->commands = NULL;
            rs->trigger->function_value = lua_tointeger(lua, 4);
            rs->trigger->sub_function = lua_tointeger(lua, 5);
            rs->trigger->mask = lua_tointeger(lua, 6);
            rs->trigger->once = lua_tointeger(lua, 7);
            rs->trigger->timer = lua_tointeger(lua, 8);
        }
        else
        {
            Con_AddLine("wrong sector info or sector trigger already exists", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("sectorAddTrigger: wrong arguments number, must be (room_id, index_x, index_y, function, sub_function, mask, once, timer)", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_SectorAddTriggerCommand(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 6)
    {
        int id = lua_tointeger(lua, 1);
        int sx = lua_tointeger(lua, 2);
        int sy = lua_tointeger(lua, 3);
        room_sector_p rs = World_GetRoomSector(id, sx, sy);
        if(rs && rs->trigger)
        {
            trigger_command_p cmd = (trigger_command_p)malloc(sizeof(trigger_command_t));

            cmd->next = NULL;
            cmd->function = lua_tointeger(lua, 4);
            cmd->operands = lua_tointeger(lua, 5);
            cmd->once = lua_tointeger(lua, 6);
            cmd->camera.index = 0;
            cmd->camera.move = 0;
            cmd->camera.timer = 0;
            cmd->camera.unused = 0;

            if(top >= 9)
            {
                cmd->camera.index = lua_tointeger(lua, 7);
                cmd->camera.move =  lua_tointeger(lua, 8);
                cmd->camera.timer = lua_tointeger(lua, 9);
                cmd->camera.unused = 0;
            }

            trigger_command_p *last = &rs->trigger->commands;
            for(; *last; last = &(*last)->next);
            *last = cmd;
        }
        else
        {
            Con_AddLine("wrong sector info or sector has no base trigger", FONTSTYLE_CONSOLE_WARNING);
        }
    }
    else
    {
        Con_AddLine("sectorAddTriggerCommand: wrong arguments number, must be (room_id, index_x, index_y, function, operands, once, (cam_index, cam_move, cam_timer))", FONTSTYLE_CONSOLE_WARNING);
    }

    return 0;
}


int lua_GetGravity(lua_State * lua)
{
    float g[3];
    Physics_GetGravity(g);
    lua_pushnumber(lua, g[0]);
    lua_pushnumber(lua, g[1]);
    lua_pushnumber(lua, g[2]);

    return 3;
}


int lua_SetGravity(lua_State * lua)                                             // function to be exported to Lua
{
    float g[3];

    switch(lua_gettop(lua))
    {
        case 0:                                                                 // get value
            Physics_GetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        case 1:                                                                 // set z only value
            g[0] = 0.0;
            g[1] = 0.0;
            g[2] = lua_tonumber(lua, 1);
            Physics_SetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        case 3:                                                                 // set xyz value
            g[0] = lua_tonumber(lua, 1);
            g[1] = lua_tonumber(lua, 2);
            g[2] = lua_tonumber(lua, 3);
            Physics_SetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        default:
            Con_Warning("setGravity: expecting arguments none or (oz), or (ox, oy, oz)");
            break;
    };

    return 0;
}


int lua_GetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int secret_number = lua_tointeger(lua, 1);
        if((secret_number <= GF_MAX_SECRETS) && (secret_number >= 0))
        {
            lua_pushinteger(lua, Gameflow_GetSecretStateAtIndex(secret_number));
            return 1;
        }
    }
    return 0;   // No parameter specified - return
}


int lua_SetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int secret_number = lua_tointeger(lua, 1);
        if((secret_number <= GF_MAX_SECRETS) && (secret_number >= 0))
        {
            Gameflow_SetSecretStateAtIndex(secret_number, lua_tointeger(lua, 2));
        }
    }
    return 0;
}


int lua_AddRoomToOverlappedList(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        room_p r0 = World_GetRoomByID(lua_tointeger(lua, 1));
        room_p r1 = World_GetRoomByID(lua_tointeger(lua, 2));
        if(r0 && r1 && (r0 != r1) && !Room_IsInOverlappedRoomsList(r0, r1))
        {
            room_p *old_list = r0->content->overlapped_room_list;
            room_p *new_list = (room_p*)malloc((r0->content->overlapped_room_list_size + 1) * sizeof(room_p));
            for(uint16_t i = 0; i < r0->content->overlapped_room_list_size; ++i)
            {
                new_list[i] = r0->content->overlapped_room_list[i];
            }
            new_list[r0->content->overlapped_room_list_size] = r1->real_room;
            r0->content->overlapped_room_list = new_list;
            r0->content->overlapped_room_list_size++;
            if(old_list)
            {
                free(old_list);
            }
        }
    }
    return 0;
}


int lua_AddRoomToNearList(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        room_p r0 = World_GetRoomByID(lua_tointeger(lua, 1));
        room_p r1 = World_GetRoomByID(lua_tointeger(lua, 2));
        if(r0 && r1 && (r0 != r1) && !Room_IsInOverlappedRoomsList(r0, r1))
        {
            room_p *old_list = r0->content->near_room_list;
            room_p *new_list = (room_p*)malloc((r0->content->near_room_list_size + 1) * sizeof(room_p));
            for(uint16_t i = 0; i < r0->content->near_room_list_size; ++i)
            {
                new_list[i] = r0->content->near_room_list[i];
            }
            new_list[r0->content->near_room_list_size] = r1->real_room;
            r0->content->near_room_list = new_list;
            r0->content->near_room_list_size++;
            if(old_list)
            {
                free(old_list);
            }
        }
    }
    return 0;
}


int lua_CreateBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) >= 5)
    {
        int item_id         = lua_tointeger(lua, 1);
        int model_id        = lua_tointeger(lua, 2);
        int world_model_id  = lua_tointeger(lua, 3);
        int type            = lua_tointeger(lua, 4);
        int count           = lua_tointeger(lua, 5);

        World_CreateItem(item_id, model_id, world_model_id, type, count, lua_tostring(lua, 6));
    }
    else
    {
        Con_Warning("createBaseItem: expecting arguments (item_id, model_id, world_model_id, type, count, (name))");
    }

    return 0;
}


int lua_DeleteBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        World_DeleteItem(lua_tointeger(lua, 1));
    }
    else
    {
        Con_Warning("deleteBaseItem: expecting arguments (item_id)");
    }
    return 0;
}


int lua_GetBaseItemInfo(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        base_item_p item = World_GetBaseItemByID(lua_tointeger(lua, 1));
        if(item)
        {
            lua_pushinteger(lua, item->count);
            lua_pushinteger(lua, item->type);
            lua_pushinteger(lua, item->world_model_id);
            lua_pushstring(lua, item->name);
            return 4;
        }
        else
        {
            Con_Warning("no base item with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getBaseItemInfo: expecting arguments (item_id)");
    }

    return 0;
}


int lua_GetBaseItemInfoByWorldID(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        base_item_p item = World_GetBaseItemByWorldModelID(lua_tointeger(lua, 1));
        if(item)
        {
            lua_pushinteger(lua, item->count);
            lua_pushinteger(lua, item->type);
            lua_pushinteger(lua, item->id);
            lua_pushstring(lua, item->name);
            return 4;
        }
        else
        {
            Con_Warning("no base item with world_model_id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getBaseItemInfoByWorldID: expecting arguments (world_model_id)");
    }

    return 0;
}


int lua_SpawnEntity(lua_State * lua)
{
    int top = lua_gettop(lua);
    if(top >= 8)
    {
        int model_id = lua_tointeger(lua, 1);
        int room_id = lua_tointeger(lua, 2);
        int32_t ov_id = -1;
        float pos[3], ang[3];

        pos[0] = lua_tonumber(lua, 3);
        pos[1] = lua_tonumber(lua, 4);
        pos[2] = lua_tonumber(lua, 5);
        ang[0] = lua_tonumber(lua, 6);
        ang[1] = lua_tonumber(lua, 7);
        ang[2] = lua_tonumber(lua, 8);

        if(top >= 9)
        {
            ov_id = lua_tointeger(lua, 9);
        }

        uint32_t id = World_SpawnEntity(model_id, room_id, pos, ang, ov_id);
        if(id == ENTITY_ID_NONE)
        {
            lua_pushnil(lua);
        }
        else
        {
            lua_pushinteger(lua, id);
        }

        return 1;
    }
    else
    {
        Con_Warning("spawnEntity: expecting arguments (model_id1, room_id, x, y, z, ax, ay, az, (ov_id))");
    }

    return 0;
}


int lua_DeleteEntity(lua_State * lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        World_DeleteEntity(lua_tointeger(lua, 1));
    }
    else
    {
        Con_Warning("deleteEntity: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetLevel(lua_State *lua)
{
    lua_pushinteger(lua, Gameflow_GetCurrentLevelID());
    return 1;
}


int lua_GameflowSend(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        int opcode  = lua_tointeger(lua, 1);
        int id  = lua_tointeger(lua, 2);
        if(!Gameflow_Send(opcode, id))
        {
            Con_Warning("gameflowSend: Failed to add opcode to gameflow action list");
        }
    }
    else
    {
        Con_Warning("setLevel: expecting arguments (level_id)");
    }

    return 0;
}


int lua_SetGame(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        Gameflow_SetCurrentGameID(lua_tointeger(lua, 1));
        if(!lua_isnil(lua, 2))
        {
            Gameflow_SetCurrentLevelID(lua_tointeger(lua, 2));
        }

        lua_getglobal(lua, "getTitleScreen");
        if(lua_isfunction(lua, -1))
        {
            lua_pushnumber(lua, Gameflow_GetCurrentGameID());
            if(lua_CallAndLog(lua, 1, 1, 0))
            {
                //Gui_FadeAssignPic(FADER_LOADSCREEN, lua_tostring(lua, -1));
                lua_pop(lua, 1);
            }
        }
        lua_settop(lua, top);

        Con_Notify("level was changed to %d", Gameflow_GetCurrentGameID());
        Game_LevelTransition(Gameflow_GetCurrentLevelID());

        if(!Gameflow_Send(GF_OP_LEVELCOMPLETE, Gameflow_GetCurrentLevelID()))
        {
            Con_Warning("setGame: Failed to add opcode to gameflow action list");
        }
    }
    else
    {
        Con_Warning("setGame: expecting arguments (gameversion, (level_id))");
    }

    return 0;
}


int lua_LoadMap(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        if(lua_isstring(lua, 1))
        {
            const char *s = lua_tostring(lua, 1);
            if(s && s[0])
            {
                if(!lua_isnil(lua, 2))
                {
                    Gameflow_SetCurrentGameID(lua_tointeger(lua, 2));
                }
                if(!lua_isnil(lua, 3))
                {
                    Gameflow_SetCurrentLevelID(lua_tointeger(lua, 3));
                }
                char file_path[MAX_ENGINE_PATH];
                Script_GetLoadingScreen(lua, Gameflow_GetCurrentLevelID(), file_path);
                if(!Gui_LoadScreenAssignPic(file_path))
                {
                    Gui_LoadScreenAssignPic("resource/graphics/legal");
                }
                Engine_LoadMap(s);
            }
        }
    }
    else
    {
        Con_Warning("loadMap: expecting arguments (map_name, (game_id, map_id))");
    }

    return 0;
}


/*
 * Flipped (alternate) room functions
 */
int lua_SetGlobalFlipState(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        World_SetGlobalFlipState(lua_tointeger(lua, 1));
    }
    else
    {
        Con_Warning("setGlobalFlipState: expecting arguments (flip_state)");
    }

    return 0;
}


int lua_SetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint32_t flip_state = (uint32_t)lua_tointeger(lua, 2);
        World_SetFlipState(flip_index, flip_state);
    }
    else
    {
        Con_Warning("setFlipState: expecting arguments (flip_index, flip_state)");
    }

    return 0;
}


int lua_SetPlayer(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        World_SetPlayer(World_GetEntityByID(lua_tointeger(lua, 1)));
    }
    else
    {
        Con_Warning("setPlayer: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint8_t  flip_mask = (uint32_t)lua_tointeger(lua, 2);
        uint8_t  flip_operation = (uint32_t)lua_tointeger(lua, 3);
        World_SetFlipMap(flip_index, flip_mask, flip_operation);
    }
    else
    {
        Con_Warning("setFlipMap: expecting arguments (flip_index, flip_mask, flip_operation)");
    }

    return 0;
}


int lua_GetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint32_t flip_map = World_GetFlipMap(flip_index);

        if(flip_map != 0xFFFFFFFF)
        {
            lua_pushinteger(lua, flip_map);
            return 1;
        }
        else
        {
            Con_Warning("wrong flipmap index");
        }
    }
    else
    {
        Con_Warning("getFlipState: expecting arguments (flip_index)");
    }

    return 0;
}


int lua_GetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint32_t flip_state = World_GetFlipState(flip_index);

        if(flip_state != 0xFFFFFFFF)
        {
            lua_pushinteger(lua, flip_state);
            return 1;
        }
        else
        {
            Con_Warning("wrong flipmap index");
        }
    }
    else
    {
        Con_Warning("getFlipState: expecting arguments (flip_index)");
    }

    return 0;
}

int lua_SetRoomActiveContent(lua_State *lua)
{
    if(lua_gettop(lua) == 2)
    {
        room_p r1 = World_GetRoomByID(lua_tointeger(lua, 1));
        room_p r2 = World_GetRoomByID(lua_tointeger(lua, 2));

        if(r1 && r2 && (r1->content->original_room_id != r2->id))
        {
            Room_SetActiveContent(r1, r2);
        }
    }
    else
    {
        Con_Warning("setRoomActiveContent: expecting arguments (room1_id, room2_id)");
    }

    return 0;
}


int lua_SetBoxBlocked(lua_State *lua)
{
    if(lua_gettop(lua) == 2)
    {
        room_box_p box = World_GetRoomBoxByID(lua_tointeger(lua, 1));
        if(box && box->is_blockable)
        {
            box->is_blocked = lua_toboolean(lua, 2);
        }
    }
    else
    {
        Con_Warning("setBoxBlocked: expecting arguments (box_id, value)");
    }

    return 0;
}

/*
 * Generate UV rotate animations
 */
int lua_genUVRotateAnimation(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
        if(model)
        {
            polygon_p p = model->mesh_tree->mesh_base->transparency_polygons;
            if((p) && (p->anim_id == 0))
            {
                anim_seq_t seq;
                anim_seq_p tmp_seq;
                uint32_t seqs_conut;
                World_GetAnimSeqInfo(&tmp_seq, &seqs_conut);

                // Fill up new sequence with frame list.
                seq.anim_type         = TR_ANIMTEXTURE_FORWARD;
                seq.frame_lock        = false;         // by default anim is playing
                seq.uvrotate          = true;
                seq.reverse_direction = false;         // Needed for proper reverse-type start-up.
                seq.frame_rate        = 0.025 * 16;    // Should be passed as 1 / FPS.
                seq.frame_time        = 0.0;           // Reset frame time to initial state.
                seq.current_frame     = 0;             // Reset current frame to zero.
                seq.frames_count      = 1;
                seq.frame_list        = (uint32_t*)calloc(seq.frames_count, sizeof(uint32_t));
                seq.frame_list[0]     = 0;
                seq.frames            = (tex_frame_p)calloc(seq.frames_count, sizeof(tex_frame_t));

                float v_min, v_max;
                v_min = v_max = p->vertices->tex_coord[1];
                for(uint16_t j = 1; j < p->vertex_count; j++)
                {
                    if(p->vertices[j].tex_coord[1] > v_max)
                    {
                        v_max = p->vertices[j].tex_coord[1];
                    }
                    if(p->vertices[j].tex_coord[1] < v_min)
                    {
                        v_min = p->vertices[j].tex_coord[1];
                    }
                }

                seq.frames->uvrotate_max = 0.5 * (v_max - v_min);
                seq.frames->current_uvrotate = 0.0f;
                seq.frames->texture_index = p->texture_index;
                seq.frames->mat[0] = 1.0;
                seq.frames->mat[1] = 0.0;
                seq.frames->mat[2] = 0.0;
                seq.frames->mat[3] = 1.0;
                seq.frames->move[0] = 0.0;
                seq.frames->move[1] = 0.0;

                for(; p; p = p->next)
                {
                    p->anim_id = seqs_conut + 1;
                    for(uint16_t j = 0; j < p->vertex_count; j++)
                    {
                        p->vertices[j].tex_coord[1] = v_min + 0.5 * (p->vertices[j].tex_coord[1] - v_min) + seq.frames->uvrotate_max;
                    }
                }

                if(!World_AddAnimSeq(&seq))
                {
                    if(seq.frames_count)
                    {
                        free(seq.frame_list);
                        seq.frame_list = NULL;
                        free(seq.frames);
                        seq.frames = NULL;
                    }
                    seq.frames_count = 0;
                }
            }
        }
    }
    else
    {
        Con_Warning("genUVRotateAnimation: expecting arguments (model_id)");
    }

    return 0;
}


/*
 * Camera functions
 */
int lua_CamShake(lua_State *lua)
{
    /*if(lua_gettop(lua) >= 2)
    {
        float power = lua_tonumber(lua, 1);
        float time  = lua_tonumber(lua, 2);
        Cam_Shake(&engine_camera, power, time);
    }*/
    return 0;
}


int lua_PlayFlyby(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        uint32_t flyby_id   = lua_tointeger(lua, 1);
        int      once       = lua_tointeger(lua, 2);
        Game_PlayFlyBy(flyby_id, once);
    }

    return 0;
}


int lua_FlashSetup(lua_State *lua)
{
    /*if(lua_gettop(lua) != 6) return 0;

    Gui_FadeSetup(FADER_EFFECT,
                  (uint8_t)(lua_tointeger(lua, 1)),
                  (uint8_t)(lua_tointeger(lua, 2)), (uint8_t)(lua_tointeger(lua, 3)), (uint8_t)(lua_tointeger(lua, 4)),
                  BM_MULTIPLY,
                  (uint16_t)(lua_tointeger(lua, 5)), (uint16_t)(lua_tointeger(lua, 6)));*/
    return 0;
}


int lua_FlashStart(lua_State *lua)
{
    //Gui_FadeStart(FADER_EFFECT, GUI_FADER_DIR_TIMED);
    return 0;
}


void Script_LuaRegisterWorldFuncs(lua_State *lua)
{
    lua_register(lua, "getLevelVersion", lua_GetLevelVersion);
    lua_register(lua, "gameflowSend", lua_GameflowSend);
    lua_register(lua, "getLevel", lua_GetLevel);

    lua_register(lua, "setSectorFloorConfig", lua_SetSectorFloorConfig);
    lua_register(lua, "setSectorCeilingConfig", lua_SetSectorCeilingConfig);
    lua_register(lua, "setSectorPortal", lua_SetSectorPortal);
    lua_register(lua, "setSectorFlags", lua_SetSectorFlags);

    lua_register(lua, "setGame", lua_SetGame);
    lua_register(lua, "loadMap", lua_LoadMap);

    lua_register(lua, "setPlayer", lua_SetPlayer);
    lua_register(lua, "setFlipMap", lua_SetFlipMap);
    lua_register(lua, "getFlipMap", lua_GetFlipMap);
    lua_register(lua, "setGlobalFlipState", lua_SetGlobalFlipState);
    lua_register(lua, "setFlipState", lua_SetFlipState);
    lua_register(lua, "getFlipState", lua_GetFlipState);
    lua_register(lua, "setRoomActiveContent", lua_SetRoomActiveContent);
    lua_register(lua, "setBoxBlocked", lua_SetBoxBlocked);

    lua_register(lua, "createBaseItem", lua_CreateBaseItem);
    lua_register(lua, "deleteBaseItem", lua_DeleteBaseItem);
    lua_register(lua, "getBaseItemInfo", lua_GetBaseItemInfo);
    lua_register(lua, "getBaseItemInfoByWorldID", lua_GetBaseItemInfoByWorldID);
    lua_register(lua, "spawnEntity", lua_SpawnEntity);
    lua_register(lua, "deleteEntity", lua_DeleteEntity);

    lua_register(lua, "sameRoom", lua_SameRoom);
    lua_register(lua, "similarSector", lua_SimilarSector);
    lua_register(lua, "getSectorHeight", lua_GetSectorHeight);
    lua_register(lua, "sectorTriggerClear", lua_SectorTriggerClear);
    lua_register(lua, "sectorAddTrigger", lua_SectorAddTrigger);
    lua_register(lua, "sectorAddTriggerCommand", lua_SectorAddTriggerCommand);

    lua_register(lua, "getSecretStatus", lua_GetSecretStatus);
    lua_register(lua, "setSecretStatus", lua_SetSecretStatus);
    lua_register(lua, "addRoomToOverlappedList", lua_AddRoomToOverlappedList);
    lua_register(lua, "addRoomToNearList", lua_AddRoomToNearList);

    lua_register(lua, "genUVRotateAnimation", lua_genUVRotateAnimation);

    lua_register(lua, "getGravity", lua_GetGravity);
    lua_register(lua, "setGravity", lua_SetGravity);

    lua_register(lua, "camShake", lua_CamShake);
    lua_register(lua, "playFlyby", lua_PlayFlyby);
    lua_register(lua, "flashSetup", lua_FlashSetup);
    lua_register(lua, "flashStart", lua_FlashStart);
}
