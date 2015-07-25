
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <numeric>

#include <SDL2/SDL.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "vt/vt_level.h"
#include "gl_util.h"
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
#include "bsp_tree.h"
#include "shader_description.h"

#include <lua.hpp>
#include "LuaState.h"

lua::State objects_flags_conf;
lua::State ent_ID_override;
lua::State level_script;


void Res_SetEntityModelProperties(std::shared_ptr<Entity> ent)
{
    if(ent->m_bf.animations.model != NULL && objects_flags_conf["getEntityModelProperties"])
    {
        uint16_t flg;
        lua::tie(ent->m_self->collision_type, ent->m_self->collision_shape, ent->m_bf.animations.model->hide, flg) = objects_flags_conf["getEntityModelProperties"](engine_world.version, ent->m_bf.animations.model->id);
        ent->m_typeFlags |= flg;
    }

    if(ent->m_bf.animations.model != NULL && level_script["getEntityModelProperties"])
    {
        uint16_t flg;
        lua::tie(ent->m_self->collision_type, ent->m_self->collision_shape, ent->m_bf.animations.model->hide, flg) = level_script["getEntityModelProperties"](engine_world.version, ent->m_bf.animations.model->id);
        ent->m_typeFlags &= ~(ENTITY_TYPE_TRAVERSE | ENTITY_TYPE_TRAVERSE_FLOOR);
        ent->m_typeFlags |= flg;                 // get traverse information
    }
}


void Res_SetEntityFunction(std::shared_ptr<Entity> ent)
{
    if(ent->m_bf.animations.model)
    {
        const char* funcName = objects_flags_conf["getEntityFunction"](engine_world.version, ent->m_bf.animations.model->id);
        if(funcName)
            Res_CreateEntityFunc(engine_lua, funcName ? funcName : std::string(), ent->id());
    }
}


void Res_CreateEntityFunc(lua::State& state, const std::string& func_name, int entity_id)
{
    char bufC[64] = "";
    snprintf(bufC, 64, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", entity_id, entity_id);
    state.doString(bufC);
    state[(func_name + "_init").c_str()](entity_id);
}

void Res_GenEntityFunctions(std::map<uint32_t, std::shared_ptr<Entity> > &entities)
{
    for(const auto& pair : entities)
        Res_SetEntityFunction(pair.second);
}

void Res_SetStaticMeshProperties(std::shared_ptr<StaticMesh> r_static)
{
    lua::tie(r_static->self->collision_type, r_static->self->collision_shape, r_static->hide) = level_script["getStaticMeshProperties"](r_static->object_id);
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


void Res_Sector_SetTweenFloorConfig(SectorTween *tween)
{
    if(tween->floor_corners[0][2] > tween->floor_corners[1][2])
    {
        std::swap(tween->floor_corners[0][2], tween->floor_corners[1][2]);
        std::swap(tween->floor_corners[2][2], tween->floor_corners[3][2]);
    }

    if(tween->floor_corners[3][2] > tween->floor_corners[2][2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_2TRIANGLES;              // like a butterfly
    }
    else if((tween->floor_corners[0][2] != tween->floor_corners[1][2]) &&
       (tween->floor_corners[2][2] != tween->floor_corners[3][2]))
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_QUAD;
    }
    else if(tween->floor_corners[0][2] != tween->floor_corners[1][2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT;
    }
    else if(tween->floor_corners[2][2] != tween->floor_corners[3][2])
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT;
    }
    else
    {
        tween->floor_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
    }
}

void Res_Sector_SetTweenCeilingConfig(SectorTween *tween)
{
    if(tween->ceiling_corners[0][2] > tween->ceiling_corners[1][2])
    {
        std::swap(tween->ceiling_corners[0][2], tween->ceiling_corners[1][2]);
        std::swap(tween->ceiling_corners[2][2], tween->ceiling_corners[3][2]);
    }

    if(tween->ceiling_corners[3][2] > tween->ceiling_corners[2][2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_2TRIANGLES;            // like a butterfly
    }
    else if((tween->ceiling_corners[0][2] != tween->ceiling_corners[1][2]) &&
       (tween->ceiling_corners[2][2] != tween->ceiling_corners[3][2]))
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_QUAD;
    }
    else if(tween->ceiling_corners[0][2] != tween->ceiling_corners[1][2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT;
    }
    else if(tween->ceiling_corners[2][2] != tween->ceiling_corners[3][2])
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT;
    }
    else
    {
        tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
    }
}

int Res_Sector_IsWall(RoomSector* ws, RoomSector* ns)
{
    if((ws->portal_to_room < 0) && (ns->portal_to_room < 0) && (ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL))
    {
        return 1;
    }

    if((ns->portal_to_room < 0) && (ns->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (ws->portal_to_room >= 0))
    {
        ws = ws->checkPortalPointer();
        if((ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) || !ns->is2SidePortals(ws))
        {
            return 1;
        }
    }

    return 0;
}

///@TODO: resolve floor >> ceiling case
void Res_Sector_GenTweens(std::shared_ptr<Room> room, SectorTween *room_tween)
{
    for(uint16_t h = 0; h < room->sectors_y-1; h++)
    {
        for(uint16_t w = 0; w < room->sectors_x-1; w++)
        {
            // Init X-plane tween [ | ]

            RoomSector* current_heightmap = &room->sectors[(w * room->sectors_y + h) ];
            RoomSector* next_heightmap    = current_heightmap + 1;
            char joined_floors = 0;
            char joined_ceilings = 0;

            /* XY corners coordinates must be calculated from native room sector */
            room_tween->floor_corners[0][1] = current_heightmap->floor_corners[0][1];
            room_tween->floor_corners[1][1] = room_tween->floor_corners[0][1];
            room_tween->floor_corners[2][1] = room_tween->floor_corners[0][1];
            room_tween->floor_corners[3][1] = room_tween->floor_corners[0][1];
            room_tween->floor_corners[0][0] = current_heightmap->floor_corners[0][0];
            room_tween->floor_corners[1][0] = room_tween->floor_corners[0][0];
            room_tween->floor_corners[2][0] = current_heightmap->floor_corners[1][0];
            room_tween->floor_corners[3][0] = room_tween->floor_corners[2][0];

            room_tween->ceiling_corners[0][1] = current_heightmap->ceiling_corners[0][1];
            room_tween->ceiling_corners[1][1] = room_tween->ceiling_corners[0][1];
            room_tween->ceiling_corners[2][1] = room_tween->ceiling_corners[0][1];
            room_tween->ceiling_corners[3][1] = room_tween->ceiling_corners[0][1];
            room_tween->ceiling_corners[0][0] = current_heightmap->ceiling_corners[0][0];
            room_tween->ceiling_corners[1][0] = room_tween->ceiling_corners[0][0];
            room_tween->ceiling_corners[2][0] = current_heightmap->ceiling_corners[1][0];
            room_tween->ceiling_corners[3][0] = room_tween->ceiling_corners[2][0];

            if(w > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))                                                           // Init X-plane tween [ | ]
                {
                    if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[0][2];
                        room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                        room_tween->floor_corners[3][2] = next_heightmap->floor_corners[2][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else
                    {
                        /************************** SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || current_heightmap->is2SidePortals(next_heightmap))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            next_heightmap    = next_heightmap->checkPortalPointer();
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                                    room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                                    room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                                    room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                                    Res_Sector_SetTweenFloorConfig(room_tween);
                                    joined_floors = 1;
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                                    room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                                    room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                                    room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                                    Res_Sector_SetTweenCeilingConfig(room_tween);
                                    joined_ceilings = 1;
                                }
                            }
                        }
                    }
                }

                current_heightmap = &room->sectors[ (w * room->sectors_y + h) ];
                next_heightmap    = current_heightmap + 1;
                if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = next_heightmap->checkPortalPointer();
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = current_heightmap->checkPortalPointer();
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                    }
                }

                current_heightmap = &room->sectors[ (w * room->sectors_y + h) ];
                next_heightmap    = current_heightmap + 1;
                if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != NULL) && (current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = next_heightmap->checkPortalPointer();
                        if(next_heightmap->owner_room->id == current_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = current_heightmap->checkPortalPointer();
                        if(current_heightmap->owner_room->id == next_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                        Res_Sector_SetTweenCeilingConfig(room_tween);
                    }
                }
            }

            /*****************************************************************************************************
             ********************************   CENTRE  OF  THE  ALGORITHM   *************************************
             *****************************************************************************************************/

            room_tween++;
            current_heightmap = &room->sectors[ (w * room->sectors_y + h) ];
            next_heightmap    = &room->sectors[ ((w + 1) * room->sectors_y + h) ];
            room_tween->floor_corners[0][0] = current_heightmap->floor_corners[1][0];
            room_tween->floor_corners[1][0] = room_tween->floor_corners[0][0];
            room_tween->floor_corners[2][0] = room_tween->floor_corners[0][0];
            room_tween->floor_corners[3][0] = room_tween->floor_corners[0][0];
            room_tween->floor_corners[0][1] = current_heightmap->floor_corners[1][1];
            room_tween->floor_corners[1][1] = room_tween->floor_corners[0][1];
            room_tween->floor_corners[2][1] = current_heightmap->floor_corners[2][1];
            room_tween->floor_corners[3][1] = room_tween->floor_corners[2][1];

            room_tween->ceiling_corners[0][0] = current_heightmap->ceiling_corners[1][0];
            room_tween->ceiling_corners[1][0] = room_tween->ceiling_corners[0][0];
            room_tween->ceiling_corners[2][0] = room_tween->ceiling_corners[0][0];
            room_tween->ceiling_corners[3][0] = room_tween->ceiling_corners[0][0];
            room_tween->ceiling_corners[0][1] = current_heightmap->ceiling_corners[1][1];
            room_tween->ceiling_corners[1][1] = room_tween->ceiling_corners[0][1];
            room_tween->ceiling_corners[2][1] = current_heightmap->ceiling_corners[2][1];
            room_tween->ceiling_corners[3][1] = room_tween->ceiling_corners[2][1];

            joined_floors = 0;
            joined_ceilings = 0;

            if(h > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                {
                    // Init Y-plane tween  [ - ]
                    if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                        room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[2][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0][2] = next_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                        room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->floor_corners[3][2] = next_heightmap->floor_corners[3][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                        joined_floors = 1;
                        joined_ceilings = 1;
                    }
                    else
                    {
                        /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || current_heightmap->is2SidePortals(next_heightmap))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            next_heightmap    = next_heightmap->checkPortalPointer();
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                                    room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                                    room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                                    room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                                    Res_Sector_SetTweenFloorConfig(room_tween);
                                    joined_floors = 1;
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                                    room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                                    room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                                    room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                                    Res_Sector_SetTweenCeilingConfig(room_tween);
                                    joined_ceilings = 1;
                                }
                            }
                        }
                    }
                }

                current_heightmap = &room->sectors[ (w * room->sectors_y + h) ];
                next_heightmap    = &room->sectors[ ((w + 1) * room->sectors_y + h) ];
                if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = next_heightmap->checkPortalPointer();
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = current_heightmap->checkPortalPointer();
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                    }
                }

                current_heightmap = &room->sectors[ (w * room->sectors_y + h) ];
                next_heightmap    = &room->sectors[ ((w + 1) * room->sectors_y + h) ];
                if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != NULL) && (current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = next_heightmap->checkPortalPointer();
                        if(next_heightmap->owner_room->id == current_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = current_heightmap->checkPortalPointer();
                        if(current_heightmap->owner_room->id == next_heightmap->sector_below->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->pos);
                            if(rs && ((uint32_t)rs->portal_to_room == current_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }

                    if((valid == 1) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
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

    if(!engine_world.getEntityByID(entity_index)) return true;

    int32_t *curr_table_index = lookup_table;

    while(*curr_table_index != -1)
    {
        if(*curr_table_index == (int32_t)entity_index) return true;
        curr_table_index++;
    }

    *curr_table_index = (int32_t)entity_index;
    return false;
}

int TR_Sector_TranslateFloorData(RoomSector* sector, class VT_Level *tr)
{
    if(!sector || (sector->trig_index <= 0) || (sector->trig_index >= tr->floor_data_size))
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
        // uint16_t function_value = ((*entry) & 0x00E0) >> 5;        // 0b00000000 11100000  TR_III+
        uint16_t sub_function   = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        end_bit = ((*entry) & 0x8000) >> 15;       // 0b10000000 00000000

        entry++;

        switch(function)
        {
            case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                if(sub_function == 0x00)
                {
                    if(*entry < engine_world.rooms.size())
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
                        sector->floor_corners[2][2] -= ((btScalar)raw_x_slant * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= ((btScalar)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->floor_corners[0][2] -= (std::abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= (std::abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->floor_corners[0][2] -= ((btScalar)raw_y_slant * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= ((btScalar)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->floor_corners[1][2] -= (std::abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= (std::abs((btScalar)raw_y_slant) * TR_METERING_STEP);
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
                        sector->ceiling_corners[3][2] += ((btScalar)raw_x_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += ((btScalar)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->ceiling_corners[1][2] += (std::abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[0][2] += (std::abs((btScalar)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->ceiling_corners[1][2] += ((btScalar)raw_y_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += ((btScalar)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->ceiling_corners[0][2] += (std::abs((btScalar)raw_y_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += (std::abs((btScalar)raw_y_slant) * TR_METERING_STEP);
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
                                                snprintf(buf, 256, " if((switch_state == 0) and switch_sectorstatus) then \n   setEntitySectorStatus(%d, false); \n   setEntityTimer(%d, %d); \n", operands, operands, timer_field);
                                                if(only_once)
                                                {
                                                    // Just lock out activator, no anti-action needed.
                                                    snprintf(buf2, 128, " setEntityLock(%d, true) \n", operands);
                                                }
                                                else
                                                {
                                                    // Create statement for antitriggering a switch.
                                                    snprintf(buf2, 256, " elseif((switch_state == 1) and switch_sectorstatus) then\n   setEntitySectorStatus(%d, false); \n   setEntityTimer(%d, 0); \n", operands, operands);
                                                }
                                            }
                                            else
                                            {
                                                // Ordinary type case (e.g. heavy switch).
                                                snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, %d); \n", operands, mask_mode, only_once?"true":"false", timer_field);
                                                strcat(item_events, buf);
                                                snprintf(buf, 128, " if(not switch_sectorstatus) then \n   setEntitySectorStatus(entity_index, true) \n");
                                            }
                                            break;

                                        case TR_ACTIVATOR_KEY:
                                            snprintf(buf, 256, " if((getEntityLock(%d)) and (not getEntitySectorStatus(%d))) then \n   setEntitySectorStatus(%d, true); \n", operands, operands, operands);
                                            break;

                                        case TR_ACTIVATOR_PICKUP:
                                            snprintf(buf, 256, " if((not getEntityEnability(%d)) and (not getEntitySectorStatus(%d))) then \n   setEntitySectorStatus(%d, true); \n", operands, operands, operands);
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
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, %d); \n", operands, mask_mode, only_once?"true":"false", timer_field);
                                            strcat(item_events, buf);
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, 0); \n", operands, mask_mode, only_once?"true":"false");
                                            strcat(anti_events, buf);
                                        }
                                        else
                                        {
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, 0x%02X, %d, %s, %d); \n", operands, trigger_mask, mask_mode, only_once?"true":"false", timer_field);
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
                                    snprintf(buf, 128, "   setFlipMap(%d, switch_mask, 1); \n   setFlipState(%d, true); \n", operands, operands);
                                    strcat(single_events, buf);
                                }
                                else
                                {
                                    snprintf(buf, 128, "   setFlipMap(%d, 0x%02X, 0); \n   setFlipState(%d, true); \n", operands, trigger_mask, operands);
                                    strcat(single_events, buf);
                                }
                                break;

                            case TR_FD_TRIGFUNC_FLIPON:
                                // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                                // the switch with FLIP_ON trigger, room will remain flipped.
                                snprintf(buf, 128, "   setFlipState(%d, true); \n", operands);
                                strcat(single_events, buf);
                                break;

                            case TR_FD_TRIGFUNC_FLIPOFF:
                                // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                                // the switch with FLIP_OFF trigger, room will remain unflipped.
                                snprintf(buf, 128, "   setFlipState(%d, false); \n", operands);
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
                                // Override for looped BGM tracks in TR1: if there are any sectors
                                // triggering looped tracks, ignore it, as BGM is always set in script.
                                if(engine_world.version < TR_II)
                                {
                                    int looped;
                                    lua_GetSoundtrack(engine_lua, operands, nullptr, nullptr, &looped);
                                    if(looped == TR_AUDIO_STREAM_TYPE_BACKGROUND) break;
                                }

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
                                strcat(once_condition, " if(not getEntitySectorStatus(entity_index)) then \n");
                                strcat(script, once_condition);
                                strcat(script, single_events);
                                strcat(script, "   setEntitySectorStatus(entity_index, true); \n");

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
                        engine_lua.doString(script);
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
                    // uint16_t slope_func = ((*entry) & 0x001F);            // 0b00000000 00011111

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

                        sector->floor_corners[0][2] -= overall_adjustment - ((btScalar)slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= overall_adjustment - ((btScalar)slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= overall_adjustment - ((btScalar)slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= overall_adjustment - ((btScalar)slope_t11 * TR_METERING_STEP);

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

                        sector->floor_corners[0][2] -= overall_adjustment - ((btScalar)slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= overall_adjustment - ((btScalar)slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= overall_adjustment - ((btScalar)slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= overall_adjustment - ((btScalar)slope_t11 * TR_METERING_STEP);

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

                        sector->ceiling_corners[0][2] += overall_adjustment - (btScalar)(slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1][2] += overall_adjustment - (btScalar)(slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += overall_adjustment - (btScalar)(slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += overall_adjustment - (btScalar)(slope_t12 * TR_METERING_STEP);

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

                        sector->ceiling_corners[0][2] += overall_adjustment - (btScalar)(slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1][2] += overall_adjustment - (btScalar)(slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += overall_adjustment - (btScalar)(slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += overall_adjustment - (btScalar)(slope_t12 * TR_METERING_STEP);

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

    return ret;
}

void Res_Sector_FixHeights(RoomSector* sector)
{
    if(sector->floor == TR_METERING_WALLHEIGHT)
    {
        sector->floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
    }
    if(sector->ceiling == TR_METERING_WALLHEIGHT)
    {
        sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
    }

    // Fix non-material crevices

    for(size_t i = 0; i < 4; i++)
    {
        if(sector->ceiling_corners[i].m_floats[2] == sector->floor_corners[i].m_floats[2])
            sector->ceiling_corners[i].m_floats[2] += LARA_HANG_VERTICAL_EPSILON;
    }
}


void GenerateAnimCommandsTransform(SkeletalModel* model)
{
    if(engine_world.anim_commands.empty())
    {
        return;
    }
    //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model->id);
    for(size_t anim = 0; anim < model->animations.size(); anim++)
    {
        if(model->animations[anim].num_anim_commands > 255)
        {
            continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
        }

        AnimationFrame* af  = &model->animations[anim];
        if(af->num_anim_commands == 0)
            continue;

        assert( af->anim_command < engine_world.anim_commands.size() );
        int16_t *pointer    = &engine_world.anim_commands[af->anim_command];

        for(uint32_t i = 0; i < af->num_anim_commands; i++)
        {
            const auto command = *pointer;
            ++pointer;
            switch(command)
            {
                case TR_ANIMCOMMAND_SETPOSITION:
                    // This command executes ONLY at the end of animation.
                    af->frames[af->frames.size()-1].move[0] = (btScalar)pointer[0];                          // x = x;
                    af->frames[af->frames.size()-1].move[2] =-(btScalar)pointer[1];                          // z =-y
                    af->frames[af->frames.size()-1].move[1] = (btScalar)pointer[2];                          // y = z
                    af->frames[af->frames.size()-1].command |= ANIM_CMD_MOVE;
                    //Sys_DebugLog("anim_transform.txt", "move[anim = %d, frame = %d, frames = %d]", anim, af->frames.size()-1, af->frames.size());
                    pointer += 3;
                    break;

                case TR_ANIMCOMMAND_JUMPDISTANCE:
                    af->frames[af->frames.size()-1].v_Vertical   = pointer[0];
                    af->frames[af->frames.size()-1].v_Horizontal = pointer[1];
                    af->frames[af->frames.size()-1].command |= ANIM_CMD_JUMP;
                    pointer += 2;
                    break;

                case TR_ANIMCOMMAND_EMPTYHANDS:
                    break;

                case TR_ANIMCOMMAND_KILL:
                    break;

                case TR_ANIMCOMMAND_PLAYSOUND:
                    pointer += 2;
                    break;

                case TR_ANIMCOMMAND_PLAYEFFECT:
                    switch(pointer[1] & 0x3FFF)
                    {
                        case TR_EFFECT_CHANGEDIRECTION:
                            af->frames[pointer[0]].command |= ANIM_CMD_CHANGE_DIRECTION;
                            ConsoleInfo::instance().printf("ROTATE: anim = %d, frame = %d of %d", anim, pointer[0], af->frames.size());
                            //Sys_DebugLog("anim_transform.txt", "dir[anim = %d, frame = %d, frames = %d]", anim, frame, af->frames.size());
                            break;
                    }
                    pointer += 2;
                    break;
            }
        }
    }
}


bool TR_IsSectorsIn2SideOfPortal(RoomSector* s1, RoomSector* s2, const Portal& p)
{
    if((s1->pos[0] == s2->pos[0]) && (s1->pos[1] != s2->pos[1]) && (std::abs(p.normal.normal[1]) > 0.99))
    {
        btScalar min_x, max_x, min_y, max_y;
        max_x = min_x = p.vertices.front().x();
        for(const auto& v : p.vertices)
        {
            if(v.x() > max_x)
            {
                max_x = v.x();
            }
            if(v.x() < min_x)
            {
                min_x = v.x();
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

        if((s1->pos[0] < max_x) && (s1->pos[0] > min_x) && (p.centre[1] < max_y) && (p.centre[1] > min_y))
        {
            return true;
        }
    }
    else if((s1->pos[0] != s2->pos[0]) && (s1->pos[1] == s2->pos[1]) && (std::abs(p.normal.normal[0]) > 0.99))
    {
        btScalar min_x, max_x, min_y, max_y;
        max_y = min_y = p.vertices.front().y();
        for(const auto& v : p.vertices)
        {
            if(v.y() > max_y)
            {
                max_y = v.y();
            }
            if(v.y() < min_y)
            {
                min_y = v.y();
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

        if((p.centre[0] < max_x) && (p.centre[0] > min_x) && (s1->pos[1] < max_y) && (s1->pos[1] > min_y))
        {
            return true;
        }
    }

    return false;
}

void TR_Sector_Calculate(World *world, class VT_Level *tr, long int room_index)
{
    std::shared_ptr<Room> room = world->rooms[room_index];
    tr5_room_t *tr_room = &tr->rooms[room_index];

    /*
     * Sectors loading
     */

    RoomSector* sector = room->sectors.data();
    for(uint32_t i=0;i<room->sectors.size();i++,sector++)
    {
        /*
         * Let us fill pointers to sectors above and sectors below
         */

        uint8_t rp = tr_room->sector_list[i].room_below;
        sector->sector_below = NULL;
        if(rp < world->rooms.size() && rp != 255)
        {
            sector->sector_below = world->rooms[rp]->getSectorRaw(sector->pos);
        }
        rp = tr_room->sector_list[i].room_above;
        sector->sector_above = NULL;
        if(rp < world->rooms.size() && rp != 255)
        {
            sector->sector_above = world->rooms[rp]->getSectorRaw(sector->pos);
        }

        RoomSector* near_sector = NULL;

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
            for(const Portal& p : room->portals)
            {
                if((p.normal.normal[2] < 0.01) && ((p.normal.normal[2] > -0.01)))
                {
                    RoomSector* dst = p.dest_room ? p.dest_room->getSectorRaw(sector->pos) : nullptr;
                    RoomSector* orig_dst = engine_world.rooms[sector->portal_to_room]->getSectorRaw(sector->pos);
                    if((dst != NULL) && (dst->portal_to_room < 0) && (dst->floor != TR_METERING_WALLHEIGHT) && (dst->ceiling != TR_METERING_WALLHEIGHT) && ((uint32_t)sector->portal_to_room != p.dest_room->id) && (dst->floor < orig_dst->floor) && TR_IsSectorsIn2SideOfPortal(near_sector, dst, p))
                    {
                        sector->portal_to_room = p.dest_room->id;
                        orig_dst = dst;
                    }
                }
            }
        }
    }
}

void TR_vertex_to_arr(btVector3& v, const tr5_vertex_t& tr_v)
{
    v[0] = tr_v.x;
    v[1] =-tr_v.z;
    v[2] = tr_v.y;
}

void TR_color_to_arr(std::array<GLfloat,4>& v, const tr5_colour_t& tr_c)
{
    v[0] = tr_c.r * 2;
    v[1] = tr_c.g * 2;
    v[2] = tr_c.b * 2;
    v[3] = tr_c.a * 2;
}

RoomSector* TR_GetRoomSector(uint32_t room_id, int sx, int sy)
{
    if(room_id >= engine_world.rooms.size())
    {
        return NULL;
    }

    auto room = engine_world.rooms[room_id];
    if((sx < 0) || (sx >= room->sectors_x) || (sy < 0) || (sy >= room->sectors_y))
    {
        return NULL;
    }

    return &room->sectors[ sx * room->sectors_y + sy ];
}

void lua_SetSectorFloorConfig(int id, int sx, int sy, lua::Value pen, lua::Value diag, lua::Value floor, float z0, float z1, float z2, float z3)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        ConsoleInfo::instance().addLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return;
    }

    if(pen.is<lua::Integer>())   rs->floor_penetration_config = static_cast<int>(pen);
    if(diag.is<lua::Integer>())  rs->floor_diagonal_type = static_cast<int>(diag);
    if(floor.is<lua::Integer>()) rs->floor = floor;
    rs->floor_corners[0] = {z0,z1,z2};
    rs->floor_corners[0][3] = z3;
}

void lua_SetSectorCeilingConfig(int id, int sx, int sy, lua::Value pen, lua::Value diag, lua::Value ceil, float z0, float z1, float z2, float z3)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        ConsoleInfo::instance().addLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return;
    }

    if(pen.is<lua::Integer>())  rs->ceiling_penetration_config = static_cast<int>(pen);
    if(diag.is<lua::Integer>()) rs->ceiling_diagonal_type = static_cast<int>(diag);
    if(ceil.is<lua::Integer>()) rs->ceiling = ceil;

    rs->ceiling_corners[0] = {z0,z1,z2};
    rs->ceiling_corners[0][3] = z3;
}

void lua_SetSectorPortal(int id, int sx, int sy, uint32_t p)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        ConsoleInfo::instance().addLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return;
    }

    if(p < engine_world.rooms.size())
    {
        rs->portal_to_room = p;
    }
}

void lua_SetSectorFlags(int id, int sx, int sy, lua::Value fpflag, lua::Value ftflag, lua::Value cpflag, lua::Value ctflag)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        ConsoleInfo::instance().addLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return;
    }

    if(fpflag.is<lua::Integer>())  rs->floor_penetration_config = static_cast<int>(fpflag);
    if(ftflag.is<lua::Integer>())  rs->floor_diagonal_type = static_cast<int>(ftflag);
    if(cpflag.is<lua::Integer>())  rs->ceiling_penetration_config = static_cast<int>(cpflag);
    if(ctflag.is<lua::Integer>())  rs->ceiling_diagonal_type = static_cast<int>(ctflag);
}

void Res_ScriptsOpen(int engine_version)
{
    std::string temp_script_name = Engine_GetLevelScriptName(engine_version, std::string());

    lua_register(level_script.getState(), "print", lua_print);
    level_script.set("setSectorFloorConfig", lua_SetSectorFloorConfig);
    level_script.set("setSectorCeilingConfig", lua_SetSectorCeilingConfig);
    level_script.set("setSectorPortal", lua_SetSectorPortal);
    level_script.set("setSectorFlags", lua_SetSectorFlags);

    try {
        level_script.doFile("scripts/staticmesh/staticmesh_script.lua");
    }
    catch(lua::RuntimeError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    catch(lua::LoadError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }

    if(Engine_FileFound(temp_script_name, false))
    {
        try {
            level_script.doFile(temp_script_name);
        }
        catch(lua::RuntimeError& error) {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
        catch(lua::LoadError& error) {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
    }


    try {
        objects_flags_conf.doFile("scripts/entity/entity_properties.lua");
    }
    catch(lua::RuntimeError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    catch(lua::LoadError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }

    try {
        ent_ID_override.doFile("scripts/entity/entity_model_ID_override.lua");
    }
    catch(lua::RuntimeError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    catch(lua::LoadError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
}

void Res_ScriptsClose()
{
}

void Res_AutoexecOpen(int engine_version)
{
    std::string temp_script_name = Engine_GetLevelScriptName(engine_version, "_autoexec");

    try {
        engine_lua.doFile("scripts/autoexec.lua");    // do standart autoexec
    }
    catch(lua::RuntimeError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    catch(lua::LoadError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    try {
        engine_lua.doFile(temp_script_name);          // do level-specific autoexec
    }
    catch(lua::RuntimeError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
    catch(lua::LoadError& error) {
        Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
    }
}

void TR_GenWorld(World *world, class VT_Level *tr)
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

    // Generate entity functions.

    Res_GenEntityFunctions(world->entity_tree);
    Gui_DrawLoadScreen(910);

    // Load entity collision flags and ID overrides from script.

    Res_ScriptsClose();
    Gui_DrawLoadScreen(940);

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


void Res_GenRBTrees(World *world)
{
    world->entity_tree.clear();
    world->next_entity_id = 0;
    world->items_tree.clear();
}


void TR_GenRooms(World *world, class VT_Level *tr)
{
    world->rooms.resize(tr->rooms_count);
    std::generate(std::begin(world->rooms), std::end(world->rooms), std::make_shared<Room>);
    for(uint32_t i=0; i<world->rooms.size(); i++)
    {
        TR_GenRoom(i, world->rooms[i], world, tr);
    }
}

void TR_GenRoom(size_t room_index, std::shared_ptr<Room>& room, World *world, class VT_Level *tr)
{
    tr5_room_t *tr_room = &tr->rooms[room_index];
    tr_staticmesh_t *tr_static;
    tr_room_portal_t *tr_portal;
    RoomSector* sector;
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;

    room->id = room_index;
    room->active = true;
    room->frustum.clear();
    room->flags = tr->rooms[room_index].flags;
    room->light_mode = tr->rooms[room_index].light_mode;
    room->reverb_info = tr->rooms[room_index].reverb_info;
    room->water_scheme = tr->rooms[room_index].water_scheme;
    room->alternate_group = tr->rooms[room_index].alternate_group;

    room->transform.setIdentity();
    room->transform.getOrigin()[0] = tr->rooms[room_index].offset.x;                       // x = x;
    room->transform.getOrigin()[1] =-tr->rooms[room_index].offset.z;                       // y =-z;
    room->transform.getOrigin()[2] = tr->rooms[room_index].offset.y;                       // z = y;
    room->ambient_lighting[0] = tr->rooms[room_index].light_colour.r * 2;
    room->ambient_lighting[1] = tr->rooms[room_index].light_colour.g * 2;
    room->ambient_lighting[2] = tr->rooms[room_index].light_colour.b * 2;
    room->self.reset( new EngineContainer() );
    room->self->room = room.get();
    room->self->object = room.get();
    room->self->object_type = OBJECT_ROOM_BASE;
    room->near_room_list.clear();
    room->overlapped_room_list.clear();

    TR_GenRoomMesh(world, room_index, room, tr);

    room->bt_body.reset();
    /*
     *  let us load static room meshes
     */
    room->static_mesh.clear();

    for(uint16_t i=0;i<tr_room->num_static_meshes;i++)
    {
        tr_static = tr->find_staticmesh_id(tr_room->static_meshes[i].object_id);
        if(tr_static == NULL)
        {
            continue;
        }
        room->static_mesh.emplace_back( std::make_shared<StaticMesh>() );
        std::shared_ptr<StaticMesh> r_static = room->static_mesh.back();
        r_static->self = std::make_shared<EngineContainer>();
        r_static->self->room = room.get();
        r_static->self->object = room->static_mesh[i].get();
        r_static->self->object_type = OBJECT_STATIC_MESH;
        r_static->object_id = tr_room->static_meshes[i].object_id;
        r_static->mesh = world->meshes[ tr->mesh_indices[tr_static->mesh] ];
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
        r_static->obb = new OBB();

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

        r_static->obb->transform = &room->static_mesh[i]->transform;
        r_static->obb->r = room->static_mesh[i]->mesh->m_radius;
        r_static->transform.setIdentity();
        Mat4_Translate(r_static->transform, r_static->pos);
        Mat4_RotateZ(r_static->transform, r_static->rot[0]);
        r_static->was_rendered = 0;
        r_static->obb->rebuild(r_static->vbb_min, r_static->vbb_max);
        r_static->obb->doTransform();

        r_static->bt_body = NULL;
        r_static->hide = false;

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
                    cshape = BT_CSfromBBox(r_static->mesh->m_bbMin, r_static->mesh->m_bbMax, true, true);
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
                startTransform = r_static->transform;
                btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                r_static->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
                bt_engine_dynamicsWorld->addRigidBody(r_static->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
                r_static->bt_body->setUserPointer(r_static->self.get());
            }
        }
    }

    /*
     * sprites loading section
     */
    for(uint32_t i=0;i<tr_room->num_sprites;i++)
    {
        room->sprites.emplace_back();
        if((tr_room->sprites[i].texture >= 0) && ((uint32_t)tr_room->sprites[i].texture < world->sprites.size()))
        {
            room->sprites[i].sprite = &world->sprites[tr_room->sprites[i].texture];
            TR_vertex_to_arr(room->sprites[i].pos, tr_room->vertices[tr_room->sprites[i].vertex].vertex);
            room->sprites[i].pos += room->transform.getOrigin();
        }
    }

    /*
     * let us load sectors
     */
    room->sectors_x = tr_room->num_xsectors;
    room->sectors_y = tr_room->num_zsectors;
    room->sectors.resize(room->sectors_x * room->sectors_y);

    /*
     * base sectors information loading and collisional mesh creation
     */

    // To avoid manipulating with unnecessary information, we declare simple
    // heightmap here, which will be operated with sector and floordata parsing,
    // then vertical inbetween polys will be constructed, and Bullet collisional
    // object will be created. Afterwards, this heightmap also can be used to
    // quickly detect slopes for pushable blocks and other entities that rely on
    // floor level.

    sector = room->sectors.data();
    for(uint32_t i=0;i<room->sectors.size();i++,sector++)
    {
        // Filling base sectors information.

        sector->index_x = i / room->sectors_y;
        sector->index_y = i % room->sectors_y;

        sector->pos[0] = room->transform.getOrigin()[0] + sector->index_x * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
        sector->pos[1] = room->transform.getOrigin()[1] + sector->index_y * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
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

        room->sectors[i].ceiling_corners[0][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0][2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[1][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1][2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[2][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2][2] = (btScalar)sector->ceiling;

        room->sectors[i].ceiling_corners[3][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3][2] = (btScalar)sector->ceiling;

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

        room->sectors[i].floor_corners[0][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0][2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[1][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1][2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[2][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2][2] = (btScalar)sector->floor;

        room->sectors[i].floor_corners[3][0] = (btScalar)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3][1] = (btScalar)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3][2] = (btScalar)sector->floor;
    }

    /*
     *  load lights
     */
    room->lights.resize( tr_room->num_lights );

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
    room->portals.resize(tr_room->num_portals);
    tr_portal = tr_room->portals;
    for(size_t i=0; i<room->portals.size(); i++, tr_portal++)
    {
        Portal* p = &room->portals[i];
        std::shared_ptr<Room> r_dest = world->rooms[tr_portal->adjoining_room];
        p->vertices.resize(4); // in original TR all portals are axis aligned rectangles
        p->flag = 0;
        p->dest_room = r_dest;
        p->current_room = room;
        TR_vertex_to_arr(p->vertices[0], tr_portal->vertices[3]);
        p->vertices[0] += room->transform.getOrigin();
        TR_vertex_to_arr(p->vertices[1], tr_portal->vertices[2]);
        p->vertices[1] += room->transform.getOrigin();
        TR_vertex_to_arr(p->vertices[2], tr_portal->vertices[1]);
        p->vertices[2] += room->transform.getOrigin();
        TR_vertex_to_arr(p->vertices[3], tr_portal->vertices[0]);
        p->vertices[3] += room->transform.getOrigin();
        p->centre = std::accumulate(p->vertices.begin(), p->vertices.end(), btVector3(0, 0, 0)) / 4;
        p->genNormale();

        /*
         * Portal position fix...
         */
        // X_MIN
        if((p->normal.normal[0] > 0.999) && (((int)p->centre[0])%2))
        {
            p->move({1,0,0});
        }

        // Y_MIN
        if((p->normal.normal[1] > 0.999) && (((int)p->centre[1])%2))
        {
            p->move({0,1,0});
        }

        // Z_MAX
        if((p->normal.normal[2] <-0.999) && (((int)p->centre[2])%2))
        {
            p->move({0,0,-1});
        }
    }

    /*
     * room borders calculation
     */
    room->bb_min[2] = tr_room->y_bottom;
    room->bb_max[2] = tr_room->y_top;

    room->bb_min[0] = room->transform.getOrigin()[0] + TR_METERING_SECTORSIZE;
    room->bb_min[1] = room->transform.getOrigin()[1] + TR_METERING_SECTORSIZE;
    room->bb_max[0] = room->transform.getOrigin()[0] + TR_METERING_SECTORSIZE * room->sectors_x - TR_METERING_SECTORSIZE;
    room->bb_max[1] = room->transform.getOrigin()[1] + TR_METERING_SECTORSIZE * room->sectors_y - TR_METERING_SECTORSIZE;

    /*
     * alternate room pointer calculation if one exists.
     */
    room->alternate_room = NULL;
    room->base_room = NULL;

    if((tr_room->alternate_room >= 0) && ((uint32_t)tr_room->alternate_room < tr->rooms_count))
    {
        room->alternate_room = world->rooms[ tr_room->alternate_room ];
    }
}


void Res_GenRoomCollision(World *world)
{
    /*
    if(level_script != NULL)
    {
        lua_CallVoidFunc(level_script, "doTuneSector");
    }
    */

    for(uint32_t i=0; i<world->rooms.size(); i++)
    {
        auto r = world->rooms[i];
        // Inbetween polygons array is later filled by loop which scans adjacent
        // sector heightmaps and fills the gaps between them, thus creating inbetween
        // polygon. Inbetweens can be either quad (if all four corner heights are
        // different), triangle (if one corner height is similar to adjacent) or
        // ghost (if corner heights are completely similar). In case of quad inbetween,
        // two triangles are added to collisional trimesh, in case of triangle inbetween,
        // we add only one, and in case of ghost inbetween, we ignore it.

        int num_heightmaps = (r->sectors_x * r->sectors_y);
        int num_tweens = (num_heightmaps * 4);
        SectorTween *room_tween   = new SectorTween[num_tweens];

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
            btDefaultMotionState* motionState = new btDefaultMotionState(r->transform);
            r->bt_body.reset( new btRigidBody(0.0, motionState, cshape, localInertia) );
            bt_engine_dynamicsWorld->addRigidBody(r->bt_body.get(), COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
            r->bt_body->setUserPointer(r->self.get());
            r->bt_body->setRestitution(1.0);
            r->bt_body->setFriction(1.0);
            r->self->collision_type = COLLISION_TYPE_STATIC;                    // meshtree
            r->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        }

        delete[] room_tween;
    }
}


void TR_GenRoomProperties(World *world, class VT_Level *tr)
{
    for(uint32_t i=0;i<world->rooms.size();i++)
    {
        std::shared_ptr<Room> r = world->rooms[i];
        if(r->alternate_room != NULL)
        {
            r->alternate_room->base_room = r;   // Refill base room pointer.
        }

        // Fill heightmap and translate floordata.
        for(RoomSector& sector : r->sectors)
        {
            TR_Sector_TranslateFloorData(&sector, tr);
            Res_Sector_FixHeights(&sector);
        }

        // Generate links to the near rooms.
        r->buildNearRoomsList();
        // Generate links to the overlapped rooms.
        r->buildOverlappedRoomsList();

        // Basic sector calculations.
        TR_Sector_Calculate(world, tr, i);
    }
}


void Res_GenRoomFlipMap(World *world)
{
    // Flipmap count is hardcoded, as no original levels contain such info.
    world->flip_data.resize(FLIPMAP_MAX_NUMBER);
}


void TR_GenBoxes(World *world, class VT_Level *tr)
{
    world->room_boxes.clear();

    for(uint32_t i=0;i<tr->boxes_count;i++)
    {
        world->room_boxes.emplace_back();
        auto& room = world->room_boxes.back();
        room.overlap_index = tr->boxes[i].overlap_index;
        room.true_floor =-tr->boxes[i].true_floor;
        room.x_min = tr->boxes[i].xmin;
        room.x_max = tr->boxes[i].xmax;
        room.y_min =-tr->boxes[i].zmax;
        room.y_max =-tr->boxes[i].zmin;
    }
}

void TR_GenCameras(World *world, class VT_Level *tr)
{
    world->cameras_sinks.clear();

    for(uint32_t i=0; i<tr->cameras_count; i++) {
        world->cameras_sinks.emplace_back();
        world->cameras_sinks[i].x                   =  tr->cameras[i].x;
        world->cameras_sinks[i].y                   =  tr->cameras[i].z;
        world->cameras_sinks[i].z                   = -tr->cameras[i].y;
        world->cameras_sinks[i].room_or_strength    =  tr->cameras[i].room;
        world->cameras_sinks[i].flag_or_zone        =  tr->cameras[i].unknown1;
    }
}

/**
 * sprites loading, works correct in TR1 - TR5
 */
void TR_GenSprites(World *world, class VT_Level *tr)
{
    if(tr->sprite_textures.empty())
    {
        world->sprites.clear();
        return;
    }


    for(size_t i=0; i<tr->sprite_textures.size(); i++)
    {
        world->sprites.emplace_back();
        auto s = &world->sprites.back();

        auto tr_st = &tr->sprite_textures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        world->tex_atlas->getSpriteCoordinates(i, s->texture, s->tex_coord);
    }

    for(uint32_t i=0;i<tr->sprite_sequences_count;i++)
    {
        if((tr->sprite_sequences[i].offset >= 0) && ((uint32_t)tr->sprite_sequences[i].offset < world->sprites.size()))
        {
            world->sprites[tr->sprite_sequences[i].offset].id = tr->sprite_sequences[i].object_id;
        }
    }
}

void Res_GenSpritesBuffer(World *world)
{
    for (uint32_t i = 0; i < world->rooms.size(); i++)
        Res_GenRoomSpritesBuffer(world->rooms[i]);
}

void TR_GenTextures(World* world, class VT_Level *tr)
{
    int border_size = renderer.settings().texture_border;
    border_size = (border_size < 0)?(0):(border_size);
    border_size = (border_size > 128)?(128):(border_size);
    world->tex_atlas.reset( new BorderedTextureAtlas(border_size,
                                                     renderer.settings().save_texture_memory,
                                                     tr->textile32,
                                                     tr->object_textures,
                                                     tr->sprite_textures) );

    world->textures.resize(world->tex_atlas->getNumAtlasPages() + 1);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(1, 1);
    world->tex_atlas->createTextures(world->textures.data(), 1);

    // white texture data for coloured polygons and debug lines.
    GLubyte whtx[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // Select mipmap mode
    switch(renderer.settings().mipmap_mode)
    {
        case 0:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;

        case 1:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;

        case 2:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;

        case 3:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
    };

    // Set mipmaps number
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, renderer.settings().mipmaps);

    // Set anisotropy degree
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, renderer.settings().anisotropy);

    // Read lod bias
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, renderer.settings().lod_bias);


    glBindTexture(GL_TEXTURE_2D, world->textures.back());          // solid color =)
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
void TR_GenAnimTextures(World *world, class VT_Level *tr)
{
    uint16_t *pointer;
    uint16_t  num_sequences, num_uvrotates;
    int32_t   uvrotate_script = 0;
    struct Polygon p0, p;

    p0.vertices.resize(3);
    p.vertices.resize(3);

    pointer       = tr->animated_textures;
    num_uvrotates = tr->animated_textures_uv_count;

    num_sequences = *(pointer++);   // First word in a stream is sequence count.

    world->anim_sequences.resize(num_sequences);

    AnimSeq* seq = world->anim_sequences.data();
    for(uint16_t i = 0; i < num_sequences; i++,seq++)
    {
        seq->frames.resize(*(pointer++) + 1);
        seq->frame_list.resize(seq->frames.size());

        // Fill up new sequence with frame list.
        seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
        seq->frame_lock        = false; // by default anim is playing
        seq->uvrotate          = false; // by default uvrotate
        seq->reverse_direction = false; // Needed for proper reverse-type start-up.
        seq->frame_rate        = 0.05;  // Should be passed as 1 / FPS.
        seq->frame_time        = 0.0;   // Reset frame time to initial state.
        seq->current_frame     = 0;     // Reset current frame to zero.

        for(uint16_t j = 0; j < seq->frames.size(); j++)
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

        level_script["UVRotate"].get(uvrotate_script);

        if(i < num_uvrotates)
        {
            seq->frame_lock        = false; // by default anim is playing

            seq->uvrotate = true;
            // Get texture height and divide it in half.
            // This way, we get a reference value which is used to identify
            // if scrolling is completed or not.
            seq->frames.resize(8);
            seq->uvrotate_max   = world->tex_atlas->getTextureHeight(seq->frame_list[0]) / 2;
            seq->uvrotate_speed = seq->uvrotate_max / (btScalar)seq->frames.size();
            seq->frame_list.resize(8);

            if(uvrotate_script > 0)
            {
                seq->anim_type        = TR_ANIMTEXTURE_FORWARD;
            }
            else if(uvrotate_script < 0)
            {
                seq->anim_type        = TR_ANIMTEXTURE_BACKWARD;
            }

            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0.0, true);
            for(uint16_t j=0;j<seq->frames.size();j++)
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
            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p0);
            for(uint16_t j=0;j<seq->frames.size();j++)
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
}

/**   Assign animated texture to a polygon.
  *   While in original TRs we had TexInfo abstraction layer to refer texture,
  *   in OpenTomb we need to re-think animated texture concept to work on a
  *   per-polygon basis. For this, we scan all animated texture lists for
  *   same TexInfo index that is applied to polygon, and if corresponding
  *   animation list is found, we assign it to polygon.
  */
bool SetAnimTexture(struct Polygon *polygon, uint32_t tex_index, struct World *world)
{
    polygon->anim_id = 0;                           // Reset to 0 by default.

    for(uint32_t i = 0; i < world->anim_sequences.size(); i++)
    {
        for(uint16_t j = 0; j < world->anim_sequences[i].frames.size(); j++)
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

void TR_GenMeshes(World *world, class VT_Level *tr)
{
    world->meshes.resize( tr->meshes_count );
    size_t i=0;
    for(std::shared_ptr<BaseMesh>& baseMesh : world->meshes) {
        baseMesh = std::make_shared<BaseMesh>();
        TR_GenMesh(world, i++, baseMesh, tr);
    }
}

static void tr_copyNormals(struct Polygon *polygon, const std::shared_ptr<BaseMesh>& mesh, const uint16_t *mesh_vertex_indices)
{
    for (size_t i=0; i<polygon->vertices.size(); ++i)
    {
        polygon->vertices[i].normal = mesh->m_vertices[mesh_vertex_indices[i]].normal;
    }
}

void tr_accumulateNormals(tr4_mesh_t *tr_mesh, BaseMesh* mesh, int numCorners, const uint16_t *vertex_indices, struct Polygon *p)
{
    p->vertices.resize(numCorners);

    for (int i = 0; i < numCorners; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, tr_mesh->vertices[vertex_indices[i]]);
    }
    p->findNormal();

    for (int i = 0; i < numCorners; i++)
    {
        mesh->m_vertices[vertex_indices[i]].normal += p->plane.normal;
    }
}

void tr_setupColoredFace(tr4_mesh_t *tr_mesh, VT_Level *tr, BaseMesh* mesh, const uint16_t *vertex_indices, unsigned color, struct Polygon *p)
{
    for (size_t i = 0; i < p->vertices.size(); i++)
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
    mesh->m_usesVertexColors = true;
}

void tr_setupTexturedFace(tr4_mesh_t *tr_mesh, BaseMesh* mesh, const uint16_t *vertex_indices, struct Polygon *p)
{
    for (size_t i = 0; i < p->vertices.size(); i++)
    {
        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[i].color[0] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[1] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[2] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[3] = 1.0f;

            mesh->m_usesVertexColors = true;
        }
        else
        {
            p->vertices[i].color.fill(1);
        }
    }
}

void TR_GenMesh(World *world, size_t mesh_index, std::shared_ptr<BaseMesh> mesh, class VT_Level *tr)
{
    const uint32_t tex_mask = (world->version == TR_IV)?(TR_TEXTURE_INDEX_MASK_TR4):(TR_TEXTURE_INDEX_MASK);

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

    tr4_mesh_t* tr_mesh = &tr->meshes[mesh_index];
    mesh->m_id = mesh_index;
    mesh->m_center[0] = tr_mesh->centre.x;
    mesh->m_center[1] =-tr_mesh->centre.z;
    mesh->m_center[2] = tr_mesh->centre.y;
    mesh->m_radius = tr_mesh->collision_size;
    mesh->m_texturePageCount = (uint32_t)world->tex_atlas->getNumAtlasPages() + 1;

    mesh->m_vertices.resize( tr_mesh->num_vertices );
    auto vertex = mesh->m_vertices.data();
    for(size_t i=0; i<mesh->m_vertices.size(); i++, vertex++)
    {
        TR_vertex_to_arr(vertex->position, tr_mesh->vertices[i]);
        vertex->normal.setZero();                                          // paranoid
    }

    mesh->findBB();

    mesh->m_polygons.clear();

    /*
     * textured triangles
     */
    for(int i=0; i<tr_mesh->num_textured_triangles; ++i)
    {
        mesh->m_polygons.emplace_back();
        struct Polygon &p = mesh->m_polygons.back();

        auto face3 = &tr_mesh->textured_triangles[i];
        auto tex = &tr->object_textures[face3->texture & tex_mask];

        p.double_side = (bool)(face3->texture >> 15);    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(&p, face3->texture & tex_mask, world);

        if(face3->lighting & 0x01)
        {
            p.transparency = BM_MULTIPLY;
        }
        else
        {
            p.transparency = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, mesh.get(), 3, face3->vertices, &p);
        tr_setupTexturedFace(tr_mesh, mesh.get(), face3->vertices, &p);

        world->tex_atlas->getCoordinates(face3->texture & tex_mask, 0, &p);
    }

    /*
     * coloured triangles
     */
    for(int i=0; i<tr_mesh->num_coloured_triangles; ++i)
    {
        mesh->m_polygons.emplace_back();
        struct Polygon &p = mesh->m_polygons.back();

        auto face3 = &tr_mesh->coloured_triangles[i];
        auto col = face3->texture & 0xff;
        p.tex_index = (uint32_t)world->tex_atlas->getNumAtlasPages();
        p.transparency = 0;
        p.anim_id = 0;

        tr_accumulateNormals(tr_mesh, mesh.get(), 3, face3->vertices, &p);
        tr_setupColoredFace(tr_mesh, tr, mesh.get(), face3->vertices, col, &p);
    }

    /*
     * textured rectangles
     */
    for(int i=0; i<tr_mesh->num_textured_rectangles; ++i)
    {
        mesh->m_polygons.emplace_back();
        struct Polygon &p = mesh->m_polygons.back();

        auto face4 = &tr_mesh->textured_rectangles[i];
        auto tex = &tr->object_textures[face4->texture & tex_mask];

        p.double_side = (bool)(face4->texture >> 15);    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(&p, face4->texture & tex_mask, world);

        if(face4->lighting & 0x01)
        {
            p.transparency = BM_MULTIPLY;
        }
        else
        {
            p.transparency = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, mesh.get(), 4, face4->vertices, &p);
        tr_setupTexturedFace(tr_mesh, mesh.get(), face4->vertices, &p);

        world->tex_atlas->getCoordinates(face4->texture & tex_mask, 0, &p);
    }

    /*
     * coloured rectangles
     */
    for(int16_t i=0;i<tr_mesh->num_coloured_rectangles;i++)
    {
        mesh->m_polygons.emplace_back();
        struct Polygon &p = mesh->m_polygons.back();

        auto face4 = &tr_mesh->coloured_rectangles[i];
        auto col = face4->texture & 0xff;
        p.vertices.resize(4);
        p.tex_index = (uint32_t)world->tex_atlas->getNumAtlasPages();
        p.transparency = 0;
        p.anim_id = 0;

        tr_accumulateNormals(tr_mesh, mesh.get(), 4, face4->vertices, &p);
        tr_setupColoredFace(tr_mesh, tr, mesh.get(), face4->vertices, col, &p);
    }

    /*
     * let us normalise normales %)
     */
    for(Vertex& v : mesh->m_vertices) {
        v.normal.normalize();
    }

    /*
     * triangles
     */
    auto p = mesh->m_polygons.begin();
    for(int16_t i=0;i<tr_mesh->num_textured_triangles;i++,p++)
    {
        tr_copyNormals(&*p, mesh, tr_mesh->textured_triangles[i].vertices);
    }

    for(int16_t i=0;i<tr_mesh->num_coloured_triangles;i++,p++)
    {
        tr_copyNormals(&*p, mesh, tr_mesh->coloured_triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(int16_t i=0;i<tr_mesh->num_textured_rectangles;i++,p++)
    {
        tr_copyNormals(&*p, mesh, tr_mesh->textured_rectangles[i].vertices);
    }

    for(int16_t i=0;i<tr_mesh->num_coloured_rectangles;i++,p++)
    {
        tr_copyNormals(&*p, mesh, tr_mesh->coloured_rectangles[i].vertices);
    }

    mesh->m_vertices.clear();
    mesh->genFaces();
    mesh->polySortInMesh();
}

void tr_setupRoomVertices(World *world, VT_Level *tr, tr5_room_t *tr_room, const std::shared_ptr<BaseMesh>& mesh, int numCorners, const uint16_t *vertices, uint16_t masked_texture, struct Polygon *p)
{
    p->vertices.resize(numCorners);

    for (int i = 0; i < numCorners; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, tr_room->vertices[vertices[i]].vertex);
    }
    p->findNormal();

    for (int i = 0; i < numCorners; i++)
    {
        mesh->m_vertices[vertices[i]].normal += p->plane.normal;
        p->vertices[i].normal = p->plane.normal;
        TR_color_to_arr(p->vertices[i].color, tr_room->vertices[vertices[i]].colour);
    }

    tr4_object_texture_t *tex = &tr->object_textures[masked_texture];
    SetAnimTexture(p, masked_texture, world);
    p->transparency = tex->transparency_flags;

    world->tex_atlas->getCoordinates(masked_texture, 0, p);

}

void TR_GenRoomMesh(World *world, size_t room_index, std::shared_ptr<Room> room, class VT_Level *tr)
{
    const uint32_t tex_mask = (world->version == TR_IV)?(TR_TEXTURE_INDEX_MASK_TR4):(TR_TEXTURE_INDEX_MASK);

    auto tr_room = &tr->rooms[room_index];

    if(tr_room->num_triangles + tr_room->num_rectangles == 0)
    {
        room->mesh = NULL;
        return;
    }

    room->mesh = std::make_shared<BaseMesh>();
    room->mesh->m_id = room_index;
    room->mesh->m_texturePageCount = (uint32_t)world->tex_atlas->getNumAtlasPages() + 1;
    room->mesh->m_usesVertexColors = true; // This is implicitly true on room meshes

    room->mesh->m_vertices.resize( tr_room->num_vertices );
    auto vertex = room->mesh->m_vertices.data();
    for(size_t i=0; i<room->mesh->m_vertices.size(); i++, vertex++)
    {
        TR_vertex_to_arr(vertex->position, tr_room->vertices[i].vertex);
        vertex->normal.setZero();                                          // paranoid
    }

    room->mesh->findBB();

    room->mesh->m_polygons.resize( tr_room->num_triangles + tr_room->num_rectangles );
    auto p = room->mesh->m_polygons.begin();

    /*
     * triangles
     */
    for(uint32_t i=0;i<tr_room->num_triangles;i++,p++)
    {
        tr_setupRoomVertices(world, tr, tr_room, room->mesh, 3, tr_room->triangles[i].vertices, tr_room->triangles[i].texture & tex_mask, &*p);
        p->double_side = tr_room->triangles[i].texture & 0x8000;
    }

    /*
     * rectangles
     */
    for(uint32_t i=0;i<tr_room->num_rectangles;i++,p++)
    {
        tr_setupRoomVertices(world, tr, tr_room, room->mesh, 4, tr_room->rectangles[i].vertices, tr_room->rectangles[i].texture & tex_mask, &*p);
        p->double_side = tr_room->rectangles[i].texture & 0x8000;
    }

    /*
     * let us normalise normales %)
     */
    for(Vertex& v : room->mesh->m_vertices)
    {
        v.normal.normalize();
    }

    /*
     * triangles
     */
    p = room->mesh->m_polygons.begin();
    for(size_t i=0; i<tr_room->num_triangles; i++, p++)
    {
        tr_copyNormals(&*p, room->mesh, tr_room->triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(uint32_t i=0;i<tr_room->num_rectangles;i++,p++)
    {
        tr_copyNormals(&*p, room->mesh, tr_room->rectangles[i].vertices);
    }

    room->mesh->m_vertices.clear();
    room->mesh->genFaces();
    room->mesh->polySortInMesh();
}

void Res_GenRoomSpritesBuffer(std::shared_ptr<Room> room)
{
    // Find the number of different texture pages used and the number of non-null sprites
    uint32_t highestTexturePageFound = 0;
    int actualSpritesFound = 0;
    for(RoomSprite& sp : room->sprites)
    {
        if (sp.sprite)
        {
            actualSpritesFound += 1;
            highestTexturePageFound = std::max(highestTexturePageFound, sp.sprite->texture);
        }
    }
    if (actualSpritesFound == 0)
    {
        room->sprite_buffer = 0;
        return;
    }

    room->sprite_buffer = (SpriteBuffer *) calloc(sizeof(SpriteBuffer), 1);
    room->sprite_buffer->num_texture_pages = highestTexturePageFound + 1;
    room->sprite_buffer->element_count_per_texture = (uint32_t *) calloc(sizeof(uint32_t), room->sprite_buffer->num_texture_pages);

    // First collect indices on a per-texture basis
    uint16_t **elements_for_texture = (uint16_t **)calloc(sizeof(uint16_t*), highestTexturePageFound + 1);

    GLfloat *spriteData = (GLfloat *) calloc(sizeof(GLfloat [7]), actualSpritesFound * 4);

    int writeIndex = 0;
    for (const RoomSprite& room_sprite : room->sprites)
    {
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
    glGenBuffers(1, &arrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat [7]) * 4 * actualSpritesFound, spriteData, GL_STATIC_DRAW);
    free(spriteData);

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * elementsSoFar, elements, GL_STATIC_DRAW);
    free(elements);

    VertexArrayAttribute attribs[3] = {
        VertexArrayAttribute(SpriteShaderDescription::vertex_attribs::position, 3, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [0])),
        VertexArrayAttribute(SpriteShaderDescription::vertex_attribs::tex_coord, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [3])),
        VertexArrayAttribute(SpriteShaderDescription::vertex_attribs::corner_offset, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat [7]), sizeof(GLfloat [5]))
    };

    room->sprite_buffer->data.reset( new VertexArray(elementBuffer, 3, attribs) );
}

void Res_GenVBOs(World *world)
{
    for(uint32_t i=0; i<world->meshes.size(); i++)
    {
        if(!world->meshes[i]->m_vertices.empty() || !world->meshes[i]->m_animatedVertices.empty())
        {
            world->meshes[i]->genVBO(&renderer);
        }
    }

    for(uint32_t i=0; i<world->rooms.size(); i++)
    {
        if(world->rooms[i]->mesh && (!world->rooms[i]->mesh->m_vertices.empty() || !world->rooms[i]->mesh->m_animatedVertices.empty()))
        {
            world->rooms[i]->mesh->genVBO(&renderer);
        }
    }
}

void Res_GenBaseItems(World* world)
{
    engine_lua["genBaseItems"]();

    if(!world->items_tree.empty())
    {
        Res_EntityToItem(world->items_tree);
    }
}

void Res_FixRooms(World *world)
{
    for(uint32_t i=0;i<world->rooms.size();i++)
    {
        auto r = world->rooms[i];
        if(r->base_room != NULL)
        {
            r->disable();    // Disable current room
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

SkeletalModel* Res_GetSkybox(World *world, uint32_t engine_version)
{
    switch(engine_version)
    {
        case TR_II:
        case TR_II_DEMO:
            return world->getModelByID(TR_ITEM_SKYBOX_TR2);

        case TR_III:
            return world->getModelByID(TR_ITEM_SKYBOX_TR3);

        case TR_IV:
        case TR_IV_DEMO:
            return world->getModelByID(TR_ITEM_SKYBOX_TR4);

        case TR_V:
            return world->getModelByID(TR_ITEM_SKYBOX_TR5);

        default:
            return NULL;
    }
}

void TR_GenAnimCommands(World *world, class VT_Level *tr)
{
    world->anim_commands.assign( tr->anim_commands+0, tr->anim_commands+tr->anim_commands_count );
    free(tr->anim_commands);
    tr->anim_commands = nullptr;
    tr->anim_commands_count = 0;
}

void TR_GenSkeletalModel(World *world, size_t model_num, SkeletalModel *model, class VT_Level *tr)
{
    tr_moveable_t *tr_moveable;
    tr_animation_t *tr_animation;

    uint32_t frame_offset, frame_step;
    uint16_t temp1, temp2;
    float ang;

    BoneTag* bone_tag;
    BoneFrame* bone_frame;
    MeshTreeTag* tree_tag;
    AnimationFrame* anim;

    tr_moveable = &tr->moveables[model_num];                                    // original tr structure
    model->collision_map.resize(model->mesh_count);
    for(uint16_t i=0;i<model->mesh_count;i++)
    {
        model->collision_map[i] = i;
    }

    model->mesh_tree.resize(model->mesh_count);
    tree_tag = model->mesh_tree.data();

    uint32_t *mesh_index = tr->mesh_indices + tr_moveable->starting_mesh;

    for(uint16_t k=0;k<model->mesh_count;k++,tree_tag++)
    {
        tree_tag->mesh_base = world->meshes[mesh_index[k]];
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

    if(tr_moveable->animation_index >= tr->animations_count)
    {
        /*
         * model has no start offset and any animation
         */
        model->animations.resize(1);
        model->animations.front().frames.resize(1);
        bone_frame = model->animations.front().frames.data();

        model->animations.front().id = 0;
        model->animations.front().next_anim = NULL;
        model->animations.front().next_frame = 0;
        model->animations.front().state_change.clear();
        model->animations.front().original_frame_rate = 1;

        bone_frame->bone_tags.resize( model->mesh_count );

        bone_frame->pos.setZero();
        bone_frame->move.setZero();
        bone_frame->v_Horizontal = 0.0;
        bone_frame->v_Vertical = 0.0;
        bone_frame->command = 0x00;
        for(uint16_t k=0;k<bone_frame->bone_tags.size();k++)
        {
            tree_tag = &model->mesh_tree[k];
            bone_tag = &bone_frame->bone_tags[k];

            vec4_SetTRRotations(bone_tag->qrotate, {0,0,0});
            bone_tag->offset = tree_tag->offset;
        }
        return;
    }
    //Sys_DebugLog(LOG_FILENAME, "model = %d, anims = %d", tr_moveable->object_id, GetNumAnimationsForMoveable(tr, model_num));
    model->animations.resize( TR_GetNumAnimationsForMoveable(tr, model_num) );
    if(model->animations.empty())
    {
        /*
         * the animation count must be >= 1
         */
        model->animations.resize(1);
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
    anim = model->animations.data();
    for(uint16_t i=0;i<model->animations.size();i++,anim++)
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

        anim->frames.resize( TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index+i) );

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.

        if( (anim->num_anim_commands > 0) && (anim->num_anim_commands <= 255) )
        {
            // Calculate current animation anim command block offset.
            assert( anim->anim_command < world->anim_commands.size() );
            int16_t *pointer = &world->anim_commands[ anim->anim_command ];

            for(uint32_t count = 0; count < anim->num_anim_commands; count++)
            {
                const auto command = *pointer;
                ++pointer;
                switch(command)
                {
                    case TR_ANIMCOMMAND_PLAYEFFECT:
                    case TR_ANIMCOMMAND_PLAYSOUND:
                        // Recalculate absolute frame number to relative.
                        pointer[0] -= tr_animation->frame_start;
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


        if(anim->frames.empty())
        {
            /*
             * number of animations must be >= 1, because frame contains base model offset
             */
            anim->frames.resize(1);
        }

        /*
         * let us begin to load animations
         */
        bone_frame = anim->frames.data();
        for(uint16_t j=0;j<anim->frames.size();j++,bone_frame++,frame_offset+=frame_step)
        {
            bone_frame->bone_tags.resize( model->mesh_count );
            bone_frame->pos.setZero();
            bone_frame->move.setZero();
            TR_GetBFrameBB_Pos(tr, frame_offset, bone_frame);

            if(frame_offset >= tr->frame_data_size)
            {
                //Con_Printf("Bad frame offset");
                for(uint16_t k=0;k<bone_frame->bone_tags.size();k++)
                {
                    tree_tag = &model->mesh_tree[k];
                    bone_tag = &bone_frame->bone_tags[k];
                    vec4_SetTRRotations(bone_tag->qrotate, {0,0,0});
                    bone_tag->offset = tree_tag->offset;
                }
            }
            else
            {
                uint16_t l = l_start;
                for(uint16_t k=0;k<bone_frame->bone_tags.size();k++)
                {
                    tree_tag = &model->mesh_tree[k];
                    bone_tag = &bone_frame->bone_tags[k];
                    vec4_SetTRRotations(bone_tag->qrotate, {0,0,0});
                    bone_tag->offset = tree_tag->offset;

                    switch(tr->game_version)
                    {
                        case TR_I:                                              /* TR_I */
                        case TR_I_UB:
                        case TR_I_DEMO: {
                            temp2 = tr->frame_data[frame_offset + l];
                            l ++;
                            temp1 = tr->frame_data[frame_offset + l];
                            l ++;
                            btVector3 rot;
                            rot[0] = (float)((temp1 & 0x3ff0) >> 4);
                            rot[2] =-(float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                            rot[1] = (float)(temp2 & 0x03ff);
                            rot *= 360.0 / 1024.0;
                            vec4_SetTRRotations(bone_tag->qrotate, rot);
                            break;
                        }

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
                                    vec4_SetTRRotations(bone_tag->qrotate, {ang,0,0});
                                    break;

                                case 0x8000:    // y only
                                    vec4_SetTRRotations(bone_tag->qrotate, {0,0,-ang});
                                    break;

                                case 0xc000:    // z only
                                    vec4_SetTRRotations(bone_tag->qrotate, {0,ang,0});
                                    break;

                                default: {        // all three
                                    temp2 = tr->frame_data[frame_offset + l];
                                    btVector3 rot;
                                    rot[0] = (float)((temp1 & 0x3ff0) >> 4);
                                    rot[2] =-(float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                                    rot[1] = (float)(temp2 & 0x03ff);
                                    rot[0] *= 360.0 / 1024.0;
                                    rot[1] *= 360.0 / 1024.0;
                                    rot[2] *= 360.0 / 1024.0;
                                    vec4_SetTRRotations(bone_tag->qrotate, rot);
                                    l ++;
                                    break;
                                }
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
    model->interpolateFrames();
    /*
     * state change's loading
     */

#if LOG_ANIM_DISPATCHES
    if(model->animations.size() > 1)
    {
        Sys_DebugLog(LOG_FILENAME, "MODEL[%d], anims = %d", model_num, model->animations.size());
    }
#endif
    anim = model->animations.data();
    for(uint16_t i=0;i<model->animations.size();i++,anim++)
    {
        anim->state_change.clear();

        tr_animation = &tr->animations[tr_moveable->animation_index+i];
        int16_t j = tr_animation->next_animation - tr_moveable->animation_index;
        j &= 0x7fff; // this masks out the sign bit
        assert( j >= 0 );
        if(static_cast<size_t>(j) < model->animations.size())
        {
            anim->next_anim = &model->animations[j];
            anim->next_frame = tr_animation->next_frame - tr->animations[tr_animation->next_animation].frame_start;
            anim->next_frame %= anim->next_anim->frames.size();
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

        anim->state_change.clear();

        if((tr_animation->num_state_changes > 0) && (model->animations.size() > 1))
        {
            StateChange* sch_p;
#if LOG_ANIM_DISPATCHES
            Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, (anim->next_anim)?(anim->next_anim->id):(-1), anim->next_frame);
#endif
            anim->state_change.resize( tr_animation->num_state_changes );
            sch_p = anim->state_change.data();

            for(uint16_t j=0;j<tr_animation->num_state_changes;j++,sch_p++)
            {
                tr_state_change_t *tr_sch;
                tr_sch = &tr->state_changes[j+tr_animation->state_change_offset];
                sch_p->id = tr_sch->state_id;
                sch_p->anim_dispatch.clear();
                for(uint16_t l=0;l<tr_sch->num_anim_dispatches;l++)
                {
                    tr_anim_dispatch_t *tr_adisp = &tr->anim_dispatches[tr_sch->anim_dispatch+l];
                    uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                    if(next_anim_ind < model->animations.size())
                    {
                        sch_p->anim_dispatch.emplace_back();

                        AnimDispatch* adsp = &sch_p->anim_dispatch.back();
                        uint16_t next_frames_count = model->animations[next_anim - tr_moveable->animation_index].frames.size();
                        uint16_t next_frame = tr_adisp->next_frame - tr->animations[next_anim].frame_start;

                        uint16_t low  = tr_adisp->low  - tr_animation->frame_start;
                        uint16_t high = tr_adisp->high - tr_animation->frame_start;

                        adsp->frame_low  = low  % anim->frames.size();
                        adsp->frame_high = (high - 1) % anim->frames.size();
                        adsp->next_anim = next_anim - tr_moveable->animation_index;
                        adsp->next_frame = next_frame % next_frames_count;

#if LOG_ANIM_DISPATCHES
                        Sys_DebugLog(LOG_FILENAME, "anim_disp[%d], frames.size() = %d: interval[%d.. %d], next_anim = %d, next_frame = %d", l,
                                    anim->frames.size(), adsp->frame_low, adsp->frame_high,
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

void TR_GetBFrameBB_Pos(class VT_Level *tr, size_t frame_offset, BoneFrame *bone_frame)
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

void TR_GenSkeletalModels(World *world, class VT_Level *tr)
{
    world->skeletal_models.resize(tr->moveables_count);

    for(uint32_t i=0;i<tr->moveables_count;i++)
    {
        auto tr_moveable = &tr->moveables[i];
        auto smodel = &world->skeletal_models[i];
        smodel->id = tr_moveable->object_id;
        smodel->mesh_count = tr_moveable->num_meshes;
        TR_GenSkeletalModel(world, i, smodel, tr);
        smodel->fillTransparency();
    }
}


void TR_GenEntities(World *world, class VT_Level *tr)
{
    for(uint32_t i=0;i<tr->items_count;i++)
    {
        tr2_item_t *tr_item = &tr->items[i];
        std::shared_ptr<Entity> entity = (tr_item->object_id==0) ? std::make_shared<Character>(i) : std::make_shared<Entity>(i);
        entity->m_transform.getOrigin()[0] = tr_item->pos.x;
        entity->m_transform.getOrigin()[1] =-tr_item->pos.z;
        entity->m_transform.getOrigin()[2] = tr_item->pos.y;
        entity->m_angles[0] = tr_item->rotation;
        entity->m_angles[1] = 0;
        entity->m_angles[2] = 0;
        entity->updateTransform();
        if((tr_item->room >= 0) && ((uint32_t)tr_item->room < world->rooms.size()))
        {
            entity->m_self->room = world->rooms[tr_item->room].get();
        }
        else
        {
            entity->m_self->room = NULL;
        }

        entity->m_triggerLayout  = (tr_item->flags & 0x3E00) >> 9;   ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
        entity->m_OCB             = tr_item->ocb;
        entity->m_timer           = 0.0;

        entity->m_self->collision_type = COLLISION_TYPE_KINEMATIC;
        entity->m_self->collision_shape = COLLISION_SHAPE_TRIMESH;
        entity->m_moveType          = 0x0000;
        entity->m_inertiaLinear     = 0.0;
        entity->m_inertiaAngular[0] = 0.0;
        entity->m_inertiaAngular[1] = 0.0;
        entity->m_moveType          = 0;

        entity->m_bf.animations.model = world->getModelByID(tr_item->object_id);

        if(entity->m_bf.animations.model == nullptr) {
            int id = ent_ID_override["getOverridedID"](tr->game_version, tr_item->object_id);
            entity->m_bf.animations.model = world->getModelByID(id);
        }

        int replace_anim_id = ent_ID_override["getOverridedAnim"](tr->game_version, tr_item->object_id);
        if(replace_anim_id > 0) {
            SkeletalModel* replace_anim_model = world->getModelByID(replace_anim_id);
            std::swap(entity->m_bf.animations.model->animations, replace_anim_model->animations);
        }

        if(entity->m_bf.animations.model == NULL)
        {
            // SPRITE LOADING
            Sprite* sp = world->getSpriteByID(tr_item->object_id);
            if(sp && entity->m_self->room)
            {
                entity->m_self->room->sprites.emplace_back();
                RoomSprite& rsp = entity->m_self->room->sprites.back();
                rsp.sprite = sp;
                rsp.pos = entity->m_transform.getOrigin();
                rsp.was_rendered = false;
            }

            continue;                                                           // that entity has no model. may be it is a some trigger or look at object
        }

        if(tr->game_version < TR_II && tr_item->object_id == 83)                ///@FIXME: brutal magick hardcode! ;-)
        {
            // skip PSX save model
            continue;
        }

        entity->m_bf.fromModel(entity->m_bf.animations.model);

        if(0 == tr_item->object_id)                                             // Lara is unical model
        {
            std::shared_ptr<Character> lara = std::dynamic_pointer_cast<Character>(entity);
            assert(lara != nullptr);

            lara->m_moveType = MOVE_ON_FLOOR;
            world->character = lara;
            lara->m_self->collision_type = COLLISION_TYPE_ACTOR;
            lara->m_self->collision_shape = COLLISION_SHAPE_TRIMESH_CONVEX;
            lara->m_bf.animations.model->hide = 0;
            lara->m_typeFlags |= ENTITY_TYPE_TRIGGER_ACTIVATOR;
            SkeletalModel* LM = lara->m_bf.animations.model;

            engine_lua.set("player", lara->id());

            switch(tr->game_version)
            {
                case TR_I:
                    if(gameflow_manager.CurrentLevelID == 0)
                    {
                        LM = world->getModelByID(TR_ITEM_LARA_SKIN_ALTERNATE_TR1);
                        if(LM)
                        {
                            // In TR1, Lara has unified head mesh for all her alternate skins.
                            // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                            SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree.data(), LM->mesh_tree.data(), world->skeletal_models[0].mesh_count - 1);
                        }
                    }
                    break;

                case TR_III:
                    LM = world->getModelByID(TR_ITEM_LARA_SKIN_TR3);
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree.data(), LM->mesh_tree.data(), world->skeletal_models[0].mesh_count);
                        auto tmp = world->getModelByID(11);                   // moto / quadro cycle animations
                        if(tmp)
                        {
                            SkeletonCopyMeshes(tmp->mesh_tree.data(), LM->mesh_tree.data(), world->skeletal_models[0].mesh_count);
                        }
                    }
                    break;

                case TR_IV:
                case TR_IV_DEMO:
                case TR_V:
                    LM = world->getModelByID(TR_ITEM_LARA_SKIN_TR45);                         // base skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree.data(), LM->mesh_tree.data(), world->skeletal_models[0].mesh_count);
                    }
                    LM = world->getModelByID(TR_ITEM_LARA_SKIN_JOINTS_TR45);                         // skin skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes2(world->skeletal_models[0].mesh_tree.data(), LM->mesh_tree.data(), world->skeletal_models[0].mesh_count);
                    }
                    world->skeletal_models[0].fillSkinnedMeshMap();
                    break;
            };

            for(uint16_t j=0;j<lara->m_bf.bone_tags.size();j++)
            {
                lara->m_bf.bone_tags[j].mesh_base = lara->m_bf.animations.model->mesh_tree[j].mesh_base;
                lara->m_bf.bone_tags[j].mesh_skin = lara->m_bf.animations.model->mesh_tree[j].mesh_skin;
                lara->m_bf.bone_tags[j].mesh_slot = NULL;
            }
            world->character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            lara->genEntityRigidBody();
            lara->createGhosts();
            lara->m_height = 768.0;
            lara->state_func = State_Control_Lara;

            continue;
        }

        entity->setAnimation(0, 0);                                      // Set zero animation and zero frame

        Res_SetEntityModelProperties(entity);
        entity->rebuildBV();
        entity->genEntityRigidBody();

        entity->m_self->room->addEntity(entity.get());
        world->addEntity(entity);

        if(!entity->m_enabled || (entity->m_self->collision_type & 0x0001) == 0)
            entity->disableCollision();
    }
}


void TR_GenSamples(World *world, class VT_Level *tr)
{
    // Generate new buffer array.
    world->audio_buffers.resize(tr->samples_count, 0);
    alGenBuffers(world->audio_buffers.size(), world->audio_buffers.data());

    // Generate stream track map array.
    // We use scripted amount of tracks to define map bounds.
    // If script had no such parameter, we define map bounds by default.
    world->stream_track_map.resize( lua_GetNumTracks(engine_lua), 0 );
    if(world->stream_track_map.empty())
        world->stream_track_map.resize( TR_AUDIO_STREAM_MAP_SIZE, 0 );

    // Generate new audio effects array.
    world->audio_effects.resize(tr->sound_details_count);

    // Generate new audio emitters array.
    world->audio_emitters.resize(tr->sound_sources_count);

    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(!tr->samples_data.empty())
    {
        auto pointer = tr->samples_data.data();
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                world->audio_map.assign(tr->soundmap + 0, tr->soundmap + TR_AUDIO_MAP_SIZE_TR1);

                for(size_t i = 0; i < tr->samples_count-1; i++)
                {
                    pointer = tr->samples_data.data() + tr->sample_indices[i];
                    uint32_t size = tr->sample_indices[(i+1)] - tr->sample_indices[i];
                    Audio_LoadALbufferFromMem(world->audio_buffers[i], pointer, size);
                }
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
            {
                world->audio_map.assign(tr->soundmap + 0, tr->soundmap + ((tr->game_version == TR_III)?(TR_AUDIO_MAP_SIZE_TR3):(TR_AUDIO_MAP_SIZE_TR2)));
                size_t ind1 = 0;
                size_t ind2 = 0;
                bool flag = false;
                size_t i = 0;
                while(pointer < tr->samples_data.data() + tr->samples_data.size() - 4) {
                    pointer = tr->samples_data.data() + ind2;
                    if(0x46464952 == *((int32_t*)pointer)) {
                        // RIFF
                        if(!flag) {
                            ind1 = ind2;
                            flag = true;
                        }
                        else {
                            size_t uncomp_size = ind2 - ind1;
                            auto* srcData = tr->samples_data.data() + ind1;
                            Audio_LoadALbufferFromMem(world->audio_buffers[i], srcData, uncomp_size);
                            i++;
                            if(i >= world->audio_buffers.size())
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                size_t uncomp_size = tr->samples_data.size() - ind1;
                pointer = tr->samples_data.data() + ind1;
                if(i < world->audio_buffers.size()) {
                    Audio_LoadALbufferFromMem(world->audio_buffers[i], pointer, uncomp_size);
                }
                break;
            }

            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                world->audio_map.assign(tr->soundmap + 0, tr->soundmap + ((tr->game_version == TR_V)?(TR_AUDIO_MAP_SIZE_TR5):(TR_AUDIO_MAP_SIZE_TR4)));

                for(size_t i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    size_t uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    size_t comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromMem(world->audio_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                world->audio_map.resize( TR_AUDIO_MAP_SIZE_NONE );
                tr->samples_data.clear();
                return;
        }

        tr->samples_data.clear();
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(size_t i = 0; i < world->audio_effects.size(); i++)
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

    for(size_t i = 0; i < world->audio_emitters.size(); i++)
    {
        world->audio_emitters[i].emitter_index = i;
        world->audio_emitters[i].sound_index   =  tr->sound_sources[i].sound_id;
        world->audio_emitters[i].position[0]   =  tr->sound_sources[i].x;
        world->audio_emitters[i].position[1]   =  tr->sound_sources[i].z;
        world->audio_emitters[i].position[2]   = -tr->sound_sources[i].y;
        world->audio_emitters[i].flags         =  tr->sound_sources[i].flags;
    }
}


void Res_EntityToItem(std::map<uint32_t, std::shared_ptr<BaseItem> >& map)
{
    for(std::map<uint32_t, std::shared_ptr<BaseItem> >::iterator it = map.begin();
        it != map.end();
        ++it)
    {
        std::shared_ptr<BaseItem> item = it->second;

        for(const std::shared_ptr<Room>& room : engine_world.rooms)
        {
            for(const std::shared_ptr<EngineContainer>& cont : room->containers)
            {
                if(cont->object_type != OBJECT_ENTITY)
                    continue;

                Entity* ent = static_cast<Entity*>(cont->object);
                if(ent->m_bf.animations.model->id != item->world_model_id)
                    continue;

                char buf[64] = {0};
                snprintf(buf, 64, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", ent->id(), ent->id());
                engine_lua.doString(buf);

                snprintf(buf, 32, "pickup_init(%d, %d);", ent->id(), item->id);
                engine_lua.doString(buf);
                ent->disableCollision();
            }
        }
    }
}
