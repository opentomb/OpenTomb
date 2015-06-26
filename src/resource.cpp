
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "gl_util.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

#include "vt/vt_level.h"
#include "audio.h"
#include "world.h"
#include "mesh.h"
#include "entity.h"
#include "gameflow.h"
#include "resource.h"
#include "vmath.h"
#include "polygon.h"
#include "portal.h"
#include "console.h"
#include "frustum.h"
#include "system.h"
#include "game.h"
#include "gui.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "obb.h"
#include "engine.h"
#include "bordered_texture_atlas.h"
#include "render.h"
#include "redblack.h"
#include "bsp_tree.h"
#include "shader_description.h"

lua_State *objects_flags_conf = NULL;
lua_State *ent_ID_override = NULL;
lua_State *level_script = NULL;


void Res_SetEntityModelProperties(struct entity_s *ent)
{
    if((objects_flags_conf != NULL) && (ent->bf.animations.model != NULL))
    {
        int top = lua_gettop(objects_flags_conf);
        assert(top >= 0);
        lua_getglobal(objects_flags_conf, "getEntityModelProperties");
        if(lua_isfunction(objects_flags_conf, -1))
        {
            lua_pushinteger(objects_flags_conf, engine_world.version);              // engine version
            lua_pushinteger(objects_flags_conf, ent->bf.animations.model->id);      // entity model id
            if (lua_CallAndLog(objects_flags_conf, 2, 5, 0))
            {
                ent->self->collision_type = lua_tointeger(objects_flags_conf, -5);      // get collision type flag
                ent->self->collision_shape = lua_tointeger(objects_flags_conf, -4);     // get collision shape flag
                ent->bf.animations.model->hide = lua_tointeger(objects_flags_conf, -3); // get info about model visibility
                ent->type_flags |= lua_tointeger(objects_flags_conf, -2);               // get traverse information

                if(!lua_isnil(objects_flags_conf, -1))
                {
                    Res_CreateEntityFunc(engine_lua, lua_tolstring(objects_flags_conf, -1, 0), ent->id);
                }
            }
        }
        lua_settop(objects_flags_conf, top);
    }

    if((level_script != NULL) && (ent->bf.animations.model != NULL))
    {
        int top = lua_gettop(level_script);
        assert(top >= 0);
        lua_getglobal(level_script, "getEntityModelProperties");
        if(lua_isfunction(level_script, -1))
        {
            lua_pushinteger(level_script, engine_world.version);                // engine version
            lua_pushinteger(level_script, ent->bf.animations.model->id);        // entity model id
            if (lua_CallAndLog(level_script, 2, 5, 0))                          // call that function
            {
                if(!lua_isnil(level_script, -5))
                {
                    ent->self->collision_type = lua_tointeger(level_script, -5);        // get collision type flag
                }
                if(!lua_isnil(level_script, -4))
                {
                    ent->self->collision_shape = lua_tointeger(level_script, -4);       // get collision shape flag
                }
                if(!lua_isnil(level_script, -3))
                {
                    ent->bf.animations.model->hide = lua_tointeger(level_script, -3);   // get info about model visibility
                }
                if(!lua_isnil(level_script, -2))
                {
                    ent->type_flags &= ~(ENTITY_TYPE_TRAVERSE | ENTITY_TYPE_TRAVERSE_FLOOR);
                    ent->type_flags |= lua_tointeger(level_script, -2);                 // get traverse information
                }
                if(!lua_isnil(level_script, -1))
                {
                    size_t string_length;
                    Res_CreateEntityFunc(engine_lua, lua_tolstring(level_script, -1, &string_length), ent->id);
                }
            }
        }
        lua_settop(level_script, top);
    }
}

bool Res_CreateEntityFunc(lua_State *lua, const char* func_name, int entity_id)
{
    if(lua)
    {
        const char* func_template = "%s_init";
        char buf[64] = {0};
        int top = lua_gettop(lua);
        assert(top >= 0);
        snprintf(buf, 64, func_template, func_name);
        lua_getglobal(lua, buf);

        if(lua_isfunction(lua, -1))
        {
            snprintf(buf, 64, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", entity_id, entity_id);
            luaL_loadstring(lua, buf);
            if (lua_CallAndLog(lua, 0, LUA_MULTRET, 0))
            {
                lua_pushinteger(lua, entity_id);
                lua_CallAndLog(lua, 1, 0, 0);
            }
            lua_settop(lua, top);
            return true;
        }
        else
        {
            lua_settop(lua, top);
            return false;
        }
    }
    return false;
}

void Res_SetStaticMeshProperties(struct static_mesh_s *r_static)
{
    if(level_script != NULL)
    {
        int top = lua_gettop(level_script);
        lua_getglobal(level_script, "getStaticMeshProperties");
        if(lua_isfunction(level_script, -1))
        {
            lua_pushinteger(level_script, r_static->object_id);
            if(lua_CallAndLog(level_script, 1, 3, 0))
            {
                if(!lua_isnil(level_script, -3))
                {
                    r_static->self->collision_type = lua_tointeger(level_script, -3);
                }
                if(!lua_isnil(level_script, -2))
                {
                    r_static->self->collision_shape = lua_tointeger(level_script, -2);
                }
                if(!lua_isnil(level_script, -1))
                {
                    r_static->hide = lua_tointeger(level_script, -1);
                }
            }
        }
        lua_settop(level_script, top);
    }
}

/*
 * BASIC SECTOR COLLISION LAYOUT
 *
 *
 *  OY                            OZ
 *  ^   0 ___________ 1            ^  1  ___________  2
 *  |    |           |             |    |           |
 *  |    |           |             |    |   tween   |
 *  |    |   SECTOR  |             |    |  corners  |
 *  |    |           |             |    |           |
 *  |   3|___________|2            |  0 |___________| 3
 *  |-------------------> OX       |--------------------> OXY
 */


void Res_Sector_SetTweenFloorConfig(struct sector_tween_s *tween)
{
    if(tween->floor_corners[0].m_floats[2] > tween->floor_corners[1].m_floats[2])
    {
        btScalar t;
        SWAPT(tween->floor_corners[0].m_floats[2], tween->floor_corners[1].m_floats[2], t);
        SWAPT(tween->floor_corners[2].m_floats[2], tween->floor_corners[3].m_floats[2], t);
    }

    if(tween->floor_corners[3].m_floats[2] > tween->floor_corners[2].m_floats[2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_2TRIANGLES;              // like a butterfly
    }
    else if((tween->floor_corners[0].m_floats[2] != tween->floor_corners[1].m_floats[2]) &&
       (tween->floor_corners[2].m_floats[2] != tween->floor_corners[3].m_floats[2]))
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_QUAD;
    }
    else if(tween->floor_corners[0].m_floats[2] != tween->floor_corners[1].m_floats[2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT;
    }
    else if(tween->floor_corners[2].m_floats[2] != tween->floor_corners[3].m_floats[2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT;
    }
    else
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
    }
}

void Res_Sector_SetTweenCeilingConfig(struct sector_tween_s *tween)
{
    if(tween->ceiling_corners[0].m_floats[2] > tween->ceiling_corners[1].m_floats[2])
    {
        btScalar t;
        SWAPT(tween->ceiling_corners[0].m_floats[2], tween->ceiling_corners[1].m_floats[2], t);
        SWAPT(tween->ceiling_corners[2].m_floats[2], tween->ceiling_corners[3].m_floats[2], t);
    }

    if(tween->ceiling_corners[3].m_floats[2] > tween->ceiling_corners[2].m_floats[2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_2TRIANGLES;            // like a butterfly
    }
    else if((tween->ceiling_corners[0].m_floats[2] != tween->ceiling_corners[1].m_floats[2]) &&
       (tween->ceiling_corners[2].m_floats[2] != tween->ceiling_corners[3].m_floats[2]))
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_QUAD;
    }
    else if(tween->ceiling_corners[0].m_floats[2] != tween->ceiling_corners[1].m_floats[2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT;
    }
    else if(tween->ceiling_corners[2].m_floats[2] != tween->ceiling_corners[3].m_floats[2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT;
    }
    else
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
    }
}

int Res_Sector_IsWall(room_sector_p ws, room_sector_p ns)
{
    if((ws->portal_to_room < 0) && (ns->portal_to_room < 0) && (ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL))
    {
        return 1;
    }

    if((ns->portal_to_room < 0) && (ns->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (ws->portal_to_room >= 0))
    {
        ws = Sector_CheckPortalPointer(ws);
        if((ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) || (0 == Sectors_Is2SidePortals(ns, ws)))
        {
            return 1;
        }
    }

    return 0;
}

///@TODO: resolve floor >> ceiling case
void Res_Sector_GenTweens(struct room_s *room, struct sector_tween_s *room_tween)
{
    for(uint16_t h = 0; h < room->sectors_y-1; h++)
    {
        for(uint16_t w = 0; w < room->sectors_x-1; w++)
        {
            // Init X-plane tween [ | ]

            room_sector_p current_heightmap = room->sectors + (w * room->sectors_y + h);
            room_sector_p next_heightmap    = current_heightmap + 1;
            char joined_floors = 0;
            char joined_ceilings = 0;

            /* XY corners coordinates must be calculated from native room sector */
            room_tween->floor_corners[0].m_floats[1] = current_heightmap->floor_corners[0].m_floats[1];
            room_tween->floor_corners[1].m_floats[1] = room_tween->floor_corners[0].m_floats[1];
            room_tween->floor_corners[2].m_floats[1] = room_tween->floor_corners[0].m_floats[1];
            room_tween->floor_corners[3].m_floats[1] = room_tween->floor_corners[0].m_floats[1];
            room_tween->floor_corners[0].m_floats[0] = current_heightmap->floor_corners[0].m_floats[0];
            room_tween->floor_corners[1].m_floats[0] = room_tween->floor_corners[0].m_floats[0];
            room_tween->floor_corners[2].m_floats[0] = current_heightmap->floor_corners[1].m_floats[0];
            room_tween->floor_corners[3].m_floats[0] = room_tween->floor_corners[2].m_floats[0];

            room_tween->ceiling_corners[0].m_floats[1] = current_heightmap->ceiling_corners[0].m_floats[1];
            room_tween->ceiling_corners[1].m_floats[1] = room_tween->ceiling_corners[0].m_floats[1];
            room_tween->ceiling_corners[2].m_floats[1] = room_tween->ceiling_corners[0].m_floats[1];
            room_tween->ceiling_corners[3].m_floats[1] = room_tween->ceiling_corners[0].m_floats[1];
            room_tween->ceiling_corners[0].m_floats[0] = current_heightmap->ceiling_corners[0].m_floats[0];
            room_tween->ceiling_corners[1].m_floats[0] = room_tween->ceiling_corners[0].m_floats[0];
            room_tween->ceiling_corners[2].m_floats[0] = current_heightmap->ceiling_corners[1].m_floats[0];
            room_tween->ceiling_corners[3].m_floats[0] = room_tween->ceiling_corners[2].m_floats[0];

            if(w > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))                                                           // Init X-plane tween [ | ]
                {
                    if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = current_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->ceiling_corners[2].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = next_heightmap->floor_corners[2].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else
                    {
                        /************************** SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || Sectors_Is2SidePortals(current_heightmap, next_heightmap))
                        {
                            current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                            next_heightmap    = Sector_CheckPortalPointer(next_heightmap);
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[0].m_floats[2];
                                    room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                                    room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[2].m_floats[2];
                                    room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                                    Res_Sector_SetTweenFloorConfig(room_tween);
                                    joined_floors = 1;
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[0].m_floats[2];
                                    room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                                    room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[2].m_floats[2];
                                    room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                                    Res_Sector_SetTweenCeilingConfig(room_tween);
                                    joined_ceilings = 1;
                                }
                            }
                        }
                    }
                }

                current_heightmap = room->sectors + (w * room->sectors_y + h);
                next_heightmap    = current_heightmap + 1;
                if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(current_heightmap->sector_above->owner_room, next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(next_heightmap->sector_above->owner_room, current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[2].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                    }
                }

                current_heightmap = room->sectors + (w * room->sectors_y + h);
                next_heightmap    = current_heightmap + 1;
                if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != NULL) && (current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(current_heightmap->sector_below->owner_room, next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(next_heightmap->sector_below->owner_room, current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[2].m_floats[2];
                        room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                        Res_Sector_SetTweenCeilingConfig(room_tween);
                    }
                }
            }

            /*****************************************************************************************************
             ********************************   CENTRE  OF  THE  ALGORITHM   *************************************
             *****************************************************************************************************/

            room_tween++;
            current_heightmap = room->sectors + (w * room->sectors_y + h);
            next_heightmap    = room->sectors + ((w + 1) * room->sectors_y + h);
            room_tween->floor_corners[0].m_floats[0] = current_heightmap->floor_corners[1].m_floats[0];
            room_tween->floor_corners[1].m_floats[0] = room_tween->floor_corners[0].m_floats[0];
            room_tween->floor_corners[2].m_floats[0] = room_tween->floor_corners[0].m_floats[0];
            room_tween->floor_corners[3].m_floats[0] = room_tween->floor_corners[0].m_floats[0];
            room_tween->floor_corners[0].m_floats[1] = current_heightmap->floor_corners[1].m_floats[1];
            room_tween->floor_corners[1].m_floats[1] = room_tween->floor_corners[0].m_floats[1];
            room_tween->floor_corners[2].m_floats[1] = current_heightmap->floor_corners[2].m_floats[1];
            room_tween->floor_corners[3].m_floats[1] = room_tween->floor_corners[2].m_floats[1];

            room_tween->ceiling_corners[0].m_floats[0] = current_heightmap->ceiling_corners[1].m_floats[0];
            room_tween->ceiling_corners[1].m_floats[0] = room_tween->ceiling_corners[0].m_floats[0];
            room_tween->ceiling_corners[2].m_floats[0] = room_tween->ceiling_corners[0].m_floats[0];
            room_tween->ceiling_corners[3].m_floats[0] = room_tween->ceiling_corners[0].m_floats[0];
            room_tween->ceiling_corners[0].m_floats[1] = current_heightmap->ceiling_corners[1].m_floats[1];
            room_tween->ceiling_corners[1].m_floats[1] = room_tween->ceiling_corners[0].m_floats[1];
            room_tween->ceiling_corners[2].m_floats[1] = current_heightmap->ceiling_corners[2].m_floats[1];
            room_tween->ceiling_corners[3].m_floats[1] = room_tween->ceiling_corners[2].m_floats[1];

            joined_floors = 0;
            joined_ceilings = 0;

            if(h > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                {
                    // Init Y-plane tween  [ - ]
                    if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = current_heightmap->ceiling_corners[2].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[2].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = next_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else
                    {
                        /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || Sectors_Is2SidePortals(current_heightmap, next_heightmap))
                        {
                            current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                            next_heightmap    = Sector_CheckPortalPointer(next_heightmap);
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                                    room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[0].m_floats[2];
                                    room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                                    room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[2].m_floats[2];
                                    Res_Sector_SetTweenFloorConfig(room_tween);
                                    joined_floors = 1;
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                                    room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[0].m_floats[2];
                                    room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                                    room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[2].m_floats[2];
                                    Res_Sector_SetTweenCeilingConfig(room_tween);
                                    joined_ceilings = 1;
                                }
                            }
                        }
                    }
                }

                current_heightmap = room->sectors + (w * room->sectors_y + h);
                next_heightmap    = room->sectors + ((w + 1) * room->sectors_y + h);
                if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(current_heightmap->sector_above->owner_room, next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(next_heightmap->sector_above->owner_room, current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[2].m_floats[2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                    }
                }

                current_heightmap = room->sectors + (w * room->sectors_y + h);
                next_heightmap    = room->sectors + ((w + 1) * room->sectors_y + h);
                if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != NULL) && (current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(current_heightmap->sector_below->owner_room, next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSectorRaw(next_heightmap->sector_below->owner_room, current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                        room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[2].m_floats[2];
                        Res_Sector_SetTweenCeilingConfig(room_tween);
                    }
                }
            }
            room_tween++;
        }    ///END for
    }    ///END for
}

uint32_t Res_Sector_BiggestCorner(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4)
{
    v1 = (v1 > v2)?(v1):(v2);
    v2 = (v3 > v4)?(v3):(v4);
    return (v1 > v2)?(v1):(v2);
}

bool Res_IsEntityProcessed(int32_t *lookup_table, uint16_t entity_index)
{
    // Fool-proof check for entity existence. Fixes LOTS of stray non-existent
    // entity #256 occurences in original games (primarily TR4-5).

    if(!World_GetEntityByID(&engine_world, entity_index)) return true;

    int32_t *curr_table_index = lookup_table;

    while(*curr_table_index != -1)
    {
        if(*curr_table_index == (int32_t)entity_index) return true;
        curr_table_index++;
    }

    *curr_table_index = (int32_t)entity_index;
    return false;
}

int TR_Sector_TranslateFloorData(room_sector_p sector, class VT_Level *tr)
{
    if(!sector || (sector->trig_index <= 0) || (sector->trig_index >= tr->floor_data_size) || !engine_lua)
    {
        return 0;
    }

    sector->flags = 0;  // Clear sector flags before parsing.

    /*
     * PARSE FUNCTIONS
     */

    uint16_t *end_p = tr->floor_data + tr->floor_data_size - 1;
    uint16_t *entry = tr->floor_data + sector->trig_index;

    int ret = 0;
    uint16_t end_bit = 0;

    do
    {
        // TR_I - TR_II
        //function = (*entry) & 0x00FF;                   // 0b00000000 11111111
        //sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        //TR_III+, but works with TR_I - TR_II
        uint16_t function       = ((*entry) & 0x001F);             // 0b00000000 00011111
        uint16_t function_value = ((*entry) & 0x00E0) >> 5;        // 0b00000000 11100000  TR_III+
        uint16_t sub_function   = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        end_bit = ((*entry) & 0x8000) >> 15;       // 0b10000000 00000000

        entry++;

        switch(function)
        {
            case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                if(sub_function == 0x00)
                {
                    if((*entry >= 0) && (*entry < engine_world.room_count))
                    {
                        sector->portal_to_room = *entry;
                        sector->floor_penetration_config   = TR_PENETRATION_CONFIG_GHOST;
                        sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_GHOST;
                    }
                    entry ++;
                }
                break;

            case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                if(sub_function == 0x00)
                {
                    int8_t raw_y_slant =  (*entry & 0x00FF);
                    int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                    sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
                    sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;

                    if(raw_x_slant > 0)
                    {
                        sector->floor_corners[2].m_floats[2] -= ((btScalar)raw_x_slant * TR_METERING_STEP);
                        sector->floor_corners[3].m_floats[2] -= ((btScalar)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->floor_corners[0].m_floats[2] -= (abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                        sector->floor_corners[1].m_floats[2] -= (abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->floor_corners[0].m_floats[2] -= ((btScalar)raw_y_slant * TR_METERING_STEP);
                        sector->floor_corners[3].m_floats[2] -= ((btScalar)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->floor_corners[1].m_floats[2] -= (abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                        sector->floor_corners[2].m_floats[2] -= (abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                    }

                    entry++;
                }
                break;

            case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                if(sub_function == 0x00)
                {
                    int8_t raw_y_slant =  (*entry & 0x00FF);
                    int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                    sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
                    sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;

                    if(raw_x_slant > 0)
                    {
                        sector->ceiling_corners[3].m_floats[2] += ((btScalar)raw_x_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2].m_floats[2] += ((btScalar)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->ceiling_corners[1].m_floats[2] += (abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[0].m_floats[2] += (abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->ceiling_corners[1].m_floats[2] += ((btScalar)raw_y_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2].m_floats[2] += ((btScalar)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->ceiling_corners[0].m_floats[2] += (abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[3].m_floats[2] += (abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                    }

                    entry++;
                }
                break;

            case TR_FD_FUNC_TRIGGER:          // TRIGGERS
                {
                    char header[128];               header[0]            = 0;   // Header condition
                    char once_condition[128];       once_condition[0]    = 0;   // One-shot condition
                    char cont_events[4096];         cont_events[0]       = 0;   // Continous trigger events
                    char single_events[4096];       single_events[0]     = 0;   // One-shot trigger events
                    char item_events[4096];         item_events[0]       = 0;   // Item activation events
                    char anti_events[4096];         anti_events[0]       = 0;   // Item deactivation events, if needed

                    char script[8192];              script[0]            = 0;   // Final script compile

                    char buf[512];                  buf[0]  = 0;    // Stream buffer
                    char buf2[512];                 buf2[0] = 0;    // Conditional pre-buffer for SWITCH triggers

                    int activator   = TR_ACTIVATOR_NORMAL;      // Activator is normal by default.
                    int action_type = TR_ACTIONTYPE_NORMAL;     // Action type is normal by default.
                    int condition   = 0;                        // No condition by default.
                    int mask_mode   = AMASK_OP_OR;              // Activation mask by default.

                    int8_t  timer_field  =  (*entry) & 0x00FF;          // Used as common parameter for some commands.
                    uint8_t trigger_mask = ((*entry) & 0x3E00) >> 9;
                    uint8_t only_once    = ((*entry) & 0x0100) >> 8;    // Lock out triggered items after activation.

                    // Processed entities lookup array initialization.

                    int32_t ent_lookup_table[64];
                    memset(ent_lookup_table, 0xFF, sizeof(int32_t)*64);

                    // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
                    // some specific entity classes.

                    int activator_type = ( (sub_function == TR_FD_TRIGTYPE_HEAVY)            ||
                                           (sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER) ||
                                           (sub_function == TR_FD_TRIGTYPE_HEAVYSWITCH) )     ? TR_ACTIVATORTYPE_MISC : TR_ACTIVATORTYPE_LARA;

                    // Table cell header.

                    snprintf(buf, 256, "trigger_list[%d] = {activator_type = %d, func = function(entity_index) \n",
                                         sector->trig_index, activator_type);

                    strcat(script, buf);
                    buf[0] = 0;     // Zero out buffer to prevent further trashing.

                    switch(sub_function)
                    {
                        case TR_FD_TRIGTYPE_TRIGGER:
                        case TR_FD_TRIGTYPE_HEAVY:
                            activator = TR_ACTIVATOR_NORMAL;
                            break;

                        case TR_FD_TRIGTYPE_PAD:
                        case TR_FD_TRIGTYPE_ANTIPAD:
                            // Check move type for triggering entity.
                            snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", MOVE_ON_FLOOR);
                            if(sub_function == TR_FD_TRIGTYPE_ANTIPAD) action_type = TR_ACTIONTYPE_ANTI;
                            condition = 1;  // Set additional condition.
                            break;

                        case TR_FD_TRIGTYPE_SWITCH:
                            // Set activator and action type for now; conditions are linked with first item in operand chain.
                            activator = TR_ACTIVATOR_SWITCH;
                            action_type = TR_ACTIONTYPE_SWITCH;
                            mask_mode = AMASK_OP_XOR;
                            break;

                        case TR_FD_TRIGTYPE_HEAVYSWITCH:
                            // Action type remains normal, as HEAVYSWITCH acts as "heavy trigger" with activator mask filter.
                            activator = TR_ACTIVATOR_SWITCH;
                            mask_mode = AMASK_OP_XOR;
                            break;

                        case TR_FD_TRIGTYPE_KEY:
                            // Action type remains normal, as key acts one-way (no need in switch routines).
                            activator = TR_ACTIVATOR_KEY;
                            break;

                        case TR_FD_TRIGTYPE_PICKUP:
                            // Action type remains normal, as pick-up acts one-way (no need in switch routines).
                            activator = TR_ACTIVATOR_PICKUP;
                            break;

                        case TR_FD_TRIGTYPE_COMBAT:
                            // Check weapon status for triggering entity.
                            snprintf(buf, 128, " if(getCharacterCombatMode(entity_index) > 0) then \n");
                            condition = 1;  // Set additional condition.
                            break;

                        case TR_FD_TRIGTYPE_DUMMY:
                        case TR_FD_TRIGTYPE_SKELETON:   ///@FIXME: Find the meaning later!!!
                            // These triggers are being parsed, but not added to trigger script!
                            action_type = TR_ACTIONTYPE_BYPASS;
                            break;

                        case TR_FD_TRIGTYPE_ANTITRIGGER:
                        case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                            action_type = TR_ACTIONTYPE_ANTI;
                            break;

                        case TR_FD_TRIGTYPE_MONKEY:
                        case TR_FD_TRIGTYPE_CLIMB:
                            // Check move type for triggering entity.
                            snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", (sub_function == TR_FD_TRIGTYPE_MONKEY)?MOVE_MONKEYSWING:MOVE_CLIMBING);
                            condition = 1;  // Set additional condition.
                            break;

                        case TR_FD_TRIGTYPE_TIGHTROPE:
                            // Check state range for triggering entity.
                            snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", TR_STATE_LARA_TIGHTROPE_IDLE, TR_STATE_LARA_TIGHTROPE_EXIT);
                            condition = 1;  // Set additional condition.
                            break;
                        case TR_FD_TRIGTYPE_CRAWLDUCK:
                            // Check state range for triggering entity.
                            snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN, TR_ANIMATION_LARA_CRAWL_SMASH_LEFT);
                            condition = 1;  // Set additional condition.
                            break;
                    }

                    strcat(header, buf);    // Add condition to header.

                    uint16_t cont_bit = 0;
                    uint16_t argn = 0;

                    // Now parse operand chain for trigger function!

                    do
                    {
                        entry++;

                        uint16_t trigger_function = (((*entry) & 0x7C00)) >> 10;    // 0b01111100 00000000
                        uint16_t operands = (*entry) & 0x03FF;                      // 0b00000011 11111111
                                 cont_bit = ((*entry) & 0x8000) >> 15;              // 0b10000000 00000000

                        switch(trigger_function)
                        {
                            case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                                // If activator is specified, first item operand counts as activator index (except
                                // heavy switch case, which is ordinary heavy trigger case with certain differences).
                                if((argn == 0) && (activator))
                                {
                                    switch(activator)
                                    {
                                        case TR_ACTIVATOR_SWITCH:
                                            if(action_type == TR_ACTIONTYPE_SWITCH)
                                            {
                                                // Switch action type case.
                                                snprintf(buf, 256, " local switch_state = getEntityState(%d); \n local switch_sectorstatus = getEntitySectorStatus(%d); \n local switch_mask = getEntityMask(%d); \n\n", operands, operands, operands);
                                            }
                                            else
                                            {
                                                // Ordinary type case (e.g. heavy switch).
                                                snprintf(buf, 256, " local switch_sectorstatus = getEntitySectorStatus(entity_index); \n local switch_mask = getEntityMask(entity_index); \n\n");
                                            }
                                            strcat(script, buf);

                                            // Trigger activation mask is here filtered through activator's own mask.
                                            snprintf(buf, 256, " if(switch_mask == 0) then switch_mask = 0x1F end; \n switch_mask = bit32.band(switch_mask, 0x%02X); \n\n", trigger_mask);
                                            strcat(script, buf);
                                            if(action_type == TR_ACTIONTYPE_SWITCH)
                                            {
                                                // Switch action type case.
                                                snprintf(buf, 256, " if((switch_state == 0) and (switch_sectorstatus == 1)) then \n   setEntitySectorStatus(%d, 0); \n   setEntityTimer(%d, %d); \n", operands, operands, timer_field);
                                                if(only_once)
                                                {
                                                    // Just lock out activator, no anti-action needed.
                                                    snprintf(buf2, 128, " setEntityLock(%d, 1) \n", operands);
                                                }
                                                else
                                                {
                                                    // Create statement for antitriggering a switch.
                                                    snprintf(buf2, 256, " elseif((switch_state == 1) and (switch_sectorstatus == 1)) then\n   setEntitySectorStatus(%d, 0); \n   setEntityTimer(%d, 0); \n", operands, operands, operands);
                                                }
                                            }
                                            else
                                            {
                                                // Ordinary type case (e.g. heavy switch).
                                                snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, %d); \n", operands, mask_mode, only_once, timer_field);
                                                strcat(item_events, buf);
                                                snprintf(buf, 128, " if(switch_sectorstatus == 0) then \n   setEntitySectorStatus(entity_index, 1) \n");
                                            }
                                            break;

                                        case TR_ACTIVATOR_KEY:
                                            snprintf(buf, 256, " if((getEntityLock(%d) == 1) and (getEntitySectorStatus(%d) == 0)) then \n   setEntitySectorStatus(%d, 1); \n", operands, operands, operands);
                                            break;

                                        case TR_ACTIVATOR_PICKUP:
                                            snprintf(buf, 256, " if((getEntityEnability(%d) == 0) and (getEntitySectorStatus(%d) == 0)) then \n   setEntitySectorStatus(%d, 1); \n", operands, operands, operands);
                                            break;
                                    }

                                    strcat(script, buf);
                                }
                                else
                                {
                                    // In many original Core Design levels, level designers left dublicated entity activation operands.
                                    // This results in setting same activation mask twice, effectively blocking entity from activation.
                                    // To prevent this, a lookup table was implemented to know if entity already had its activation
                                    // command added.
                                    if(!Res_IsEntityProcessed(ent_lookup_table, operands))
                                    {
                                        // Other item operands are simply parsed as activation functions. Switch case is special, because
                                        // function is fed with activation mask argument derived from activator mask filter (switch_mask),
                                        // and also we need to process deactivation in a same way as activation, excluding resetting timer
                                        // field. This is needed for two-way switch combinations (e.g. Palace Midas).
                                        if(activator == TR_ACTIVATOR_SWITCH)
                                        {
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, %d); \n", operands, mask_mode, only_once, timer_field);
                                            strcat(item_events, buf);
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, 0); \n", operands, mask_mode, only_once);
                                            strcat(anti_events, buf);
                                        }
                                        else
                                        {
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, 0x%02X, %d, %d, %d); \n", operands, trigger_mask, mask_mode, only_once, timer_field);
                                            strcat(item_events, buf);
                                            snprintf(buf, 128, "   deactivateEntity(%d, entity_index); \n", operands);
                                            strcat(anti_events, buf);
                                        }
                                    }
                                }
                                argn++;
                                break;

                            case TR_FD_TRIGFUNC_CAMERATARGET:
                                {
                                    uint8_t cam_index = (*entry) & 0x007F;
                                    entry++;
                                    uint8_t cam_timer = ((*entry) & 0x00FF);
                                    uint8_t cam_once  = ((*entry) & 0x0100) >> 8;
                                    uint8_t cam_zoom  = ((*entry) & 0x1000) >> 12;
                                    cont_bit  = ((*entry) & 0x8000) >> 15;                       // 0b10000000 00000000

                                    snprintf(buf, 128, "   setCamera(%d, %d, %d, %d); \n", cam_index, cam_timer, cam_once, cam_zoom);
                                    strcat(single_events, buf);
                                }
                                break;

                            case TR_FD_TRIGFUNC_UWCURRENT:
                                snprintf(buf, 128, "   moveToSink(entity_index, %d); \n", operands);
                                strcat(cont_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_FLIPMAP:
                                // FLIPMAP trigger acts two-way for switch cases, so we add FLIPMAP off event to
                                // anti-events array.
                                if(activator == TR_ACTIVATOR_SWITCH)
                                {
                                    snprintf(buf, 128, "   setFlipMap(%d, switch_mask, 1); \n   setFlipState(%d, 1); \n", operands, operands);
                                    strcat(single_events, buf);
                                }
                                else
                                {
                                    snprintf(buf, 128, "   setFlipMap(%d, 0x%02X, 0); \n   setFlipState(%d, 1); \n", operands, trigger_mask, operands);
                                    strcat(single_events, buf);
                                }
                                break;

                            case TR_FD_TRIGFUNC_FLIPON:
                                // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                                // the switch with FLIP_ON trigger, room will remain flipped.
                                snprintf(buf, 128, "   setFlipState(%d, 1); \n", operands);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_FLIPOFF:
                                // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                                // the switch with FLIP_OFF trigger, room will remain unflipped.
                                snprintf(buf, 128, "   setFlipState(%d, 0); \n", operands);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_LOOKAT:
                                snprintf(buf, 128, "   setCamTarget(%d, %d); \n", operands, timer_field);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_ENDLEVEL:
                                snprintf(buf, 128, "   setLevel(%d); \n", operands);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_PLAYTRACK:
                                snprintf(buf, 128, "   playStream(%d, 0x%02X); \n", operands, (trigger_mask << 1) + only_once);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_FLIPEFFECT:
                                snprintf(buf, 128, "   doEffect(%d, %d); \n", operands, timer_field);
                                strcat(cont_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_SECRET:
                                snprintf(buf, 128, "   findSecret(%d); \n", operands);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_CLEARBODIES:
                                snprintf(buf, 128, "   clearBodies(); \n");
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_FLYBY:
                                {
                                    entry++;
                                    uint8_t flyby_once  = ((*entry) & 0x0100) >> 8;
                                    cont_bit  = ((*entry) & 0x8000) >> 15;

                                    snprintf(buf, 128, "   playFlyby(%d, %d); \n", operands, flyby_once);
                                    strcat(cont_events, buf);
                                }
                                break;

                            case TR_FD_TRIGFUNC_CUTSCENE:
                                snprintf(buf, 128, "   playCutscene(%d); \n", operands);
                                strcat(single_events, buf);
                                break;

                            default: // UNKNOWN!
                                break;
                        };
                    }
                    while(!cont_bit && entry < end_p);

                    if(script[0])
                    {
                        strcat(script, header);

                        // Heavy trigger and antitrigger item events are engaged ONLY
                        // once, when triggering item is approaching sector. Hence, we
                        // copy item events to single events and nullify original item
                        // events sequence to prevent it to be merged into continous
                        // events.

                        if((sub_function == TR_FD_TRIGTYPE_HEAVY) ||
                           (sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER))
                        {
                            if(action_type == TR_ACTIONTYPE_ANTI)
                            {
                                strcat(single_events, anti_events);
                            }
                            else
                            {
                                strcat(single_events, item_events);
                            }

                            anti_events[0] = 0;
                            item_events[0] = 0;
                        }

                        if(activator == TR_ACTIVATOR_NORMAL)    // Ordinary trigger cases.
                        {
                            if(single_events[0])
                            {
                                if(condition) strcat(once_condition, " ");
                                strcat(once_condition, " if(getEntitySectorStatus(entity_index) == 0) then \n");
                                strcat(script, once_condition);
                                strcat(script, single_events);
                                strcat(script, "   setEntitySectorStatus(entity_index, 1); \n");

                                if(condition)
                                {
                                    strcat(script, "  end;\n"); // First ENDIF is tabbed for extra condition.
                                }
                                else
                                {
                                    strcat(script, " end;\n");
                                }
                            }

                            // Item commands kind depends on action type. If type is ANTI, then item
                            // antitriggering is engaged. If type is normal, ordinary triggering happens
                            // in cycle with other continous commands. It is needed to prevent timer dispatch
                            // before activator leaves trigger sector.

                            if(action_type == TR_ACTIONTYPE_ANTI)
                            {
                                strcat(script, anti_events);
                            }
                            else
                            {
                                strcat(script, item_events);
                            }

                            strcat(script, cont_events);
                            if(condition) strcat(script, " end;\n"); // Additional ENDIF for extra condition.
                        }
                        else    // SWITCH, KEY and ITEM cases.
                        {
                            strcat(script, single_events);
                            strcat(script, item_events);
                            strcat(script, cont_events);
                            if((action_type == TR_ACTIONTYPE_SWITCH) && (activator == TR_ACTIVATOR_SWITCH))
                            {
                                strcat(script, buf2);
                                if(!only_once)
                                {
                                    strcat(script, single_events);
                                    strcat(script, anti_events);    // Single/continous events are engaged along with
                                    strcat(script, cont_events);    // antitriggered items, as described above.
                                }
                            }
                            strcat(script, " end;\n");
                        }

                        strcat(script, "return 1;\nend }\n");  // Finalize the entry.
                    }

                    if(action_type != TR_ACTIONTYPE_BYPASS)
                    {
                        // Sys_DebugLog("triggers.lua", script);    // Debug!
                        luaL_loadstring(engine_lua, script);
                        lua_CallAndLog(engine_lua, 0, LUA_MULTRET, 0); // Execute compiled script.
                    }
                }
                break;

            case TR_FD_FUNC_DEATH:
                sector->flags |= SECTOR_FLAG_DEATH;
                break;

            case TR_FD_FUNC_CLIMB:
                // First 4 sector flags are similar to subfunction layout.
                sector->flags |= sub_function;
                break;

            case TR_FD_FUNC_MONKEY:
                sector->flags |= SECTOR_FLAG_CLIMB_CEILING;
                break;

            case TR_FD_FUNC_MINECART_LEFT:
                // Minecart left (TR3) and trigger triggerer mark (TR4-5) has the same flag value.
                // We re-parse them properly here.
                if(tr->game_version < TR_IV)
                {
                    sector->flags |= SECTOR_FLAG_MINECART_LEFT;
                }
                else
                {
                    sector->flags |= SECTOR_FLAG_TRIGGERER_MARK;
                }
                break;

            case TR_FD_FUNC_MINECART_RIGHT:
                // Minecart right (TR3) and beetle mark (TR4-5) has the same flag value.
                // We re-parse them properly here.
                if(tr->game_version < TR_IV)
                {
                    sector->flags |= SECTOR_FLAG_MINECART_RIGHT;
                }
                else
                {
                    sector->flags |= SECTOR_FLAG_BEETLE_MARK;
                }
                break;

            default:
                // Other functions are TR3+ collisional triangle functions.
                if( (function >= TR_FD_FUNC_FLOORTRIANGLE_NW) &&
                    (function <= TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE) )
                {
                    entry--;    // Go back, since these functions are parsed differently.

                    end_bit = ((*entry) & 0x8000) >> 15;      // 0b10000000 00000000

                    int16_t  slope_t01  = ((*entry) & 0x7C00) >> 10;      // 0b01111100 00000000
                    int16_t  slope_t00  = ((*entry) & 0x03E0) >> 5;       // 0b00000011 11100000
                    uint16_t slope_func = ((*entry) & 0x001F);            // 0b00000000 00011111

                    // t01/t02 are 5-bit values, where sign is specified by 0x10 mask.

                    if(slope_t01 & 0x10) slope_t01 |= 0xFFF0;
                    if(slope_t00 & 0x10) slope_t00 |= 0xFFF0;

                    entry++;

                    uint16_t slope_t13  = ((*entry) & 0xF000) >> 12;      // 0b11110000 00000000
                    uint16_t slope_t12  = ((*entry) & 0x0F00) >> 8;       // 0b00001111 00000000
                    uint16_t slope_t11  = ((*entry) & 0x00F0) >> 4;       // 0b00000000 11110000
                    uint16_t slope_t10  = ((*entry) & 0x000F);            // 0b00000000 00001111

                    entry++;

                    float overall_adjustment = (float)Res_Sector_BiggestCorner(slope_t10, slope_t11, slope_t12, slope_t13) * TR_METERING_STEP;

                    if( (function == TR_FD_FUNC_FLOORTRIANGLE_NW)           ||
                        (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW) ||
                        (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)  )
                    {
                        sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NW;

                        sector->floor_corners[0].m_floats[2] -= overall_adjustment - ((btScalar)slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1].m_floats[2] -= overall_adjustment - ((btScalar)slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2].m_floats[2] -= overall_adjustment - ((btScalar)slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3].m_floats[2] -= overall_adjustment - ((btScalar)slope_t11 * TR_METERING_STEP);

                        if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (function == TR_FD_FUNC_FLOORTRIANGLE_NE)           ||
                             (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW) ||
                             (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)  )
                    {
                        sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NE;

                        sector->floor_corners[0].m_floats[2] -= overall_adjustment - ((btScalar)slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1].m_floats[2] -= overall_adjustment - ((btScalar)slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2].m_floats[2] -= overall_adjustment - ((btScalar)slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3].m_floats[2] -= overall_adjustment - ((btScalar)slope_t11 * TR_METERING_STEP);

                        if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (function == TR_FD_FUNC_CEILINGTRIANGLE_NW)           ||
                             (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW) ||
                             (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)  )
                    {
                        sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NW;

                        sector->ceiling_corners[0].m_floats[2] += overall_adjustment - (btScalar)(slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1].m_floats[2] += overall_adjustment - (btScalar)(slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2].m_floats[2] += overall_adjustment - (btScalar)(slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3].m_floats[2] += overall_adjustment - (btScalar)(slope_t12 * TR_METERING_STEP);

                        if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (function == TR_FD_FUNC_CEILINGTRIANGLE_NE)           ||
                             (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW) ||
                             (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)  )
                    {
                        sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NE;

                        sector->ceiling_corners[0].m_floats[2] += overall_adjustment - (btScalar)(slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1].m_floats[2] += overall_adjustment - (btScalar)(slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2].m_floats[2] += overall_adjustment - (btScalar)(slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3].m_floats[2] += overall_adjustment - (btScalar)(slope_t12 * TR_METERING_STEP);

                        if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                }
                else
                {
                    // Unknown floordata function!
                }
                break;
        };
        ret++;
    }
    while(!end_bit && entry < end_p);

    if(sector->floor == TR_METERING_WALLHEIGHT)
    {
        sector->floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
    }
    if(sector->ceiling == TR_METERING_WALLHEIGHT)
    {
        sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
    }

    return ret;
}


void GenerateAnimCommandsTransform(skeletal_model_p model)
{
    if(engine_world.anim_commands_count == 0)
    {
        return;
    }
    //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model->id);
    for(uint16_t anim = 0;anim < model->animation_count;anim++)
    {
        if(model->animations[anim].num_anim_commands > 255)
        {
            continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
        }

        animation_frame_p af  = model->animations + anim;
        int16_t *pointer      = engine_world.anim_commands + af->anim_command;

        for(uint32_t i = 0; i < af->num_anim_commands; i++, pointer++)
        {
            switch(*pointer)
            {
                case TR_ANIMCOMMAND_SETPOSITION:
                    // This command executes ONLY at the end of animation.
                    af->frames[af->frames_count-1].move[0] = (btScalar)(*++pointer);                          // x = x;
                    af->frames[af->frames_count-1].move[2] =-(btScalar)(*++pointer);                          // z =-y
                    af->frames[af->frames_count-1].move[1] = (btScalar)(*++pointer);                          // y = z
                    af->frames[af->frames_count-1].command |= ANIM_CMD_MOVE;
                    //Sys_DebugLog("anim_transform.txt", "move[anim = %d, frame = %d, frames = %d]", anim, af->frames_count-1, af->frames_count);
                    break;

                case TR_ANIMCOMMAND_JUMPDISTANCE:
                    af->frames[af->frames_count-1].v_Vertical   = *++pointer;
                    af->frames[af->frames_count-1].v_Horizontal = *++pointer;
                    af->frames[af->frames_count-1].command |= ANIM_CMD_JUMP;
                    break;

                case TR_ANIMCOMMAND_EMPTYHANDS:
                    break;

                case TR_ANIMCOMMAND_KILL:
                    break;

                case TR_ANIMCOMMAND_PLAYSOUND:
                    ++pointer;
                    ++pointer;
                    break;

                case TR_ANIMCOMMAND_PLAYEFFECT:
                    {
                        int frame = *++pointer;
                        switch(*++pointer & 0x3FFF)
                        {
                            case TR_EFFECT_CHANGEDIRECTION:
                                af->frames[frame].command |= ANIM_CMD_CHANGE_DIRECTION;
                                Con_Printf("ROTATE: anim = %d, frame = %d of %d", anim, frame, af->frames_count);
                                //Sys_DebugLog("anim_transform.txt", "dir[anim = %d, frame = %d, frames = %d]", anim, frame, af->frames_count);
                                break;
                        }
                    }
                    break;
            }
        }
    }
}


int TR_IsSectorsIn2SideOfPortal(room_sector_p s1, room_sector_p s2, portal_p p)
{
    if((s1->pos[0] == s2->pos[0]) && (s1->pos[1] != s2->pos[1]) && (fabs(p->norm[1]) > 0.99))
    {
        btScalar min_x, max_x, min_y, max_y, x;
        max_x = min_x = p->vertex[0];
        for(uint16_t i=1;i<p->vertex_count;i++)
        {
            x = p->vertex[3 * i + 0];
            if(x > max_x)
            {
                max_x = x;
            }
            if(x < min_x)
            {
                min_x = x;
            }
        }
        if(s1->pos[1] > s2->pos[1])
        {
            min_y = s2->pos[1];
            max_y = s1->pos[1];
        }
        else
        {
            min_y = s1->pos[1];
            max_y = s2->pos[1];
        }

        if((s1->pos[0] < max_x) && (s1->pos[0] > min_x) && (p->centre[1] < max_y) && (p->centre[1] > min_y))
        {
            return 1;
        }
    }
    else if((s1->pos[0] != s2->pos[0]) && (s1->pos[1] == s2->pos[1]) && (fabs(p->norm[0]) > 0.99))
    {
        btScalar min_x, max_x, min_y, max_y, y;
        max_y = min_y = p->vertex[1];
        for(uint16_t i=1;i<p->vertex_count;i++)
        {
            y = p->vertex[3 * i + 1];
            if(y > max_y)
            {
                max_y = y;
            }
            if(y < min_y)
            {
                min_y = y;
            }
        }
        if(s1->pos[0] > s2->pos[0])
        {
            min_x = s2->pos[0];
            max_x = s1->pos[0];
        }
        else
        {
            min_x = s1->pos[0];
            max_x = s2->pos[0];
        }

        if((p->centre[0] < max_x) && (p->centre[0] > min_x) && (s1->pos[1] < max_y) && (s1->pos[1] > min_y))
        {
            return 1;
        }
    }

    return 0;
}

void TR_Sector_Calculate(struct world_s *world, class VT_Level *tr, long int room_index)
{
    room_sector_p sector;
    room_p room = world->rooms + room_index;
    tr5_room_t *tr_room = &tr->rooms[room_index];

    /*
     * Sectors loading
     */

    sector = room->sectors;
    for(uint32_t i=0;i<room->sectors_count;i++,sector++)
    {
        /*
         * Let us fill pointers to sectors above and sectors below
         */

        uint8_t rp = tr_room->sector_list[i].room_below;
        sector->sector_below = NULL;
        if(rp >= 0 && rp < world->room_count && rp != 255)
        {
            sector->sector_below = Room_GetSectorRaw(world->rooms + rp, sector->pos);
        }
        rp = tr_room->sector_list[i].room_above;
        sector->sector_above = NULL;
        if(rp >= 0 && rp < world->room_count && rp != 255)
        {
            sector->sector_above = Room_GetSectorRaw(world->rooms + rp, sector->pos);
        }

        room_sector_p near_sector = NULL;

        /**** OX *****/
        if((sector->index_y > 0) && (sector->index_y < room->sectors_y - 1) && (sector->index_x == 0))
        {
            near_sector = sector + room->sectors_y;
        }
        if((sector->index_y > 0) && (sector->index_y < room->sectors_y - 1) && (sector->index_x == room->sectors_x - 1))
        {
            near_sector = sector - room->sectors_y;
        }
        /**** OY *****/
        if((sector->index_x > 0) && (sector->index_x < room->sectors_x - 1) && (sector->index_y == 0))
        {
            near_sector = sector + 1;
        }
        if((sector->index_x > 0) && (sector->index_x < room->sectors_x - 1) && (sector->index_y == room->sectors_y - 1))
        {
            near_sector = sector - 1;
        }

        if((near_sector != NULL) && (sector->portal_to_room >= 0))
        {
            portal_p p = room->portals;
            for(uint16_t j=0;j<room->portal_count;j++,p++)
            {
                if((p->norm[2] < 0.01) && ((p->norm[2] > -0.01)))
                {
                    room_sector_p dst = Room_GetSectorRaw(p->dest_room, sector->pos);
                    room_sector_p orig_dst = Room_GetSectorRaw(engine_world.rooms + sector->portal_to_room, sector->pos);
                    if((dst != NULL) && (dst->portal_to_room < 0) && (dst->floor != TR_METERING_WALLHEIGHT) && (dst->ceiling != TR_METERING_WALLHEIGHT) && ((uint32_t)sector->portal_to_room != p->dest_room->id) && (dst->floor < orig_dst->floor) && TR_IsSectorsIn2SideOfPortal(near_sector, dst, p))
                    {
                        sector->portal_to_room = p->dest_room->id;
                        orig_dst = dst;
                    }
                }
            }
        }
    }
}

void TR_vertex_to_arr(btScalar v[3], tr5_vertex_t *tr_v)
{
    v[0] = tr_v->x;
    v[1] =-tr_v->z;
    v[2] = tr_v->y;
}

void TR_color_to_arr(btScalar v[4], tr5_colour_t *tr_c)
{
    v[0] = tr_c->r * 2;
    v[1] = tr_c->g * 2;
    v[2] = tr_c->b * 2;
    v[3] = tr_c->a * 2;
}

room_sector_p TR_GetRoomSector(uint32_t room_id, int sx, int sy)
{
    room_p room;
    if(room_id >= engine_world.room_count)
    {
        return NULL;
    }

    room = engine_world.rooms + room_id;
    if((sx < 0) || (sx >= room->sectors_x) || (sy < 0) || (sy >= room->sectors_y))
    {
        return NULL;
    }

    return room->sectors + sx * room->sectors_y + sy;
}

int lua_SetSectorFloorConfig(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 10)
    {
        Con_AddLine("Wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, floor, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->floor = lua_tonumber(lua, 6);
    rs->floor_corners[0].m_floats[2] = lua_tonumber(lua, 7);
    rs->floor_corners[1].m_floats[2] = lua_tonumber(lua, 8);
    rs->floor_corners[2].m_floats[2] = lua_tonumber(lua, 9);
    rs->floor_corners[3].m_floats[2] = lua_tonumber(lua, 10);

    return 0;
}

int lua_SetSectorCeilingConfig(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 10)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, ceiling, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->ceiling_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->ceiling_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->ceiling = lua_tonumber(lua, 6);
    rs->ceiling_corners[0].m_floats[2] = lua_tonumber(lua, 7);
    rs->ceiling_corners[1].m_floats[2] = lua_tonumber(lua, 8);
    rs->ceiling_corners[2].m_floats[2] = lua_tonumber(lua, 9);
    rs->ceiling_corners[3].m_floats[2] = lua_tonumber(lua, 10);

    return 0;
}

int lua_SetSectorPortal(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 4)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, portal_room_id)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    uint32_t p = lua_tointeger(lua, 4);
    if(p < engine_world.room_count)
    {
        rs->portal_to_room = p;
    }

    return 0;
}

int lua_SetSectorFlags(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 7)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, fp_flag, ft_flag, cp_flag, ct_flag)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->ceiling_penetration_config = lua_tointeger(lua, 6);
    if(!lua_isnil(lua, 7))  rs->ceiling_diagonal_type = lua_tointeger(lua, 7);

    return 0;
}

void Res_ScriptsOpen(int engine_version)
{
    char temp_script_name[256];
    Engine_GetLevelScriptName(engine_version, temp_script_name);

    level_script = luaL_newstate();
    if(level_script != NULL)
    {
        luaL_openlibs(level_script);
        lua_register(level_script, "print", lua_print);
        lua_register(level_script, "setSectorFloorConfig", lua_SetSectorFloorConfig);
        lua_register(level_script, "setSectorCeilingConfig", lua_SetSectorCeilingConfig);
        lua_register(level_script, "setSectorPortal", lua_SetSectorPortal);
        lua_register(level_script, "setSectorFlags", lua_SetSectorFlags);

        luaL_dofile(level_script, "scripts/staticmesh/staticmesh_script.lua");
        int lua_err = luaL_dofile(level_script, temp_script_name);

        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(level_script, -1));
            lua_pop(level_script, 1);
            lua_close(level_script);
            level_script = NULL;
        }
    }

    objects_flags_conf = luaL_newstate();
    if(objects_flags_conf != NULL)
    {
        luaL_openlibs(objects_flags_conf);
        int lua_err = luaL_loadfile(objects_flags_conf, "scripts/entity/entity_properties.lua");
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(objects_flags_conf, -1));
            lua_pop(objects_flags_conf, 1);
            lua_close(objects_flags_conf);
            objects_flags_conf = NULL;
        }
        lua_err = lua_pcall(objects_flags_conf, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(objects_flags_conf, -1));
            lua_pop(objects_flags_conf, 1);
            lua_close(objects_flags_conf);
            objects_flags_conf = NULL;
        }
    }

    ent_ID_override = luaL_newstate();
    if(ent_ID_override != NULL)
    {
        luaL_openlibs(ent_ID_override);
        int lua_err = luaL_loadfile(ent_ID_override, "scripts/entity/entity_model_ID_override.lua");
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(ent_ID_override, -1));
            lua_pop(ent_ID_override, 1);
            lua_close(ent_ID_override);
            ent_ID_override = NULL;
        }
        lua_err = lua_pcall(ent_ID_override, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(ent_ID_override, -1));
            lua_pop(ent_ID_override, 1);
            lua_close(ent_ID_override);
            ent_ID_override = NULL;
        }
    }
}

void Res_ScriptsClose()
{
    if(objects_flags_conf)
    {
        lua_close(objects_flags_conf);
        objects_flags_conf = NULL;
    }

    if(ent_ID_override)
    {
        lua_close(ent_ID_override);
        ent_ID_override = NULL;
    }

    if(level_script)
    {
        lua_close(level_script);
        level_script = NULL;
    }
}

void Res_AutoexecOpen(int engine_version)
{
    char temp_script_name[256];
    Engine_GetLevelScriptName(engine_version, temp_script_name, "_autoexec");

    luaL_dofile(engine_lua, "scripts/autoexec.lua");    // do standart autoexec
    luaL_dofile(engine_lua, temp_script_name);          // do level-specific autoexec
}

void TR_GenWorld(struct world_s *world, class VT_Level *tr)
{
    world->version = tr->game_version;

    Res_ScriptsOpen(world->version);   // Open configuration scripts.
    Gui_DrawLoadScreen(150);

    Res_GenRBTrees(world);               // Generate red-black trees
    Gui_DrawLoadScreen(200);

    TR_GenTextures(world, tr);          // Generate OGL textures
    Gui_DrawLoadScreen(300);

    TR_GenAnimCommands(world, tr);      // Copy anim commands
    Gui_DrawLoadScreen(310);

    TR_GenAnimTextures(world, tr);      // Generate animated textures
    Gui_DrawLoadScreen(320);

    TR_GenMeshes(world, tr);            // Generate all meshes
    Gui_DrawLoadScreen(400);

    TR_GenSprites(world, tr);           // Generate all sprites
    Gui_DrawLoadScreen(420);

    TR_GenBoxes(world, tr);             // Generate boxes.
    Gui_DrawLoadScreen(440);

    TR_GenCameras(world, tr);           // Generate cameras & sinks.
    Gui_DrawLoadScreen(460);

    TR_GenRooms(world, tr);             // Build all rooms
    Gui_DrawLoadScreen(500);

    Res_GenRoomFlipMap(world);           // Generate room flipmaps
    Gui_DrawLoadScreen(520);

    // Build all skeletal models. Must be generated before TR_Sector_Calculate() function.

    TR_GenSkeletalModels(world, tr);
    Gui_DrawLoadScreen(600);

    TR_GenEntities(world, tr);          // Build all moveables (entities)
    Gui_DrawLoadScreen(650);

    Res_GenBaseItems(world);             // Generate inventory item entries.
    Gui_DrawLoadScreen(680);

    // Generate sprite buffers. Only now because entity generation adds new sprites

    Res_GenSpritesBuffer(world);
    Gui_DrawLoadScreen(700);

    TR_GenRoomProperties(world, tr);
    Gui_DrawLoadScreen(750);

    Res_GenRoomCollision(world);
    Gui_DrawLoadScreen(800);

    // Initialize audio.

    TR_GenSamples(world, tr);
    Gui_DrawLoadScreen(850);

    // Find and set skybox.

    world->sky_box = Res_GetSkybox(world, world->version);
    Gui_DrawLoadScreen(860);

    // Load entity collision flags and ID overrides from script.

    Res_ScriptsClose();
    Gui_DrawLoadScreen(870);

    // Generate VBOs for meshes.

    Res_GenVBOs(world);
    Gui_DrawLoadScreen(950);

    // Process level autoexec loading.

    Res_AutoexecOpen(world->version);
    Gui_DrawLoadScreen(960);

    // Fix initial room states

    Res_FixRooms(world);
    Gui_DrawLoadScreen(970);
}


void Res_GenRBTrees(struct world_s *world)
{
    world->entity_tree = RB_Init();
    world->entity_tree->rb_compEQ = compEntityEQ;
    world->entity_tree->rb_compLT = compEntityLT;
    world->entity_tree->rb_free_data = RBEntityFree;

    world->items_tree = RB_Init();
    world->items_tree->rb_compEQ = compEntityEQ;
    world->items_tree->rb_compLT = compEntityLT;
    world->items_tree->rb_free_data = RBItemFree;
}


void TR_GenRooms(struct world_s *world, class VT_Level *tr)
{
    world->room_count = tr->rooms_count;
    room_p r = world->rooms = (room_p)calloc(world->room_count, sizeof(room_t));
    for(uint32_t i=0;i<world->room_count;i++,r++)
    {
        TR_GenRoom(i, r, world, tr);
    }
}

void TR_GenRoom(size_t room_index, struct room_s *room, struct world_s *world, class VT_Level *tr)
{
    portal_p p;
    room_p r_dest;
    tr5_room_t *tr_room = &tr->rooms[room_index];
    tr_staticmesh_t *tr_static;
    static_mesh_p r_static;
    tr_room_portal_t *tr_portal;
    room_sector_p sector;
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;

    room->id = room_index;
    room->active = 1;
    room->frustum = NULL;
    room->flags = tr->rooms[room_index].flags;
    room->light_mode = tr->rooms[room_index].light_mode;
    room->reverb_info = tr->rooms[room_index].reverb_info;
    room->water_scheme = tr->rooms[room_index].water_scheme;
    room->alternate_group = tr->rooms[room_index].alternate_group;

    Mat4_E_macro(room->transform);
    room->transform[12] = tr->rooms[room_index].offset.x;                       // x = x;
    room->transform[13] =-tr->rooms[room_index].offset.z;                       // y =-z;
    room->transform[14] = tr->rooms[room_index].offset.y;                       // z = y;
    room->ambient_lighting[0] = tr->rooms[room_index].light_colour.r * 2;
    room->ambient_lighting[1] = tr->rooms[room_index].light_colour.g * 2;
    room->ambient_lighting[2] = tr->rooms[room_index].light_colour.b * 2;
    room->self = (engine_container_p)calloc(1, sizeof(engine_container_t));
    room->self->room = room;
    room->self->object = room;
    room->self->object_type = OBJECT_ROOM_BASE;
    room->near_room_list_size = 0;
    room->overlapped_room_list_size = 0;

    TR_GenRoomMesh(world, room_index, room, tr);

    room->bt_body = NULL;
    /*
     *  let us load static room meshes
     */
    room->static_mesh_count = tr_room->num_static_meshes;
    room->static_mesh = NULL;
    if(room->static_mesh_count)
    {
        room->static_mesh = (static_mesh_p)calloc(room->static_mesh_count, sizeof(static_mesh_t));
    }

    r_static = room->static_mesh;
    for(uint16_t i=0;i<tr_room->num_static_meshes;i++)
    {
        tr_static = tr->find_staticmesh_id(tr_room->static_meshes[i].object_id);
        if(tr_static == NULL)
        {
            room->static_mesh_count--;
            continue;
        }
        r_static->self = (engine_container_p)calloc(1, sizeof(engine_container_t));
        r_static->self->room = room;
        r_static->self->object = room->static_mesh + i;
        r_static->self->object_type = OBJECT_STATIC_MESH;
        r_static->object_id = tr_room->static_meshes[i].object_id;
        r_static->mesh = world->meshes + tr->mesh_indices[tr_static->mesh];
        r_static->pos[0] = tr_room->static_meshes[i].pos.x;
        r_static->pos[1] =-tr_room->static_meshes[i].pos.z;
        r_static->pos[2] = tr_room->static_meshes[i].pos.y;
        r_static->rot[0] = tr_room->static_meshes[i].rotation;
        r_static->rot[1] = 0.0;
        r_static->rot[2] = 0.0;
        r_static->tint[0] = tr_room->static_meshes[i].tint.r * 2;
        r_static->tint[1] = tr_room->static_meshes[i].tint.g * 2;
        r_static->tint[2] = tr_room->static_meshes[i].tint.b * 2;
        r_static->tint[3] = tr_room->static_meshes[i].tint.a * 2;
        r_static->obb = OBB_Create();

        r_static->cbb_min[0] = tr_static->collision_box[0].x;
        r_static->cbb_min[1] =-tr_static->collision_box[0].z;
        r_static->cbb_min[2] = tr_static->collision_box[1].y;
        r_static->cbb_max[0] = tr_static->collision_box[1].x;
        r_static->cbb_max[1] =-tr_static->collision_box[1].z;
        r_static->cbb_max[2] = tr_static->collision_box[0].y;

        r_static->vbb_min[0] = tr_static->visibility_box[0].x;
        r_static->vbb_min[1] =-tr_static->visibility_box[0].z;
        r_static->vbb_min[2] = tr_static->visibility_box[1].y;
        r_static->vbb_max[0] = tr_static->visibility_box[1].x;
        r_static->vbb_max[1] =-tr_static->visibility_box[1].z;
        r_static->vbb_max[2] = tr_static->visibility_box[0].y;

        r_static->obb->transform = room->static_mesh[i].transform;
        r_static->obb->r = room->static_mesh[i].mesh->R;
        Mat4_E(r_static->transform);
        Mat4_Translate(r_static->transform, r_static->pos);
        Mat4_RotateZ(r_static->transform, r_static->rot[0]);
        r_static->was_rendered = 0;
        OBB_Rebuild(r_static->obb, r_static->vbb_min, r_static->vbb_max);
        OBB_Transform(r_static->obb);

        r_static->bt_body = NULL;
        r_static->hide = 0;

        // Disable static mesh collision, if flag value is 3 (TR1) or all bounding box
        // coordinates are equal (TR2-5).

        if((tr_static->flags == 3) ||
           ((r_static->cbb_min[0] == r_static->cbb_min[1]) && (r_static->cbb_min[1] == r_static->cbb_min[2]) &&
            (r_static->cbb_max[0] == r_static->cbb_max[1]) && (r_static->cbb_max[1] == r_static->cbb_max[2])) )
        {
            r_static->self->collision_type = COLLISION_NONE;
        }
        else
        {
            r_static->self->collision_type = COLLISION_TYPE_STATIC;
        }

        // Set additional static mesh properties from level script override.

        Res_SetStaticMeshProperties(r_static);

        // Set static mesh collision.
        if(r_static->self->collision_type != COLLISION_NONE)
        {
            switch(r_static->self->collision_shape)
            {
                case COLLISION_SHAPE_BOX:
                    cshape = BT_CSfromBBox(r_static->cbb_min, r_static->cbb_max, true, true);
                    break;

                case COLLISION_SHAPE_BOX_BASE:
                    cshape = BT_CSfromBBox(r_static->mesh->bb_min, r_static->mesh->bb_max, true, true);
                    break;

                case COLLISION_SHAPE_TRIMESH:
                    cshape = BT_CSfromMesh(r_static->mesh, true, true, true);
                    break;

                case COLLISION_SHAPE_TRIMESH_CONVEX:
                    cshape = BT_CSfromMesh(r_static->mesh, true, true, false);
                    break;

                default:
                    cshape = NULL;
                    break;
            };

            if(cshape)
            {
                startTransform.setFromOpenGLMatrix(r_static->transform);
                btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                r_static->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
                bt_engine_dynamicsWorld->addRigidBody(r_static->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
                r_static->bt_body->setUserPointer(r_static->self);
            }
        }
        r_static++;
    }

    /*
     * sprites loading section
     */
    room->sprites_count = tr_room->num_sprites;
    if(room->sprites_count != 0)
    {
        room->sprites = (room_sprite_p)calloc(room->sprites_count, sizeof(room_sprite_t));
        for(uint32_t i=0;i<room->sprites_count;i++)
        {
            if((tr_room->sprites[i].texture >= 0) && ((uint32_t)tr_room->sprites[i].texture < world->sprites_count))
            {
                room->sprites[i].sprite = world->sprites + tr_room->sprites[i].texture;
                TR_vertex_to_arr(room->sprites[i].pos, &tr_room->vertices[tr_room->sprites[i].vertex].vertex);
                vec3_add(room->sprites[i].pos, room->sprites[i].pos, room->transform+12);
            }
        }
    }

    /*
     * let us load sectors
     */
    room->sectors_x = tr_room->num_xsectors;
    room->sectors_y = tr_room->num_zsectors;
    room->sectors_count = room->sectors_x * room->sectors_y;
    room->sectors = (room_sector_p)malloc(room->sectors_count * sizeof(room_sector_t));

    /*
     * base sectors information loading and collisional mesh creation
     */

    // To avoid manipulating with unnecessary information, we declare simple
    // heightmap here, which will be operated with sector and floordata parsing,
    // then vertical inbetween polys will be constructed, and Bullet collisional
    // object will be created. Afterwards, this heightmap also can be used to
    // quickly detect slopes for pushable blocks and other entities that rely on
    // floor level.

    sector = room->sectors;
    for(uint32_t i=0;i<room->sectors_count;i++,sector++)
    {
        // Filling base sectors information.

        sector->index_x = i / room->sectors_y;
        sector->index_y = i % room->sectors_y;

        sector->pos[0] = room->transform[12] + sector->index_x * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
        sector->pos[1] = room->transform[13] + sector->index_y * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
        sector->pos[2] = 0.5 * (tr_room->y_bottom + tr_room->y_top);

        sector->owner_room = room;

        if(tr->game_version < TR_III)
        {
            sector->box_index = tr_room->sector_list[i].box_index;
            sector->material  = SECTOR_MATERIAL_STONE;
        }
        else
        {
            sector->box_index = (tr_room->sector_list[i].box_index & 0xFFF0) >> 4;
            sector->material  =  tr_room->sector_list[i].box_index & 0x000F;
        }

        if(sector->box_index == 0xFFFF) sector->box_index = -1;

        sector->flags = 0;  // Clear sector flags.

        sector->floor      = -TR_METERING_STEP * (int)tr_room->sector_list[i].floor;
        sector->ceiling    = -TR_METERING_STEP * (int)tr_room->sector_list[i].ceiling;
        sector->trig_index = tr_room->sector_list[i].fd_index;

        // BUILDING CEILING HEIGHTMAP.

        // Penetration config is used later to build inbetween vertical collision polys.
        // If sector's penetration config is a wall, we simply build a vertical plane to
        // isolate this sector from top to bottom. Also, this allows to trick out wall
        // sectors inside another wall sectors to be ignored completely when building
        // collisional mesh.
        // Door penetration config means that we should either ignore sector collision
        // completely (classic door) or ignore one of the triangular sector parts (TR3+).

        if(sector->ceiling == TR_METERING_WALLHEIGHT)
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_above != 0xFF)
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        // Reset some sector parameters to avoid garbaged memory issues.

        room->sectors[i].portal_to_room = -1;
        room->sectors[i].ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
        room->sectors[i].floor_diagonal_type   = TR_SECTOR_DIAGONAL_TYPE_NONE;

        // Now, we define heightmap cells position and draft (flat) height.
        // Draft height is derived from sector's floor and ceiling values, which are
        // copied into heightmap cells Y coordinates. As result, we receive flat
        // heightmap cell, which will be operated later with floordata.

        room->sectors[i].ceiling_corners[0].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0].m_floats[2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[1].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1].m_floats[2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[2].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2].m_floats[2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[3].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3].m_floats[2] = (btScalar)sector->ceiling;

        // BUILDING FLOOR HEIGHTMAP.

        // Features same steps as for the ceiling.

        if(sector->floor == TR_METERING_WALLHEIGHT)
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_below != 0xFF)
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        room->sectors[i].floor_corners[0].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0].m_floats[2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[1].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1].m_floats[2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[2].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2].m_floats[2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[3].m_floats[0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3].m_floats[1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3].m_floats[2] = (btScalar)sector->floor;
    }

    /*
     *  load lights
     */
    room->light_count = tr_room->num_lights;
    room->lights = NULL;
    if(room->light_count)
    {
        room->lights = (light_p)malloc(room->light_count * sizeof(light_t));
    }

    for(uint16_t i=0;i<tr_room->num_lights;i++)
    {
        switch(tr_room->lights[i].light_type)
        {
        case 0:
            room->lights[i].light_type = LT_SUN;
            break;
        case 1:
            room->lights[i].light_type = LT_POINT;
            break;
        case 2:
            room->lights[i].light_type = LT_SPOTLIGHT;
            break;
        case 3:
            room->lights[i].light_type = LT_SHADOW;
            break;
        default:
            room->lights[i].light_type = LT_NULL;
            break;
        }

        room->lights[i].pos[0] = tr_room->lights[i].pos.x;
        room->lights[i].pos[1] = -tr_room->lights[i].pos.z;
        room->lights[i].pos[2] = tr_room->lights[i].pos.y;
        room->lights[i].pos[3] = 1.0f;

        if(room->lights[i].light_type == LT_SHADOW)
        {
            room->lights[i].colour[0] = -(tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[1] = -(tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[2] = -(tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[3] = 1.0f;
        }
        else
        {
            room->lights[i].colour[0] = (tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[1] = (tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[2] = (tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
            room->lights[i].colour[3] = 1.0f;
        }

        room->lights[i].inner = tr_room->lights[i].r_inner;
        room->lights[i].outer = tr_room->lights[i].r_outer;
        room->lights[i].length = tr_room->lights[i].length;
        room->lights[i].cutoff = tr_room->lights[i].cutoff;

        room->lights[i].falloff = 0.001f / room->lights[i].outer;
    }


    /*
     * portals loading / calculation!!!
     */
    room->portal_count = tr_room->num_portals;
    p = room->portals = (portal_p)calloc(room->portal_count, sizeof(portal_t));
    tr_portal = tr_room->portals;
    for(uint16_t i=0;i<room->portal_count;i++,p++,tr_portal++)
    {
        r_dest = world->rooms + tr_portal->adjoining_room;
        p->vertex_count = 4;                                                    // in original TR all portals are axis aligned rectangles
        p->vertex = (btScalar*)malloc(3*p->vertex_count*sizeof(btScalar));
        p->flag = 0;
        p->dest_room = r_dest;
        p->current_room = room;
        TR_vertex_to_arr(p->vertex  , &tr_portal->vertices[3]);
        vec3_add(p->vertex, p->vertex, room->transform+12);
        TR_vertex_to_arr(p->vertex+3, &tr_portal->vertices[2]);
        vec3_add(p->vertex+3, p->vertex+3, room->transform+12);
        TR_vertex_to_arr(p->vertex+6, &tr_portal->vertices[1]);
        vec3_add(p->vertex+6, p->vertex+6, room->transform+12);
        TR_vertex_to_arr(p->vertex+9, &tr_portal->vertices[0]);
        vec3_add(p->vertex+9, p->vertex+9, room->transform+12);
        vec3_add(p->centre, p->vertex, p->vertex+3);
        vec3_add(p->centre, p->centre, p->vertex+6);
        vec3_add(p->centre, p->centre, p->vertex+9);
        p->centre[0] /= 4.0;
        p->centre[1] /= 4.0;
        p->centre[2] /= 4.0;
        Portal_GenNormale(p);

        /*
         * Portal position fix...
         */
        // X_MIN
        if((p->norm[0] > 0.999) && (((int)p->centre[0])%2))
        {
            btScalar pos[3] = {1.0, 0.0, 0.0};
            Portal_Move(p, pos);
        }

        // Y_MIN
        if((p->norm[1] > 0.999) && (((int)p->centre[1])%2))
        {
            btScalar pos[3] = {0.0, 1.0, 0.0};
            Portal_Move(p, pos);
        }

        // Z_MAX
        if((p->norm[2] <-0.999) && (((int)p->centre[2])%2))
        {
            btScalar pos[3] = {0.0, 0.0, -1.0};
            Portal_Move(p, pos);
        }
    }

    /*
     * room borders calculation
     */
    room->bb_min[2] = tr_room->y_bottom;
    room->bb_max[2] = tr_room->y_top;

    room->bb_min[0] = room->transform[12] + TR_METERING_SECTORSIZE;
    room->bb_min[1] = room->transform[13] + TR_METERING_SECTORSIZE;
    room->bb_max[0] = room->transform[12] + TR_METERING_SECTORSIZE * room->sectors_x - TR_METERING_SECTORSIZE;
    room->bb_max[1] = room->transform[13] + TR_METERING_SECTORSIZE * room->sectors_y - TR_METERING_SECTORSIZE;

    /*
     * alternate room pointer calculation if one exists.
     */
    room->alternate_room = NULL;
    room->base_room = NULL;

    if((tr_room->alternate_room >= 0) && ((uint32_t)tr_room->alternate_room < tr->rooms_count))
    {
        room->alternate_room = world->rooms + tr_room->alternate_room;
    }
}


void Res_GenRoomCollision(struct world_s *world)
{
    room_p r = world->rooms;

    /*
    if(level_script != NULL)
    {
        lua_CallVoidFunc(level_script, "doTuneSector");
    }
    */

    for(uint32_t i=0;i<world->room_count;i++,r++)
    {
        // Inbetween polygons array is later filled by loop which scans adjacent
        // sector heightmaps and fills the gaps between them, thus creating inbetween
        // polygon. Inbetweens can be either quad (if all four corner heights are
        // different), triangle (if one corner height is similar to adjacent) or
        // ghost (if corner heights are completely similar). In case of quad inbetween,
        // two triangles are added to collisional trimesh, in case of triangle inbetween,
        // we add only one, and in case of ghost inbetween, we ignore it.

        int num_heightmaps = (r->sectors_x * r->sectors_y);
        int num_tweens = (num_heightmaps * 4);
        sector_tween_s *room_tween   = new sector_tween_s[num_tweens];

        // Clear tween array.

        for(int j=0;j<num_tweens;j++)
        {
            room_tween[j].ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
            room_tween[j].floor_tween_type   = TR_SECTOR_TWEEN_TYPE_NONE;
        }

        // Most difficult task with converting floordata collision to trimesh collision is
        // building inbetween polygons which will block out gaps between sector heights.
        Res_Sector_GenTweens(r, room_tween);

        // Final step is sending actual sectors to Bullet collision model. We do it here.

        btCollisionShape *cshape = BT_CSfromHeightmap(r->sectors, room_tween, num_tweens, true, true);

        if(cshape)
        {
            btVector3 localInertia(0, 0, 0);
            btTransform tr;
            tr.setFromOpenGLMatrix(r->transform);
            btDefaultMotionState* motionState = new btDefaultMotionState(tr);
            r->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
            bt_engine_dynamicsWorld->addRigidBody(r->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
            r->bt_body->setUserPointer(r->self);
            r->bt_body->setRestitution(1.0);
            r->bt_body->setFriction(1.0);
            r->self->collision_type = COLLISION_TYPE_STATIC;                    // meshtree
            r->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        }

        delete[] room_tween;
    }
}


void TR_GenRoomProperties(struct world_s *world, class VT_Level *tr)
{
    room_p r = world->rooms;

    for(uint32_t i=0;i<world->room_count;i++,r++)
    {
        if(r->alternate_room != NULL)
        {
            r->alternate_room->base_room = r;   // Refill base room pointer.
        }

        // Fill heightmap and translate floordata.
        for(uint32_t j=0;j<r->sectors_count;j++)
        {
            TR_Sector_TranslateFloorData(r->sectors + j, tr);
        }

        // Generate links to the near rooms.
        Room_BuildNearRoomsList(r);
        // Generate links to the overlapped rooms.
        Room_BuildOverlappedRoomsList(r);

        // Basic sector calculations.
        TR_Sector_Calculate(world, tr, i);
    }
}


void Res_GenRoomFlipMap(struct world_s *world)
{
    // Flipmap count is hardcoded, as no original levels contain such info.

    world->flip_count = FLIPMAP_MAX_NUMBER;

    world->flip_map   = (uint8_t*)malloc(world->flip_count * sizeof(uint8_t));
    world->flip_state = (uint8_t*)malloc(world->flip_count * sizeof(uint8_t));

    memset(world->flip_map,   0, world->flip_count);
    memset(world->flip_state, 0, world->flip_count);
}


void TR_GenBoxes(struct world_s *world, class VT_Level *tr)
{
    world->room_boxes = NULL;
    world->room_box_count = tr->boxes_count;

    if(world->room_box_count)
    {
        world->room_boxes = (room_box_p)malloc(world->room_box_count * sizeof(room_box_t));
        for(uint32_t i=0;i<world->room_box_count;i++)
        {
            world->room_boxes[i].overlap_index = tr->boxes[i].overlap_index;
            world->room_boxes[i].true_floor =-tr->boxes[i].true_floor;
            world->room_boxes[i].x_min = tr->boxes[i].xmin;
            world->room_boxes[i].x_max = tr->boxes[i].xmax;
            world->room_boxes[i].y_min =-tr->boxes[i].zmax;
            world->room_boxes[i].y_max =-tr->boxes[i].zmin;
        }
    }
}

void TR_GenCameras(struct world_s *world, class VT_Level *tr)
{
    world->cameras_sinks = NULL;
    world->cameras_sinks_count = tr->cameras_count;

    if(world->cameras_sinks_count)
    {
        world->cameras_sinks = (stat_camera_sink_p)malloc(world->cameras_sinks_count * sizeof(stat_camera_sink_t));
        for(uint32_t i=0;i<world->cameras_sinks_count;i++)
        {
            world->cameras_sinks[i].x                   =  tr->cameras[i].x;
            world->cameras_sinks[i].y                   =  tr->cameras[i].z;
            world->cameras_sinks[i].z                   = -tr->cameras[i].y;
            world->cameras_sinks[i].room_or_strength    =  tr->cameras[i].room;
            world->cameras_sinks[i].flag_or_zone        =  tr->cameras[i].unknown1;
        }
    }
}

/**
 * sprites loading, works correct in TR1 - TR5
 */
void TR_GenSprites(struct world_s *world, class VT_Level *tr)
{
    sprite_p s;
    tr_sprite_texture_t *tr_st;

    if(tr->sprite_textures_count == 0)
    {
        world->sprites = NULL;
        world->sprites_count = 0;
        return;
    }

    world->sprites_count = tr->sprite_textures_count;
    s = world->sprites = (sprite_p)calloc(world->sprites_count, sizeof(sprite_t));

    for(uint32_t i=0;i<world->sprites_count;i++,s++)
    {
        tr_st = &tr->sprite_textures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        world->tex_atlas->getSpriteCoordinates(i, s->texture, s->tex_coord);
    }

    for(uint32_t i=0;i<tr->sprite_sequences_count;i++)
    {
        if((tr->sprite_sequences[i].offset >= 0) && ((uint32_t)tr->sprite_sequences[i].offset < world->sprites_count))
        {
            world->sprites[tr->sprite_sequences[i].offset].id = tr->sprite_sequences[i].object_id;
        }
    }
}

void Res_GenSpritesBuffer(struct world_s *world)
{
    for (uint32_t i = 0; i < world->room_count; i++)
        Res_GenRoomSpritesBuffer(&world->rooms[i]);
}

void TR_GenTextures(struct world_s* world, class VT_Level *tr)
{
    int border_size = renderer.settings.texture_border;
    border_size = (border_size < 0)?(0):(border_size);
    border_size = (border_size > 128)?(128):(border_size);
    world->tex_atlas = new bordered_texture_atlas(border_size,
                                                  tr->textile32_count,
                                                  tr->textile32,
                                                  tr->object_textures_count,
                                                  tr->object_textures,
                                                  tr->sprite_textures_count,
                                                  tr->sprite_textures);

    world->tex_count = (uint32_t) world->tex_atlas->getNumAtlasPages() + 1;
    world->textures = (GLuint*)malloc(world->tex_count * sizeof(GLuint));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(1, 1);
    world->tex_atlas->createTextures(world->textures, 1);

    // white texture data for coloured polygons and debug lines.
    GLubyte whtx[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // Mag filter is always linear.

    // Select mipmap mode
    switch(renderer.settings.mipmap_mode)
    {
        case 0:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;

        case 1:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;

        case 2:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;

        case 3:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
    };

    // Set mipmaps number
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, renderer.settings.mipmaps);

    // Set anisotropy degree
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, renderer.settings.anisotropy);

    // Read lod bias
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, renderer.settings.lod_bias);


    glBindTexture(GL_TEXTURE_2D, world->textures[world->tex_count-1]);          // solid color =)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    //glDisable(GL_TEXTURE_2D); // Why it is here? It is blocking loading screen.

}

/**   Animated textures loading.
  *   Natively, animated textures stored as a stream of bitu16s, which
  *   is then parsed on the fly. What we do is parse this stream to the
  *   proper structures to be used later within renderer.
  */
void TR_GenAnimTextures(struct world_s *world, class VT_Level *tr)
{
    uint16_t *pointer;
    uint16_t  num_sequences, num_uvrotates;
    int32_t   uvrotate_script = 0;
    polygon_t p0, p;

    p0.vertex_count = 0;
    p0.vertices = NULL;
    p.vertex_count = 0;
    p.vertices = NULL;
    Polygon_Resize(&p0, 3);
    Polygon_Resize(&p, 3);

    pointer       = tr->animated_textures;
    num_uvrotates = tr->animated_textures_uv_count;

    num_sequences = *(pointer++);   // First word in a stream is sequence count.

    world->anim_sequences_count = num_sequences;
    world->anim_sequences = (anim_seq_p)calloc(num_sequences, sizeof(anim_seq_t));

    anim_seq_p seq = world->anim_sequences;
    for(uint16_t i = 0; i < num_sequences; i++,seq++)
    {
        seq->frames_count = *(pointer++) + 1;
        seq->frame_list   =  (uint32_t*)calloc(seq->frames_count, sizeof(uint32_t));

        // Fill up new sequence with frame list.
        seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
        seq->frame_lock        = false; // by default anim is playing
        seq->uvrotate          = false; // by default uvrotate
        seq->reverse_direction = false; // Needed for proper reverse-type start-up.
        seq->frame_rate        = 0.05;  // Should be passed as 1 / FPS.
        seq->frame_time        = 0.0;   // Reset frame time to initial state.
        seq->current_frame     = 0;     // Reset current frame to zero.

        for(uint16_t j = 0; j < seq->frames_count; j++)
        {
            seq->frame_list[j] = *(pointer++);  // Add one frame.
        }

        // UVRotate textures case.
        // In TR4-5, it is possible to define special UVRotate animation mode.
        // It is specified by num_uvrotates variable. If sequence belongs to
        // UVRotate range, each frame will be divided in half and continously
        // scrolled from one part to another by shifting UV coordinates.
        // In OpenTomb, we can have BOTH UVRotate and classic frames mode
        // applied to the same sequence, but there we specify compatibility
        // method for TR4-5.

        if(level_script)
        {
            int top = lua_gettop(level_script);
            lua_getglobal(level_script, "UVRotate");
            uvrotate_script = lua_tointeger(level_script, -1);
            lua_settop(level_script, top);
        }

        if(i < num_uvrotates)
        {
            seq->frame_lock        = false; // by default anim is playing

            seq->uvrotate = true;
            // Get texture height and divide it in half.
            // This way, we get a reference value which is used to identify
            // if scrolling is completed or not.
            seq->frames_count = 8;
            seq->uvrotate_max   = world->tex_atlas->getTextureHeight(seq->frame_list[0]) / 2;
            seq->uvrotate_speed = seq->uvrotate_max / (btScalar)seq->frames_count;
            seq->frames = (tex_frame_p)calloc(seq->frames_count, sizeof(tex_frame_t));
            seq->frame_list = (uint32_t *) calloc(seq->frames_count, sizeof(uint32_t));

            if(uvrotate_script > 0)
            {
                seq->anim_type        = TR_ANIMTEXTURE_FORWARD;
            }
            else if(uvrotate_script < 0)
            {
                seq->anim_type        = TR_ANIMTEXTURE_BACKWARD;
            }

            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0.0, true);
            for(uint16_t j=0;j<seq->frames_count;j++)
            {
                engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, (GLfloat)j * seq->uvrotate_speed, true);
                seq->frames[j].tex_ind = p.tex_index;

                GLfloat A0[2], B0[2], A[2], B[2], d;                            ///@PARANOID: texture transformation may be not only move
                A0[0] = p0.vertices[1].tex_coord[0] - p0.vertices[0].tex_coord[0];
                A0[1] = p0.vertices[1].tex_coord[1] - p0.vertices[0].tex_coord[1];
                B0[0] = p0.vertices[2].tex_coord[0] - p0.vertices[0].tex_coord[0];
                B0[1] = p0.vertices[2].tex_coord[1] - p0.vertices[0].tex_coord[1];

                A[0] = p.vertices[1].tex_coord[0] - p.vertices[0].tex_coord[0];
                A[1] = p.vertices[1].tex_coord[1] - p.vertices[0].tex_coord[1];
                B[0] = p.vertices[2].tex_coord[0] - p.vertices[0].tex_coord[0];
                B[1] = p.vertices[2].tex_coord[1] - p.vertices[0].tex_coord[1];

                d = A0[0] * B0[1] - A0[1] * B0[0];
                seq->frames[j].mat[0 + 0 * 2] = (A[0] * B0[1] - A0[1] * B[0]) / d;
                seq->frames[j].mat[1 + 0 * 2] =-(A[1] * B0[1] - A0[1] * B[1]) / d;
                seq->frames[j].mat[0 + 1 * 2] =-(A0[0] * B[0] - A[0] * B0[0]) / d;
                seq->frames[j].mat[1 + 1 * 2] = (A0[0] * B[1] - A[1] * B0[0]) / d;

                seq->frames[j].move[0] = p.vertices[0].tex_coord[0] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[0 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[0 + 1 * 2]);
                seq->frames[j].move[1] = p.vertices[0].tex_coord[1] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[1 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[1 + 1 * 2]);
            }
        }
        else
        {
            seq->frames = (tex_frame_p)calloc(seq->frames_count, sizeof(tex_frame_t));
            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p0);
            for(uint16_t j=0;j<seq->frames_count;j++)
            {
                engine_world.tex_atlas->getCoordinates(seq->frame_list[j], false, &p);
                seq->frames[j].tex_ind = p.tex_index;

                GLfloat A0[2], B0[2], A[2], B[2], d;                            ///@PARANOID: texture transformation may be not only move
                A0[0] = p0.vertices[1].tex_coord[0] - p0.vertices[0].tex_coord[0];
                A0[1] = p0.vertices[1].tex_coord[1] - p0.vertices[0].tex_coord[1];
                B0[0] = p0.vertices[2].tex_coord[0] - p0.vertices[0].tex_coord[0];
                B0[1] = p0.vertices[2].tex_coord[1] - p0.vertices[0].tex_coord[1];

                A[0] = p.vertices[1].tex_coord[0] - p.vertices[0].tex_coord[0];
                A[1] = p.vertices[1].tex_coord[1] - p.vertices[0].tex_coord[1];
                B[0] = p.vertices[2].tex_coord[0] - p.vertices[0].tex_coord[0];
                B[1] = p.vertices[2].tex_coord[1] - p.vertices[0].tex_coord[1];

                d = A0[0] * B0[1] - A0[1] * B0[0];
                seq->frames[j].mat[0 + 0 * 2] = (A[0] * B0[1] - A0[1] * B[0]) / d;
                seq->frames[j].mat[1 + 0 * 2] =-(A[1] * B0[1] - A0[1] * B[1]) / d;
                seq->frames[j].mat[0 + 1 * 2] =-(A0[0] * B[0] - A[0] * B0[0]) / d;
                seq->frames[j].mat[1 + 1 * 2] = (A0[0] * B[1] - A[1] * B0[0]) / d;

                seq->frames[j].move[0] = p.vertices[0].tex_coord[0] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[0 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[0 + 1 * 2]);
                seq->frames[j].move[1] = p.vertices[0].tex_coord[1] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[1 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[1 + 1 * 2]);
            }

        }
    }
    Polygon_Clear(&p0);
    Polygon_Clear(&p);
}

/**   Assign animated texture to a polygon.
  *   While in original TRs we had TexInfo abstraction layer to refer texture,
  *   in OpenTomb we need to re-think animated texture concept to work on a
  *   per-polygon basis. For this, we scan all animated texture lists for
  *   same TexInfo index that is applied to polygon, and if corresponding
  *   animation list is found, we assign it to polygon.
  */
bool SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct world_s *world)
{
    polygon->anim_id = 0;                           // Reset to 0 by default.

    for(uint32_t i = 0; i < world->anim_sequences_count; i++)
    {
        for(uint16_t j = 0; j < world->anim_sequences[i].frames_count; j++)
        {
            if(world->anim_sequences[i].frame_list[j] == tex_index)
            {
                // If we have found assigned texture ID in animation texture lists,
                // we assign corresponding animation sequence to this polygon,
                // additionally specifying frame offset.
                polygon->anim_id      = i + 1;  // Animation sequence ID.
                polygon->frame_offset  = j;     // Animation frame offset.
                return true;
            }
        }
    }

    return false;   // No such TexInfo found in animation textures lists.
}

static void addPolygonCopyToList(const polygon_s *polygon, polygon_s *&list)
{
    polygon_p np = (polygon_p)calloc(1, sizeof(polygon_t));
    Polygon_Copy(np, polygon);
    np->next = list;
    list = np;
}

void Res_Poly_SortInMesh(struct base_mesh_s *mesh)
{
    polygon_p p = mesh->polygons;
    for(uint32_t i=0;i<mesh->polygons_count;i++,p++)
    {
        if((p->anim_id > 0) && (p->anim_id <= engine_world.anim_sequences_count))
        {
            anim_seq_p seq = engine_world.anim_sequences + (p->anim_id - 1);
            // set tex coordinates to the first frame for correct texture transform in renderer
            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, p, 0, seq->uvrotate);
        }

        if(p->transparency >= 2)
        {
            addPolygonCopyToList(p, mesh->transparency_polygons);
        }
    }
}


void TR_GenMeshes(struct world_s *world, class VT_Level *tr)
{
    base_mesh_p base_mesh;

    world->meshes_count = tr->meshes_count;
    base_mesh = world->meshes = (base_mesh_p)calloc(world->meshes_count, sizeof(base_mesh_t));
    for(uint32_t i=0;i<world->meshes_count;i++,base_mesh++)
    {
        TR_GenMesh(world, i, base_mesh, tr);
    }
}

static void tr_copyNormals(const polygon_p polygon, base_mesh_p mesh, const uint16_t *mesh_vertex_indices)
{
    for (int i = 0; i < polygon->vertex_count; i++)
    {
        vec3_copy(polygon->vertices[i].normal, mesh->vertices[mesh_vertex_indices[i]].normal);
    }
}

void tr_accumulateNormals(tr4_mesh_t *tr_mesh, base_mesh_p mesh, int numCorners, const uint16_t *vertex_indices, polygon_p p)
{
    Polygon_Resize(p, numCorners);

    for (int i = 0; i < numCorners; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, &tr_mesh->vertices[vertex_indices[i]]);
    }
    Polygon_FindNormale(p);

    for (int i = 0; i < numCorners; i++)
    {
        vec3_add(mesh->vertices[vertex_indices[i]].normal, mesh->vertices[vertex_indices[i]].normal, p->plane);
    }
}

void tr_setupColoredFace(tr4_mesh_t *tr_mesh, VT_Level *tr, base_mesh_p mesh, const uint16_t *vertex_indices, unsigned color, polygon_p p)
{
    for (int i = 0; i < p->vertex_count; i++)
    {
        p->vertices[i].color[0] = tr->palette.colour[color].r / 255.0f;
        p->vertices[i].color[1] = tr->palette.colour[color].g / 255.0f;
        p->vertices[i].color[2] = tr->palette.colour[color].b / 255.0f;
        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[i].color[0] = p->vertices[i].color[0] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[1] = p->vertices[i].color[1] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[2] = p->vertices[i].color[2] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
        }
        p->vertices[i].color[3] = 1.0f;

        p->vertices[i].tex_coord[0] = i & 2 ? 1.0 : 0.0;
        p->vertices[i].tex_coord[1] = i >= 2 ? 1.0 : 0.0;
    }
    mesh->uses_vertex_colors = 1;
}

void tr_setupTexturedFace(tr4_mesh_t *tr_mesh, base_mesh_p mesh, const uint16_t *vertex_indices, polygon_p p)
{
    for (int i = 0; i < p->vertex_count; i++)
    {
        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[i].color[0] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[1] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[2] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[3] = 1.0f;

            mesh->uses_vertex_colors = 1;
        }
        else
        {
            vec4_set_one(p->vertices[i].color);
        }
    }
}

void TR_GenMesh(struct world_s *world, size_t mesh_index, struct base_mesh_s *mesh, class VT_Level *tr)
{
    uint16_t col;
    tr4_mesh_t *tr_mesh;
    tr4_face4_t *face4;
    tr4_face3_t *face3;
    tr4_object_texture_t *tex;
    polygon_p p;
    btScalar n;
    vertex_p vertex;
    uint32_t tex_mask = (world->version == TR_IV)?(TR_TEXTURE_INDEX_MASK_TR4):(TR_TEXTURE_INDEX_MASK);

    /* TR WAD FORMAT DOCUMENTATION!
     * tr4_face[3,4]_t:
     * flipped texture & 0x8000 (1 bit  ) - horizontal flipping.
     * shape texture   & 0x7000 (3 bits ) - texture sample shape.
     * index texture   & $0FFF  (12 bits) - texture sample index.
     *
     * if bit [15] is set, as in ( texture and $8000 ), it indicates that the texture
     * sample must be flipped horizontally prior to be used.
     * Bits [14..12] as in ( texture and $7000 ), are used to store the texture
     * shape, given by: ( texture and $7000 ) shr 12.
     * The valid values are: 0, 2, 4, 6, 7, as assigned to a square starting from
     * the top-left corner and going clockwise: 0, 2, 4, 6 represent the positions
     * of the square angle of the triangles, 7 represents a quad.
     */

    tr_mesh = &tr->meshes[mesh_index];
    mesh->id = mesh_index;
    mesh->centre[0] = tr_mesh->centre.x;
    mesh->centre[1] =-tr_mesh->centre.z;
    mesh->centre[2] = tr_mesh->centre.y;
    mesh->R = tr_mesh->collision_size;
    mesh->num_texture_pages = (uint32_t)world->tex_atlas->getNumAtlasPages() + 1;

    mesh->vertex_count = tr_mesh->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(uint32_t i=0;i<mesh->vertex_count;i++,vertex++)
    {
        TR_vertex_to_arr(vertex->position, &tr_mesh->vertices[i]);
        vec3_set_zero(vertex->normal);                                          // paranoid
    }

    BaseMesh_FindBB(mesh);

    mesh->polygons_count = tr_mesh->num_textured_triangles + tr_mesh->num_coloured_triangles + tr_mesh->num_textured_rectangles + tr_mesh->num_coloured_rectangles;
    p = mesh->polygons = Polygon_CreateArray(mesh->polygons_count);

    /*
     * textured triangles
     */
    for(int16_t i=0;i<tr_mesh->num_textured_triangles;i++,p++)
    {
        face3 = &tr_mesh->textured_triangles[i];
        tex = &tr->object_textures[face3->texture & tex_mask];

        p->double_side = (bool)(face3->texture >> 15);    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(p, face3->texture & tex_mask, world);

        if(face3->lighting & 0x01)
        {
            p->transparency = BM_MULTIPLY;
        }
        else
        {
            p->transparency = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, mesh, 3, face3->vertices, p);
        tr_setupTexturedFace(tr_mesh, mesh, face3->vertices, p);

        world->tex_atlas->getCoordinates(face3->texture & tex_mask, 0, p);
    }

    /*
     * coloured triangles
     */
    for(int16_t i=0;i<tr_mesh->num_coloured_triangles;i++,p++)
    {
        face3 = &tr_mesh->coloured_triangles[i];
        col = face3->texture & 0xff;
        p->tex_index = (uint32_t)world->tex_atlas->getNumAtlasPages();
        p->transparency = 0;
        p->anim_id = 0;

        tr_accumulateNormals(tr_mesh, mesh, 3, face3->vertices, p);
        tr_setupColoredFace(tr_mesh, tr, mesh, face3->vertices, col, p);
    }

    /*
     * textured rectangles
     */
    for(int16_t i=0;i<tr_mesh->num_textured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->textured_rectangles[i];
        tex = &tr->object_textures[face4->texture & tex_mask];

        p->double_side = (bool)(face4->texture >> 15);    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(p, face4->texture & tex_mask, world);

        if(face4->lighting & 0x01)
        {
            p->transparency = BM_MULTIPLY;
        }
        else
        {
            p->transparency = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, mesh, 4, face4->vertices, p);
        tr_setupTexturedFace(tr_mesh, mesh, face4->vertices, p);

        world->tex_atlas->getCoordinates(face4->texture & tex_mask, 0, p);
    }

    /*
     * coloured rectangles
     */
    for(int16_t i=0;i<tr_mesh->num_coloured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->coloured_rectangles[i];
        col = face4->texture & 0xff;
        Polygon_Resize(p, 4);
        p->tex_index = (uint32_t)world->tex_atlas->getNumAtlasPages();
        p->transparency = 0;
        p->anim_id = 0;

        tr_accumulateNormals(tr_mesh, mesh, 4, face4->vertices, p);
        tr_setupColoredFace(tr_mesh, tr, mesh, face4->vertices, col, p);
    }

    /*
     * let us normalise normales %)
     */
    p = mesh->polygons;
    for(uint32_t i=0;i<mesh->vertex_count;i++)
    {
        vec3_norm(mesh->vertices[i].normal, n);
    }

    /*
     * triangles
     */
    for(int16_t i=0;i<tr_mesh->num_textured_triangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_mesh->textured_triangles[i].vertices);
    }

    for(int16_t i=0;i<tr_mesh->num_coloured_triangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_mesh->coloured_triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(int16_t i=0;i<tr_mesh->num_textured_rectangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_mesh->textured_rectangles[i].vertices);
    }

    for(int16_t i=0;i<tr_mesh->num_coloured_rectangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_mesh->coloured_rectangles[i].vertices);
    }

    if(mesh->vertex_count > 0)
    {
        mesh->vertex_count = 0;
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    Mesh_GenFaces(mesh);
    Res_Poly_SortInMesh(mesh);
}

void tr_setupRoomVertices(struct world_s *world, class VT_Level *tr, const tr5_room_t *tr_room, base_mesh_p mesh, int numCorners, const uint16_t *vertices, uint16_t masked_texture, polygon_p p)
{
    Polygon_Resize(p, numCorners);

    for (int i = 0; i < numCorners; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, &tr_room->vertices[vertices[i]].vertex);
    }
    Polygon_FindNormale(p);

    for (int i = 0; i < numCorners; i++)
    {
        vec3_add(mesh->vertices[vertices[i]].normal, mesh->vertices[vertices[i]].normal, p->plane);
        vec3_copy(p->vertices[i].normal, p->plane);
        TR_color_to_arr(p->vertices[i].color, &tr_room->vertices[vertices[i]].colour);
    }

    tr4_object_texture_t *tex = &tr->object_textures[masked_texture];
    SetAnimTexture(p, masked_texture, world);
    p->transparency = tex->transparency_flags;

    world->tex_atlas->getCoordinates(masked_texture, 0, p);

}

void TR_GenRoomMesh(struct world_s *world, size_t room_index, struct room_s *room, class VT_Level *tr)
{
    tr5_room_t *tr_room;
    polygon_p p;
    base_mesh_p mesh;
    btScalar n;
    vertex_p vertex;
    uint32_t tex_mask = (world->version == TR_IV)?(TR_TEXTURE_INDEX_MASK_TR4):(TR_TEXTURE_INDEX_MASK);

    tr_room = &tr->rooms[room_index];

    if(tr_room->num_triangles + tr_room->num_rectangles == 0)
    {
        room->mesh = NULL;
        return;
    }

    mesh = room->mesh = (base_mesh_p)calloc(1, sizeof(base_mesh_t));
    mesh->id = room_index;
    mesh->num_texture_pages = (uint32_t)world->tex_atlas->getNumAtlasPages() + 1;
    mesh->uses_vertex_colors = 1; // This is implicitly true on room meshes

    mesh->vertex_count = tr_room->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(uint32_t i=0;i<mesh->vertex_count;i++,vertex++)
    {
        TR_vertex_to_arr(vertex->position, &tr_room->vertices[i].vertex);
        vec3_set_zero(vertex->normal);                                          // paranoid
    }

    BaseMesh_FindBB(mesh);

    mesh->polygons_count = tr_room->num_triangles + tr_room->num_rectangles;
    p = mesh->polygons = Polygon_CreateArray(mesh->polygons_count);

    /*
     * triangles
     */
    for(uint32_t i=0;i<tr_room->num_triangles;i++,p++)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 3, tr_room->triangles[i].vertices, tr_room->triangles[i].texture & tex_mask, p);
        p->double_side = tr_room->triangles[i].texture & 0x8000;
    }

    /*
     * rectangles
     */
    for(uint32_t i=0;i<tr_room->num_rectangles;i++,p++)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 4, tr_room->rectangles[i].vertices, tr_room->rectangles[i].texture & tex_mask, p);
        p->double_side = tr_room->rectangles[i].texture & 0x8000;
    }

    /*
     * let us normalise normales %)
     */
    for(uint32_t i=0;i<mesh->vertex_count;i++)
    {
        vec3_norm(mesh->vertices[i].normal, n);
    }

    /*
     * triangles
     */
    p = mesh->polygons;
    for(uint32_t i=0;i<tr_room->num_triangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_room->triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(uint32_t i=0;i<tr_room->num_rectangles;i++,p++)
    {
        tr_copyNormals(p, mesh, tr_room->rectangles[i].vertices);
    }

    if(mesh->vertex_count > 0)
    {
        mesh->vertex_count = 0;
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    Mesh_GenFaces(mesh);
    Res_Poly_SortInMesh(mesh);
}

void Res_GenRoomSpritesBuffer(struct room_s *room)
{
    // Find the number of different texture pages used and the number of non-null sprites
    uint32_t highestTexturePageFound = 0;
    int actualSpritesFound = 0;
    for (uint32_t i = 0; i < room->sprites_count; i++)
    {
        if (room->sprites[i].sprite)
        {
            actualSpritesFound += 1;
            highestTexturePageFound = std::max(highestTexturePageFound, room->sprites[i].sprite->texture);
        }
    }
    if (actualSpritesFound == 0)
    {
        room->sprite_buffer = 0;
        return;
    }

    room->sprite_buffer = (struct sprite_buffer_s *) calloc(sizeof(struct sprite_buffer_s), 1);
    room->sprite_buffer->num_texture_pages = highestTexturePageFound + 1;
    room->sprite_buffer->element_count_per_texture = (uint32_t *) calloc(sizeof(uint32_t), room->sprite_buffer->num_texture_pages);

    // First collect indices on a per-texture basis
    uint16_t **elements_for_texture = (uint16_t **)calloc(sizeof(uint16_t*), highestTexturePageFound + 1);

    GLfloat *spriteData = (GLfloat *) calloc(sizeof(GLfloat [7]), actualSpritesFound * 4);

    int writeIndex = 0;
    for (int i = 0; i < room->sprites_count; i++)
    {
        const struct room_sprite_s &room_sprite = room->sprites[i];
        if (room_sprite.sprite)
        {
            int vertexStart = writeIndex;
            // top right
            memcpy(&spriteData[writeIndex*7 + 0], room_sprite.pos, sizeof(GLfloat [3]));
            memcpy(&spriteData[writeIndex*7 + 3], &room_sprite.sprite->tex_coord[0], sizeof(GLfloat [2]));
            spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
            spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

            writeIndex += 1;

            // top left
            memcpy(&spriteData[writeIndex*7 + 0], room_sprite.pos, sizeof(GLfloat [3]));
            memcpy(&spriteData[writeIndex*7 + 3], &room_sprite.sprite->tex_coord[2], sizeof(GLfloat [2]));
            spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
            spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

            writeIndex += 1;

            // bottom left
            memcpy(&spriteData[writeIndex*7 + 0], room_sprite.pos, sizeof(GLfloat [3]));
            memcpy(&spriteData[writeIndex*7 + 3], &room_sprite.sprite->tex_coord[4], sizeof(GLfloat [2]));
            spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
            spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

            writeIndex += 1;

            // bottom right
            memcpy(&spriteData[writeIndex*7 + 0], room_sprite.pos, sizeof(GLfloat [3]));
            memcpy(&spriteData[writeIndex*7 + 3], &room_sprite.sprite->tex_coord[6], sizeof(GLfloat [2]));
            spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
            spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

            writeIndex += 1;


            // Assign indices
            uint32_t texture = room_sprite.sprite->texture;
            uint32_t start = room->sprite_buffer->element_count_per_texture[texture];
            uint32_t newElementCount = start + 6;
            room->sprite_buffer->element_count_per_texture[texture] = newElementCount;
            elements_for_texture[texture] = (uint16_t *)realloc(elements_for_texture[texture], newElementCount * sizeof(uint16_t));

            elements_for_texture[texture][start + 0] = vertexStart + 0;
            elements_for_texture[texture][start + 1] = vertexStart + 1;
            elements_for_texture[texture][start + 2] = vertexStart + 2;
            elements_for_texture[texture][start + 3] = vertexStart + 2;
            elements_for_texture[texture][start + 4] = vertexStart + 3;
            elements_for_texture[texture][start + 5] = vertexStart + 0;
        }
    }

    // Now flatten all these indices to a single array
    uint16_t *elements = NULL;
    uint32_t elementsSoFar = 0;
    for(uint32_t i = 0; i <= highestTexturePageFound; i++)
    {
        if(elements_for_texture[i] == NULL)
        {
            continue;
        }
        elements = (uint16_t*)realloc(elements, (elementsSoFar + room->sprite_buffer->element_count_per_texture[i])*sizeof(elements_for_texture[0][0]));
        memcpy(elements + elementsSoFar, elements_for_texture[i], room->sprite_buffer->element_count_per_texture[i]*sizeof(elements_for_texture[0][0]));

        elementsSoFar += room->sprite_buffer->element_count_per_texture[i];
        free(elements_for_texture[i]);
    }
    free(elements_for_texture);

    // Now load into OpenGL
    GLuint arrayBuffer, elementBuffer;
    glGenBuffersARB(1, &arrayBuffer);
    glBindBufferARB(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat [7]) * 4 * actualSpritesFound, spriteData, GL_STATIC_DRAW);
    free(spriteData);

    glGenBuffersARB(1, &elementBuffer);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * elementsSoFar, elements, GL_STATIC_DRAW);
    free(elements);

    struct vertex_array_attribute attribs[3] = {
        vertex_array_attribute(sprite_shader_description::vertex_attribs::position, 3, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [0])),
        vertex_array_attribute(sprite_shader_description::vertex_attribs::tex_coord, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [3])),
        vertex_array_attribute(sprite_shader_description::vertex_attribs::corner_offset, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [5]))
    };

    room->sprite_buffer->data = renderer.vertex_array_manager->createArray(elementBuffer, 3, attribs);
}

void Res_GenVBOs(struct world_s *world)
{
    for(uint32_t i=0;i<world->meshes_count;i++)
    {
        if(world->meshes[i].vertex_count || world->meshes[i].animated_vertex_count)
        {
            Mesh_GenVBO(&renderer, world->meshes + i);
        }
    }

    for(uint32_t i=0;i<world->room_count;i++)
    {
        if((world->rooms[i].mesh) && (world->rooms[i].mesh->vertex_count || world->rooms[i].mesh->animated_vertex_count))
        {
            Mesh_GenVBO(&renderer, world->rooms[i].mesh);
        }
    }
}

void Res_GenBaseItems(struct world_s* world)
{
    lua_CallVoidFunc(engine_lua, "genBaseItems");

    if((world->items_tree != NULL) && (world->items_tree->root != NULL))
    {
        Res_EntityToItem(world->items_tree->root);
    }
}

void Res_FixRooms(struct world_s *world)
{
    room_p r = world->rooms;
    for(uint32_t i=0;i<world->room_count;i++,r++)
    {
        if(r->base_room != NULL)
        {
            Room_Disable(r);    // Disable current room
        }

        // Isolated rooms may be used for rolling ball trick (for ex., LEVEL4.PHD).
        // Hence, this part is commented.

        /*
        if((r->portal_count == 0) && (world->room_count > 1))
        {
            Room_Disable(r);
        }
        */
    }
}

long int TR_GetOriginalAnimationFrameOffset(uint32_t offset, uint32_t anim, class VT_Level *tr)
{
    tr_animation_t *tr_animation;

    if(anim >= tr->animations_count)
    {
        return -1;
    }

    tr_animation = &tr->animations[anim];
    if(anim + 1 == tr->animations_count)
    {
        if(offset < tr_animation->frame_offset)
        {
            return -2;
        }
    }
    else
    {
        if((offset < tr_animation->frame_offset) && (offset >= (tr_animation+1)->frame_offset))
        {
            return -2;
        }
    }

    return tr_animation->frame_offset;
}

struct skeletal_model_s* Res_GetSkybox(struct world_s *world, uint32_t engine_version)
{
    switch(engine_version)
    {
        case TR_II:
        case TR_II_DEMO:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR2);

        case TR_III:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR3);

        case TR_IV:
        case TR_IV_DEMO:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR4);

        case TR_V:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR5);

        default:
            return NULL;
    }
}

void TR_GenAnimCommands(struct world_s *world, class VT_Level *tr)
{
    world->anim_commands_count = tr->anim_commands_count;
    world->anim_commands = tr->anim_commands;
    tr->anim_commands = NULL;
    tr->anim_commands_count = 0;
}

void TR_GenSkeletalModel(struct world_s *world, size_t model_num, struct skeletal_model_s *model, class VT_Level *tr)
{
    tr_moveable_t *tr_moveable;
    tr_animation_t *tr_animation;

    uint32_t frame_offset, frame_step;
    uint16_t temp1, temp2;
    float ang;
    btScalar rot[3];

    bone_tag_p bone_tag;
    bone_frame_p bone_frame;
    mesh_tree_tag_p tree_tag;
    animation_frame_p anim;

    tr_moveable = &tr->moveables[model_num];                                    // original tr structure
    model->collision_map = (uint16_t*)malloc(model->mesh_count * sizeof(uint16_t));
    model->collision_map_size = model->mesh_count;
    for(uint16_t i=0;i<model->mesh_count;i++)
    {
        model->collision_map[i] = i;
    }

    model->mesh_tree = (mesh_tree_tag_p)calloc(model->mesh_count, sizeof(mesh_tree_tag_t));
    tree_tag = model->mesh_tree;

    uint32_t *mesh_index = tr->mesh_indices + tr_moveable->starting_mesh;

    for(uint16_t k=0;k<model->mesh_count;k++,tree_tag++)
    {
        tree_tag->mesh_base = world->meshes + (mesh_index[k]);
        tree_tag->mesh_skin = NULL;                                             ///@PARANOID: I use calloc for tree_tag's
        tree_tag->replace_anim = 0x00;
        tree_tag->replace_mesh = 0x00;
        tree_tag->body_part    = 0x00;
        tree_tag->offset[0] = 0.0;
        tree_tag->offset[1] = 0.0;
        tree_tag->offset[2] = 0.0;
        if(k == 0)
        {
            tree_tag->flag = 0x02;
        }
        else
        {
            uint32_t *tr_mesh_tree = tr->mesh_tree_data + tr_moveable->mesh_tree_index + (k-1)*4;
            tree_tag->flag = (tr_mesh_tree[0] & 0xFF);
            tree_tag->offset[0] = (float)((int32_t)tr_mesh_tree[1]);
            tree_tag->offset[1] = (float)((int32_t)tr_mesh_tree[3]);
            tree_tag->offset[2] =-(float)((int32_t)tr_mesh_tree[2]);
        }
    }

    /*
     * =================    now, animation loading    ========================
     */

    if(tr_moveable->animation_index < 0 || tr_moveable->animation_index >= tr->animations_count)
    {
        /*
         * model has no start offset and any animation
         */
        model->animation_count = 1;
        model->animations = (animation_frame_p)malloc(sizeof(animation_frame_t));
        model->animations->frames_count = 1;
        model->animations->frames = (bone_frame_p)calloc(model->animations->frames_count , sizeof(bone_frame_t));
        bone_frame = model->animations->frames;

        model->animations->id = 0;
        model->animations->next_anim = NULL;
        model->animations->next_frame = 0;
        model->animations->state_change = NULL;
        model->animations->state_change_count = 0;
        model->animations->original_frame_rate = 1;

        bone_frame->bone_tag_count = model->mesh_count;
        bone_frame->bone_tags = (bone_tag_p)malloc(bone_frame->bone_tag_count * sizeof(bone_tag_t));

        vec3_set_zero(bone_frame->pos);
        vec3_set_zero(bone_frame->move);
        bone_frame->v_Horizontal = 0.0;
        bone_frame->v_Vertical = 0.0;
        bone_frame->command = 0x00;
        for(uint16_t k=0;k<bone_frame->bone_tag_count;k++)
        {
            tree_tag = model->mesh_tree + k;
            bone_tag = bone_frame->bone_tags + k;

            rot[0] = 0.0;
            rot[1] = 0.0;
            rot[2] = 0.0;
            vec4_SetTRRotations(bone_tag->qrotate, rot);
            vec3_copy(bone_tag->offset, tree_tag->offset);
        }
        return;
    }
    //Sys_DebugLog(LOG_FILENAME, "model = %d, anims = %d", tr_moveable->object_id, GetNumAnimationsForMoveable(tr, model_num));
    model->animation_count = TR_GetNumAnimationsForMoveable(tr, model_num);
    if(model->animation_count <= 0)
    {
        /*
         * the animation count must be >= 1
         */
        model->animation_count = 1;
    }

    /*
     *   Ok, let us calculate animations;
     *   there is no difficult:
     * - first 9 words are bounding box and frame offset coordinates.
     * - 10's word is a rotations count, must be equal to number of meshes in model.
     *   BUT! only in TR1. In TR2 - TR5 after first 9 words begins next section.
     * - in the next follows rotation's data. one word - one rotation, if rotation is one-axis (one angle).
     *   two words in 3-axis rotations (3 angles). angles are calculated with bit mask.
     */
    model->animations = (animation_frame_p)calloc(model->animation_count, sizeof(animation_frame_t));
    anim = model->animations;
    for(uint16_t i=0;i<model->animation_count;i++,anim++)
    {
        tr_animation = &tr->animations[tr_moveable->animation_index+i];
        frame_offset = tr_animation->frame_offset / 2;
        uint16_t l_start = 0x09;
        if(tr->game_version == TR_I || tr->game_version == TR_I_DEMO || tr->game_version == TR_I_UB)
        {
            l_start = 0x0A;
        }
        frame_step = tr_animation->frame_size;

        anim->id = i;
        anim->original_frame_rate = tr_animation->frame_rate;

        anim->speed_x = tr_animation->speed;
        anim->accel_x = tr_animation->accel;
        anim->speed_y = tr_animation->accel_lateral;
        anim->accel_y = tr_animation->speed_lateral;

        anim->anim_command = tr_animation->anim_command;
        anim->num_anim_commands = tr_animation->num_anim_commands;
        anim->state_id = tr_animation->state_id;

        anim->frames_count = TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index+i);

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.

        if( (anim->num_anim_commands > 0) && (anim->num_anim_commands <= 255) )
        {
            // Calculate current animation anim command block offset.
            int16_t *pointer = world->anim_commands + anim->anim_command;

            for(uint32_t count = 0; count < anim->num_anim_commands; count++, pointer++)
            {
                switch(*pointer)
                {
                    case TR_ANIMCOMMAND_PLAYEFFECT:
                    case TR_ANIMCOMMAND_PLAYSOUND:
                        // Recalculate absolute frame number to relative.
                        ///@FIXED: was unpredictable behavior.
                        *(pointer + 1) -= tr_animation->frame_start;
                        pointer += 2;
                        break;

                    case TR_ANIMCOMMAND_SETPOSITION:
                        // Parse through 3 operands.
                        pointer += 3;
                        break;

                    case TR_ANIMCOMMAND_JUMPDISTANCE:
                        // Parse through 2 operands.
                        pointer += 2;
                        break;

                    default:
                        // All other commands have no operands.
                        break;
                }
            }
        }


        if(anim->frames_count <= 0)
        {
            /*
             * number of animations must be >= 1, because frame contains base model offset
             */
            anim->frames_count = 1;
        }
        anim->frames = (bone_frame_p)calloc(anim->frames_count, sizeof(bone_frame_t));

        /*
         * let us begin to load animations
         */
        bone_frame = anim->frames;
        for(uint16_t j=0;j<anim->frames_count;j++,bone_frame++,frame_offset+=frame_step)
        {
            bone_frame->bone_tag_count = model->mesh_count;
            bone_frame->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            vec3_set_zero(bone_frame->pos);
            vec3_set_zero(bone_frame->move);
            TR_GetBFrameBB_Pos(tr, frame_offset, bone_frame);

            if(frame_offset >= tr->frame_data_size)
            {
                //Con_Printf("Bad frame offset");
                for(uint16_t k=0;k<bone_frame->bone_tag_count;k++)
                {
                    tree_tag = model->mesh_tree + k;
                    bone_tag = bone_frame->bone_tags + k;
                    rot[0] = 0.0;
                    rot[1] = 0.0;
                    rot[2] = 0.0;
                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                    vec3_copy(bone_tag->offset, tree_tag->offset);
                }
            }
            else
            {
                uint16_t l = l_start;
                for(uint16_t k=0;k<bone_frame->bone_tag_count;k++)
                {
                    tree_tag = model->mesh_tree + k;
                    bone_tag = bone_frame->bone_tags + k;
                    rot[0] = 0.0;
                    rot[1] = 0.0;
                    rot[2] = 0.0;
                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                    vec3_copy(bone_tag->offset, tree_tag->offset);

                    switch(tr->game_version)
                    {
                        case TR_I:                                              /* TR_I */
                        case TR_I_UB:
                        case TR_I_DEMO:
                            temp2 = tr->frame_data[frame_offset + l];
                            l ++;
                            temp1 = tr->frame_data[frame_offset + l];
                            l ++;
                            rot[0] = (float)((temp1 & 0x3ff0) >> 4);
                            rot[2] =-(float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                            rot[1] = (float)(temp2 & 0x03ff);
                            rot[0] *= 360.0 / 1024.0;
                            rot[1] *= 360.0 / 1024.0;
                            rot[2] *= 360.0 / 1024.0;
                            vec4_SetTRRotations(bone_tag->qrotate, rot);
                            break;

                        default:                                                /* TR_II + */
                            temp1 = tr->frame_data[frame_offset + l];
                            l ++;
                            if(tr->game_version >= TR_IV)
                            {
                                ang = (float)(temp1 & 0x0fff);
                                ang *= 360.0 / 4096.0;
                            }
                            else
                            {
                                ang = (float)(temp1 & 0x03ff);
                                ang *= 360.0 / 1024.0;
                            }

                            switch (temp1 & 0xc000)
                            {
                                case 0x4000:    // x only
                                    rot[0] = ang;
                                    rot[1] = 0;
                                    rot[2] = 0;
                                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                                    break;

                                case 0x8000:    // y only
                                    rot[0] = 0;
                                    rot[1] = 0;
                                    rot[2] =-ang;
                                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                                    break;

                                case 0xc000:    // z only
                                    rot[0] = 0;
                                    rot[1] = ang;
                                    rot[2] = 0;
                                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                                    break;

                                default:        // all three
                                    temp2 = tr->frame_data[frame_offset + l];
                                    rot[0] = (float)((temp1 & 0x3ff0) >> 4);
                                    rot[2] =-(float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                                    rot[1] = (float)(temp2 & 0x03ff);
                                    rot[0] *= 360.0 / 1024.0;
                                    rot[1] *= 360.0 / 1024.0;
                                    rot[2] *= 360.0 / 1024.0;
                                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                                    l ++;
                                    break;
                            };
                            break;
                    };
                }
            }
        }
    }

    /*
     * Animations interpolation to 1/30 sec like in original. Needed for correct state change works.
     */
    SkeletalModel_InterpolateFrames(model);
    /*
     * state change's loading
     */

#if LOG_ANIM_DISPATCHES
    if(model->animation_count > 1)
    {
        Sys_DebugLog(LOG_FILENAME, "MODEL[%d], anims = %d", model_num, model->animation_count);
    }
#endif
    anim = model->animations;
    for(uint16_t i=0;i<model->animation_count;i++,anim++)
    {
        anim->state_change_count = 0;
        anim->state_change = NULL;

        tr_animation = &tr->animations[tr_moveable->animation_index+i];
        int16_t j = tr_animation->next_animation - tr_moveable->animation_index;
        j &= 0x7fff;
        if((j >= 0) && (j < model->animation_count))
        {
            anim->next_anim = model->animations + j;
            anim->next_frame = tr_animation->next_frame - tr->animations[tr_animation->next_animation].frame_start;
            anim->next_frame %= anim->next_anim->frames_count;
            if(anim->next_frame < 0)
            {
                anim->next_frame = 0;
            }
#if LOG_ANIM_DISPATCHES
            Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, anim->next_anim->id, anim->next_frame);
#endif
        }
        else
        {
            anim->next_anim = NULL;
            anim->next_frame = 0;
        }

        anim->state_change_count = 0;
        anim->state_change = NULL;

        if((tr_animation->num_state_changes > 0) && (model->animation_count > 1))
        {
            state_change_p sch_p;
#if LOG_ANIM_DISPATCHES
            Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, (anim->next_anim)?(anim->next_anim->id):(-1), anim->next_frame);
#endif
            anim->state_change_count = tr_animation->num_state_changes;
            sch_p = anim->state_change = (state_change_p)malloc(tr_animation->num_state_changes * sizeof(state_change_t));

            for(uint16_t j=0;j<tr_animation->num_state_changes;j++,sch_p++)
            {
                tr_state_change_t *tr_sch;
                tr_sch = &tr->state_changes[j+tr_animation->state_change_offset];
                sch_p->id = tr_sch->state_id;
                sch_p->anim_dispatch = NULL;
                sch_p->anim_dispatch_count = 0;
                for(uint16_t l=0;l<tr_sch->num_anim_dispatches;l++)
                {
                    tr_anim_dispatch_t *tr_adisp = &tr->anim_dispatches[tr_sch->anim_dispatch+l];
                    uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                    if((next_anim_ind >= 0) &&(next_anim_ind < model->animation_count))
                    {
                        sch_p->anim_dispatch_count++;
                        sch_p->anim_dispatch = (anim_dispatch_p)realloc(sch_p->anim_dispatch, sch_p->anim_dispatch_count * sizeof(anim_dispatch_t));

                        anim_dispatch_p adsp = sch_p->anim_dispatch + sch_p->anim_dispatch_count - 1;
                        uint16_t next_frames_count = model->animations[next_anim - tr_moveable->animation_index].frames_count;
                        uint16_t next_frame = tr_adisp->next_frame - tr->animations[next_anim].frame_start;

                        uint16_t low  = tr_adisp->low  - tr_animation->frame_start;
                        uint16_t high = tr_adisp->high - tr_animation->frame_start;

                        adsp->frame_low  = low  % anim->frames_count;
                        adsp->frame_high = (high - 1) % anim->frames_count;
                        adsp->next_anim = next_anim - tr_moveable->animation_index;
                        adsp->next_frame = next_frame % next_frames_count;

#if LOG_ANIM_DISPATCHES
                        Sys_DebugLog(LOG_FILENAME, "anim_disp[%d], frames_count = %d: interval[%d.. %d], next_anim = %d, next_frame = %d", l,
                                    anim->frames_count, adsp->frame_low, adsp->frame_high,
                                    adsp->next_anim, adsp->next_frame);
#endif
                    }
                }
            }
        }
    }
    GenerateAnimCommandsTransform(model);
}

int TR_GetNumAnimationsForMoveable(class VT_Level *tr, size_t moveable_ind)
{
    int ret;
    tr_moveable_t *curr_moveable, *next_moveable;

    curr_moveable = &tr->moveables[moveable_ind];

    if(curr_moveable->animation_index == 0xFFFF)
    {
        return 0;
    }

    if(moveable_ind == tr->moveables_count-1)
    {
        ret = (int32_t)tr->animations_count - (int32_t)curr_moveable->animation_index;
        if(ret < 0)
        {
            return 1;
        }
        else
        {
            return ret;
        }
    }

    next_moveable = &tr->moveables[moveable_ind+1];
    if(next_moveable->animation_index == 0xFFFF)
    {
        if(moveable_ind + 2 < tr->moveables_count)                              // I hope there is no two neighboard movables with animation_index'es == 0xFFFF
        {
            next_moveable = &tr->moveables[moveable_ind+2];
        }
        else
        {
            return 1;
        }
    }

    ret = (next_moveable->animation_index <= tr->animations_count)?(next_moveable->animation_index):(tr->animations_count);
    ret -= (int32_t)curr_moveable->animation_index;

    return ret;
}


/*
 * It returns real animation count
 */
int TR_GetNumFramesForAnimation(class VT_Level *tr, size_t animation_ind)
{
    tr_animation_t *curr_anim, *next_anim;
    int ret;

    curr_anim = &tr->animations[animation_ind];
    if(curr_anim->frame_size <= 0)
    {
        return 1;                                                               // impossible!
    }

    if(animation_ind == tr->animations_count - 1)
    {
        ret = 2 * tr->frame_data_size - curr_anim->frame_offset;
        ret /= curr_anim->frame_size * 2;                                       /// it is fully correct!
        return ret;
    }

    next_anim = tr->animations + animation_ind + 1;
    ret = next_anim->frame_offset - curr_anim->frame_offset;
    ret /= curr_anim->frame_size * 2;

    return ret;
}

void TR_GetBFrameBB_Pos(class VT_Level *tr, size_t frame_offset, bone_frame_p bone_frame)
{
    unsigned short int *frame;

    if(frame_offset < tr->frame_data_size)
    {
        frame = tr->frame_data + frame_offset;
        bone_frame->bb_min[0] = (short int)frame[0];                            // x_min
        bone_frame->bb_min[1] = (short int)frame[4];                            // y_min
        bone_frame->bb_min[2] =-(short int)frame[3];                            // z_min

        bone_frame->bb_max[0] = (short int)frame[1];                            // x_max
        bone_frame->bb_max[1] = (short int)frame[5];                            // y_max
        bone_frame->bb_max[2] =-(short int)frame[2];                            // z_max

        bone_frame->pos[0] = (short int)frame[6];
        bone_frame->pos[1] = (short int)frame[8];
        bone_frame->pos[2] =-(short int)frame[7];
    }
    else
    {
        bone_frame->bb_min[0] = 0.0;
        bone_frame->bb_min[1] = 0.0;
        bone_frame->bb_min[2] = 0.0;

        bone_frame->bb_max[0] = 0.0;
        bone_frame->bb_max[1] = 0.0;
        bone_frame->bb_max[2] = 0.0;

        bone_frame->pos[0] = 0.0;
        bone_frame->pos[1] = 0.0;
        bone_frame->pos[2] = 0.0;
    }

    bone_frame->centre[0] = (bone_frame->bb_min[0] + bone_frame->bb_max[0]) / 2.0;
    bone_frame->centre[1] = (bone_frame->bb_min[1] + bone_frame->bb_max[1]) / 2.0;
    bone_frame->centre[2] = (bone_frame->bb_min[2] + bone_frame->bb_max[2]) / 2.0;
}

void TR_GenSkeletalModels(struct world_s *world, class VT_Level *tr)
{
    skeletal_model_p smodel;
    tr_moveable_t *tr_moveable;

    world->skeletal_model_count = tr->moveables_count;
    smodel = world->skeletal_models = (skeletal_model_p)calloc(world->skeletal_model_count, sizeof(skeletal_model_t));

    for(uint32_t i=0;i<world->skeletal_model_count;i++,smodel++)
    {
        tr_moveable = &tr->moveables[i];
        smodel->id = tr_moveable->object_id;
        smodel->mesh_count = tr_moveable->num_meshes;
        TR_GenSkeletalModel(world, i, smodel, tr);
        SkeletonModel_FillTransparency(smodel);
    }
}


void TR_GenEntities(struct world_s *world, class VT_Level *tr)
{
    int top;

    tr2_item_t *tr_item;
    entity_p entity;

    for(uint32_t i=0;i<tr->items_count;i++)
    {
        tr_item = &tr->items[i];
        entity = Entity_Create();
        entity->id = i;
        entity->transform[12] = tr_item->pos.x;
        entity->transform[13] =-tr_item->pos.z;
        entity->transform[14] = tr_item->pos.y;
        entity->angles[0] = tr_item->rotation;
        entity->angles[1] = 0.0;
        entity->angles[2] = 0.0;
        Entity_UpdateRotation(entity);
        if((tr_item->room >= 0) && ((uint32_t)tr_item->room < world->room_count))
        {
            entity->self->room = world->rooms + tr_item->room;
        }
        else
        {
            entity->self->room = NULL;
        }

        entity->trigger_layout  = (tr_item->flags & 0x3E00) >> 9;   ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
        entity->OCB             = tr_item->ocb;
        entity->timer           = 0.0;

        entity->self->collision_type = COLLISION_TYPE_KINEMATIC;
        entity->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        entity->move_type          = 0x0000;
        entity->inertia_linear     = 0.0;
        entity->inertia_angular[0] = 0.0;
        entity->inertia_angular[1] = 0.0;
        entity->move_type          = 0;

        entity->bf.animations.model = World_GetModelByID(world, tr_item->object_id);

        if(ent_ID_override != NULL)
        {
            if(entity->bf.animations.model == NULL)
            {
                top = lua_gettop(ent_ID_override);                              // save LUA stack
                lua_getglobal(ent_ID_override, "getOverridedID");               // add to the up of stack LUA's function
                lua_pushinteger(ent_ID_override, tr->game_version);             // add to stack first argument
                lua_pushinteger(ent_ID_override, tr_item->object_id);           // add to stack second argument
                if (lua_CallAndLog(ent_ID_override, 2, 1, 0))                   // call that function
                {
                    entity->bf.animations.model = World_GetModelByID(world, lua_tointeger(ent_ID_override, -1));
                }
                lua_settop(ent_ID_override, top);                               // restore LUA stack
            }

            top = lua_gettop(ent_ID_override);                                  // save LUA stack
            lua_getglobal(ent_ID_override, "getOverridedAnim");                 // add to the up of stack LUA's function
            lua_pushinteger(ent_ID_override, tr->game_version);                 // add to stack first argument
            lua_pushinteger(ent_ID_override, tr_item->object_id);               // add to stack second argument
            if (lua_CallAndLog(ent_ID_override, 2, 1, 0))                       // call that function
            {
                int replace_anim_id = lua_tointeger(ent_ID_override, -1);
                if(replace_anim_id > 0)
                {
                    skeletal_model_s* replace_anim_model = World_GetModelByID(world, replace_anim_id);
                    animation_frame_p ta;
                    uint16_t tc;
                    SWAPT(entity->bf.animations.model->animations, replace_anim_model->animations, ta);
                    SWAPT(entity->bf.animations.model->animation_count, replace_anim_model->animation_count, tc);
                }
            }
            lua_settop(ent_ID_override, top);                                      // restore LUA stack

        }

        if(entity->bf.animations.model == NULL)
        {
            // SPRITE LOADING
            sprite_p sp = World_GetSpriteByID(tr_item->object_id, world);
            if(sp && entity->self->room)
            {
                room_sprite_p rsp;
                int sz = ++entity->self->room->sprites_count;
                entity->self->room->sprites = (room_sprite_p)realloc(entity->self->room->sprites, sz * sizeof(room_sprite_t));
                rsp = entity->self->room->sprites + sz - 1;
                rsp->sprite = sp;
                rsp->pos[0] = entity->transform[12];
                rsp->pos[1] = entity->transform[13];
                rsp->pos[2] = entity->transform[14];
                rsp->was_rendered = 0;
            }

            Entity_Clear(entity);
            free(entity);
            continue;                                                           // that entity has no model. may be it is a some trigger or look at object
        }

        if(tr->game_version < TR_II && tr_item->object_id == 83)                ///@FIXME: brutal magick hardcode! ;-)
        {
            Entity_Clear(entity);                                               // skip PSX save model
            free(entity);
            continue;
        }

        SSBoneFrame_CreateFromModel(&entity->bf, entity->bf.animations.model);

        if(0 == tr_item->object_id)                                             // Lara is unical model
        {
            skeletal_model_p tmp, LM;                                           // LM - Lara Model

            entity->move_type = MOVE_ON_FLOOR;
            world->Character = entity;
            entity->self->collision_type = COLLISION_TYPE_ACTOR;
            entity->self->collision_shape = COLLISION_SHAPE_TRIMESH_CONVEX;
            entity->bf.animations.model->hide = 0;
            entity->type_flags |= ENTITY_TYPE_TRIGGER_ACTIVATOR;
            LM = (skeletal_model_p)entity->bf.animations.model;

            top = lua_gettop(engine_lua);
            lua_pushinteger(engine_lua, entity->id);
            lua_setglobal(engine_lua, "player");
            lua_settop(engine_lua, top);

            switch(tr->game_version)
            {
                case TR_I:
                    if(gameflow_manager.CurrentLevelID == 0)
                    {
                        LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_ALTERNATE_TR1);
                        if(LM)
                        {
                            // In TR1, Lara has unified head mesh for all her alternate skins.
                            // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                            SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count - 1);
                        }
                    }
                    break;

                case TR_III:
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_TR3);
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        tmp = World_GetModelByID(world, 11);                   // moto / quadro cycle animations
                        if(tmp)
                        {
                            SkeletonCopyMeshes(tmp->mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        }
                    }
                    break;

                case TR_IV:
                case TR_IV_DEMO:
                case TR_V:
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_TR45);                         // base skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_JOINTS_TR45);                         // skin skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes2(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    FillSkinnedMeshMap(&world->skeletal_models[0]);
                    break;
            };

            for(uint16_t j=0;j<entity->bf.bone_tag_count;j++)
            {
                entity->bf.bone_tags[j].mesh_base = entity->bf.animations.model->mesh_tree[j].mesh_base;
                entity->bf.bone_tags[j].mesh_skin = entity->bf.animations.model->mesh_tree[j].mesh_skin;
                entity->bf.bone_tags[j].mesh_slot = NULL;
            }
            Entity_SetAnimation(world->Character, TR_ANIMATION_LARA_STAY_IDLE, 0);
            BT_GenEntityRigidBody(entity);
            Character_Create(entity);
            entity->character->Height = 768.0;
            entity->character->state_func = State_Control_Lara;

            continue;
        }

        Entity_SetAnimation(entity, 0, 0);                                      // Set zero animation and zero frame
        Res_SetEntityModelProperties(entity);
        Entity_RebuildBV(entity);
        Room_AddEntity(entity->self->room, entity);
        World_AddEntity(world, entity);

        BT_GenEntityRigidBody(entity);

        if((entity->self->collision_type & 0x0001) == 0)
        {
            Entity_DisableCollision(entity);
        }
    }
}


void TR_GenSamples(struct world_s *world, class VT_Level *tr)
{
    uint8_t      *pointer = tr->samples_data;
    int8_t        flag;
    uint32_t      ind1, ind2;
    uint32_t      comp_size, uncomp_size;
    uint32_t      i;

    // Generate new buffer array.
    world->audio_buffers_count = tr->samples_count;
    world->audio_buffers = (ALuint*)malloc(world->audio_buffers_count * sizeof(ALuint));
    memset(world->audio_buffers, 0, sizeof(ALuint) * world->audio_buffers_count);
    alGenBuffers(world->audio_buffers_count, world->audio_buffers);

    // Generate stream track map array.
    // We use scripted amount of tracks to define map bounds.
    // If script had no such parameter, we define map bounds by default.
    world->stream_track_map_count = lua_GetNumTracks(engine_lua);
    if(world->stream_track_map_count == 0) world->stream_track_map_count = TR_AUDIO_STREAM_MAP_SIZE;
    world->stream_track_map = (uint8_t*)malloc(world->stream_track_map_count * sizeof(uint8_t));
    memset(world->stream_track_map, 0, sizeof(uint8_t) * world->stream_track_map_count);

    // Generate new audio effects array.
    world->audio_effects_count = tr->sound_details_count;
    world->audio_effects =  (audio_effect_t*)malloc(tr->sound_details_count * sizeof(audio_effect_t));
    memset(world->audio_effects, 0xFF, sizeof(audio_effect_t) * tr->sound_details_count);

    // Generate new audio emitters array.
    world->audio_emitters_count = tr->sound_sources_count;
    world->audio_emitters = (audio_emitter_t*)malloc(tr->sound_sources_count * sizeof(audio_emitter_t));
    memset(world->audio_emitters, 0, sizeof(audio_emitter_t) * tr->sound_sources_count);

    // Copy sound map.
    world->audio_map = tr->soundmap;
    tr->soundmap = NULL;                   /// without it VT destructor free(tr->soundmap)

    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(pointer)
    {
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                world->audio_map_count = TR_AUDIO_MAP_SIZE_TR1;

                for(i = 0; i < world->audio_buffers_count-1; i++)
                {
                    pointer = tr->samples_data + tr->sample_indices[i];
                    uint32_t size = tr->sample_indices[(i+1)] - tr->sample_indices[i];
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, size);
                }
                i = world->audio_buffers_count-1;
                Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, (tr->samples_count - tr->sample_indices[i]));
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
                world->audio_map_count = (tr->game_version == TR_III)?(TR_AUDIO_MAP_SIZE_TR3):(TR_AUDIO_MAP_SIZE_TR2);
                ind1 = 0;
                ind2 = 0;
                flag = 0;
                i = 0;
                while(pointer < tr->samples_data + tr->samples_data_size - 4)
                {
                    pointer = tr->samples_data + ind2;
                    if(0x46464952 == *((int32_t*)pointer))  // RIFF
                    {
                        if(flag == 0x00)
                        {
                            ind1 = ind2;
                            flag = 0x01;
                        }
                        else
                        {
                            uncomp_size = ind2 - ind1;
                            Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], tr->samples_data + ind1, uncomp_size);
                            i++;
                            if(i > world->audio_buffers_count - 1)
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                uncomp_size = tr->samples_data_size - ind1;
                pointer = tr->samples_data + ind1;
                if(i < world->audio_buffers_count)
                {
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, uncomp_size);
                }
                break;

            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                world->audio_map_count = (tr->game_version == TR_V)?(TR_AUDIO_MAP_SIZE_TR5):(TR_AUDIO_MAP_SIZE_TR4);

                for(i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                world->audio_map_count = TR_AUDIO_MAP_SIZE_NONE;
                free(tr->samples_data);
                tr->samples_data = NULL;
                tr->samples_data_size = 0;
                return;
        }

        free(tr->samples_data);
        tr->samples_data = NULL;
        tr->samples_data_size = 0;
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(i = 0; i < world->audio_effects_count; i++)
    {
        if(tr->game_version < TR_III)
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 32767.0; // Max. volume in TR1/TR2 is 32767.
            world->audio_effects[i].chance = tr->sound_details[i].chance;
        }
        else if(tr->game_version > TR_III)
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            world->audio_effects[i].chance = tr->sound_details[i].chance * 255;
        }
        else
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            world->audio_effects[i].chance = tr->sound_details[i].chance * 127;
        }

        world->audio_effects[i].rand_gain_var  = 50;
        world->audio_effects[i].rand_pitch_var = 50;

        world->audio_effects[i].pitch = (float)(tr->sound_details[i].pitch) / 127.0 + 1.0;
        world->audio_effects[i].range = (float)(tr->sound_details[i].sound_range) * 1024.0;

        world->audio_effects[i].rand_pitch = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_PITCH);
        world->audio_effects[i].rand_gain  = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_VOLUME);

        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    case 0x01:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    default:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            case TR_II:
            case TR_II_DEMO:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    case 0x01:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_WAIT;
                        break;
                    case 0x03:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    default:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            default:
                world->audio_effects[i].loop = (tr->sound_details[i].num_samples_and_flags_1 & TR_AUDIO_LOOP_LOOPED);
                break;
        }

        world->audio_effects[i].sample_index =  tr->sound_details[i].sample;
        world->audio_effects[i].sample_count = (tr->sound_details[i].num_samples_and_flags_1 >> 2) & TR_AUDIO_SAMPLE_NUMBER_MASK;
    }

    // Try to override samples via script.
    // If there is no script entry exist, we just leave default samples.
    // NB! We need to override samples AFTER audio effects array is inited, as override
    //     routine refers to existence of certain audio effect in level.

    Audio_LoadOverridedSamples(world);

    // Hardcoded version-specific fixes!

    switch(world->version)
    {
        case TR_I:
        case TR_I_DEMO:
        case TR_I_UB:
            // Fix for underwater looped sound.
            if ((world->audio_map[TR_AUDIO_SOUND_UNDERWATER]) >= 0)
            {
                world->audio_effects[(world->audio_map[TR_AUDIO_SOUND_UNDERWATER])].loop = TR_AUDIO_LOOP_LOOPED;
            }
            break;
        case TR_II:
            // Fix for helicopter sound range.
            if ((world->audio_map[297]) >= 0)
            {
                world->audio_effects[(world->audio_map[297])].range *= 10.0;
            }
            break;
    }

    // Cycle through sound emitters and
    // parse them to native OpenTomb sound emitters structure.

    for(i = 0; i < world->audio_emitters_count; i++)
    {
        world->audio_emitters[i].emitter_index = i;
        world->audio_emitters[i].sound_index   =  tr->sound_sources[i].sound_id;
        world->audio_emitters[i].position[0]   =  tr->sound_sources[i].x;
        world->audio_emitters[i].position[1]   =  tr->sound_sources[i].z;
        world->audio_emitters[i].position[2]   = -tr->sound_sources[i].y;
        world->audio_emitters[i].flags         =  tr->sound_sources[i].flags;
    }
}


void Res_EntityToItem(RedBlackNode_p n)
{
    base_item_p item = (base_item_p)n->data;

    for(uint32_t i=0;i<engine_world.room_count;i++)
    {
        engine_container_p cont = engine_world.rooms[i].containers;
        for(;cont;cont=cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p ent = (entity_p)cont->object;
                if(ent->bf.animations.model->id == item->world_model_id)
                {
                    char buf[64] = {0};
                    snprintf(buf, 64, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", ent->id, ent->id);
                    luaL_dostring(engine_lua, buf);
                    snprintf(buf, 32, "pickup_init(%d, %d);", ent->id, item->id);
                    luaL_dostring(engine_lua, buf);
                    Entity_DisableCollision(ent);
                }
            }
        }
    }

    if(n->right)
    {
        Res_EntityToItem(n->right);
    }

    if(n->left)
    {
        Res_EntityToItem(n->left);
    }
}
