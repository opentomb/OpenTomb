
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>

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
#include "bounding_volume.h"
#include "engine.h"
#include "bordered_texture_atlas.h"
#include "render.h"
#include "redblack.h"

lua_State *collide_flags_conf;
lua_State *ent_ID_override;
lua_State *level_script;


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


void TR_Sector_SetTweenFloorConfig(struct sector_tween_s *tween)
{
    btScalar t;
    
    if(tween->floor_corners[0].m_floats[2] > tween->floor_corners[1].m_floats[2])
    {
        SWAPT(tween->floor_corners[0].m_floats[2], tween->floor_corners[1].m_floats[2], t);
    }
    if(tween->floor_corners[2].m_floats[2] < tween->floor_corners[3].m_floats[2])
    {
        SWAPT(tween->floor_corners[0].m_floats[2], tween->floor_corners[1].m_floats[2], t);
    }
    
    if((tween->floor_corners[0].m_floats[2] != tween->floor_corners[1].m_floats[2]) && 
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

void TR_Sector_SetTweenCeilingConfig(struct sector_tween_s *tween)
{
    btScalar t;
    
    if(tween->ceiling_corners[0].m_floats[2] > tween->ceiling_corners[1].m_floats[2])
    {
        SWAPT(tween->ceiling_corners[0].m_floats[2], tween->ceiling_corners[1].m_floats[2], t);
    }
    if(tween->ceiling_corners[2].m_floats[2] < tween->ceiling_corners[3].m_floats[2])
    {
        SWAPT(tween->ceiling_corners[0].m_floats[2], tween->ceiling_corners[1].m_floats[2], t);
    }
    
    if((tween->ceiling_corners[0].m_floats[2] != tween->ceiling_corners[1].m_floats[2]) && 
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

int TR_Sector_IsWall(room_sector_p ws, room_sector_p ns)
{
    if((ws->portal_to_room < 0) && (ns->portal_to_room < 0) && (ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL))
    {
        return 1;
    }
    
    if((ns->portal_to_room < 0) && (ns->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (ws->portal_to_room >= 0))
    {
        ws = Room_GetSector(engine_world.rooms + ws->portal_to_room, ws->pos);
        if((ws->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) || ((ws->owner_room->bb_min[2] >= ns->owner_room->bb_max[2]) || (ws->owner_room->bb_max[2] < ns->owner_room->bb_min[2])) && (ws->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
        {
            return 1;
        }
    }
    
    return 0;
}

room_sector_p TR_Sector_CheckPortalPointer(room_sector_p rs)
{
    if((rs != NULL) && (rs->portal_to_room >= 0))
    {
        room_p r = engine_world.rooms + rs->portal_to_room;
        int ind_x = (rs->pos[0] - r->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        rs = r->sectors + (ind_x * r->sectors_y + ind_y);
    }
    
    return rs;
}

void TR_Sector_GenTweens(struct room_s *room, struct sector_tween_s *room_tween)
{
    for(int h = 0; h < room->sectors_y-1; h++)
    {
        for(int w = 0; w < room->sectors_x-1; w++, room_tween++)
        {
            // Init X-plane tween [ | ]
            
            room_sector_p current_heightmap = room->sectors + (w * room->sectors_y + h);
            room_sector_p next_heightmap    = current_heightmap + 1;
            
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
            
            (room_tween+1)->floor_corners[0].m_floats[0] = current_heightmap->floor_corners[1].m_floats[0];
            (room_tween+1)->floor_corners[1].m_floats[0] = (room_tween+1)->floor_corners[0].m_floats[0];
            (room_tween+1)->floor_corners[2].m_floats[0] = (room_tween+1)->floor_corners[0].m_floats[0];
            (room_tween+1)->floor_corners[3].m_floats[0] = (room_tween+1)->floor_corners[0].m_floats[0];
            (room_tween+1)->floor_corners[0].m_floats[1] = current_heightmap->floor_corners[1].m_floats[1];
            (room_tween+1)->floor_corners[1].m_floats[1] = (room_tween+1)->floor_corners[0].m_floats[1];
            (room_tween+1)->floor_corners[2].m_floats[1] = current_heightmap->floor_corners[2].m_floats[1];
            (room_tween+1)->floor_corners[3].m_floats[1] = (room_tween+1)->floor_corners[2].m_floats[1];
            
            (room_tween+1)->ceiling_corners[0].m_floats[0] = current_heightmap->ceiling_corners[1].m_floats[0];
            (room_tween+1)->ceiling_corners[1].m_floats[0] = (room_tween+1)->ceiling_corners[0].m_floats[0];
            (room_tween+1)->ceiling_corners[2].m_floats[0] = (room_tween+1)->ceiling_corners[0].m_floats[0];
            (room_tween+1)->ceiling_corners[3].m_floats[0] = (room_tween+1)->ceiling_corners[0].m_floats[0];
            (room_tween+1)->ceiling_corners[0].m_floats[1] = current_heightmap->ceiling_corners[1].m_floats[1];
            (room_tween+1)->ceiling_corners[1].m_floats[1] = (room_tween+1)->ceiling_corners[0].m_floats[1];
            (room_tween+1)->ceiling_corners[2].m_floats[1] = current_heightmap->ceiling_corners[2].m_floats[1];
            (room_tween+1)->ceiling_corners[3].m_floats[1] = (room_tween+1)->ceiling_corners[2].m_floats[1];
            
            //if((room->id == 153)  && (w == 1) && (h == 0))                      // TR3, MAP 1
            {
                //Con_AddLine("AHTUNG!");
            }
                    
            if(w > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))                                                           // Init X-plane tween [ | ]
                {
                    if(TR_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->owner_room->bb_min[2];//current_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = current_heightmap->owner_room->bb_max[2];//current_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = current_heightmap->owner_room->bb_max[2];//current_heightmap->ceiling_corners[1].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->owner_room->bb_min[2];//current_heightmap->floor_corners[1].m_floats[2];
                        TR_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                    }
                    else if(TR_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = next_heightmap->owner_room->bb_min[2];//next_heightmap->floor_corners[3].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->owner_room->bb_max[2];//next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->owner_room->bb_max[2];//next_heightmap->ceiling_corners[2].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = next_heightmap->owner_room->bb_min[2];//next_heightmap->floor_corners[2].m_floats[2];
                        TR_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                    }
                    else
                    {
                        /************************** SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || Sectors_Is2SidePortals(current_heightmap, next_heightmap))
                        {
                            current_heightmap = TR_Sector_CheckPortalPointer(current_heightmap);
                            next_heightmap = TR_Sector_CheckPortalPointer(next_heightmap);
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[0].m_floats[2];
                                    room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                                    room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[2].m_floats[2];
                                    room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                                    TR_Sector_SetTweenFloorConfig(room_tween);
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[0].m_floats[2];
                                    room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                                    room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[2].m_floats[2];
                                    room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                                    TR_Sector_SetTweenCeilingConfig(room_tween);
                                }
                            }
                        }
                    }
                }

                current_heightmap = room->sectors + (w * room->sectors_y + h);
                next_heightmap    = current_heightmap + 1;
                if((room_tween->floor_tween_type == TR_SECTOR_TWEEN_TYPE_NONE) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                    if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = TR_Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSector(current_heightmap->sector_above->owner_room, next_heightmap->pos);
                            if(rs && (rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }
                    
                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = TR_Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSector(next_heightmap->sector_above->owner_room, current_heightmap->pos);
                            if(rs && (rs->portal_to_room == current_heightmap->owner_room->id))
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
                        TR_Sector_SetTweenFloorConfig(room_tween);
                    }
                }
            }
            
            /*****************************************************************************************************
             ********************************   CENTRE  OF  THE  ALGORITHM   *************************************
             *****************************************************************************************************/
            
            room_tween++;
            current_heightmap = room->sectors + (w * room->sectors_y + h);
            next_heightmap = room->sectors + ((w + 1) * room->sectors_y + h);
            if(h > 0)
            {
                if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                {
                    // Init Y-plane tween  [ - ]
                    if(TR_Sector_IsWall(next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = current_heightmap->owner_room->bb_min[2];//current_heightmap->floor_corners[1].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = current_heightmap->owner_room->bb_max[2];//current_heightmap->ceiling_corners[1].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = current_heightmap->owner_room->bb_max[2];//current_heightmap->ceiling_corners[2].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = current_heightmap->owner_room->bb_min[2];//current_heightmap->floor_corners[2].m_floats[2];
                        TR_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                    }
                    else if(TR_Sector_IsWall(current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0].m_floats[2] = next_heightmap->owner_room->bb_max[2];//next_heightmap->floor_corners[0].m_floats[2];
                        room_tween->floor_corners[1].m_floats[2] = next_heightmap->owner_room->bb_min[2];//next_heightmap->ceiling_corners[0].m_floats[2];
                        room_tween->floor_corners[2].m_floats[2] = next_heightmap->owner_room->bb_min[2];//next_heightmap->ceiling_corners[3].m_floats[2];
                        room_tween->floor_corners[3].m_floats[2] = next_heightmap->owner_room->bb_max[2];//next_heightmap->floor_corners[3].m_floats[2];
                        TR_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                    }
                    else
                    {
                        /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
                        if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || Sectors_Is2SidePortals(current_heightmap, next_heightmap))
                        {
                            current_heightmap = TR_Sector_CheckPortalPointer(current_heightmap);
                            next_heightmap = TR_Sector_CheckPortalPointer(next_heightmap);
                            if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                            {
                                if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->floor_corners[0].m_floats[2] = current_heightmap->floor_corners[1].m_floats[2];
                                    room_tween->floor_corners[1].m_floats[2] = next_heightmap->floor_corners[0].m_floats[2];
                                    room_tween->floor_corners[2].m_floats[2] = next_heightmap->floor_corners[3].m_floats[2];
                                    room_tween->floor_corners[3].m_floats[2] = current_heightmap->floor_corners[2].m_floats[2];
                                    TR_Sector_SetTweenFloorConfig(room_tween);
                                }
                                if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                                {
                                    room_tween->ceiling_corners[0].m_floats[2] = current_heightmap->ceiling_corners[1].m_floats[2];
                                    room_tween->ceiling_corners[1].m_floats[2] = next_heightmap->ceiling_corners[0].m_floats[2];
                                    room_tween->ceiling_corners[2].m_floats[2] = next_heightmap->ceiling_corners[3].m_floats[2];
                                    room_tween->ceiling_corners[3].m_floats[2] = current_heightmap->ceiling_corners[2].m_floats[2];
                                    TR_Sector_SetTweenCeilingConfig(room_tween);
                                }
                            }
                        }
                    }
                }
                
                if((room_tween->floor_tween_type == TR_SECTOR_TWEEN_TYPE_NONE) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                {
                    char valid = 0;
                                        if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != NULL) && (current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        next_heightmap = TR_Sector_CheckPortalPointer(next_heightmap);
                        if(next_heightmap->owner_room->id == current_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSector(current_heightmap->sector_above->owner_room, next_heightmap->pos);
                            if(rs && (rs->portal_to_room == next_heightmap->owner_room->id))
                            {
                                valid = 1;
                            }
                        }
                    }
                    
                    if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != NULL) && (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        current_heightmap = TR_Sector_CheckPortalPointer(current_heightmap);
                        if(current_heightmap->owner_room->id == next_heightmap->sector_above->owner_room->id)
                        {
                            valid = 1;
                        }
                        if(valid == 0)
                        {
                            room_sector_p rs = Room_GetSector(next_heightmap->sector_above->owner_room, current_heightmap->pos);
                            if(rs && (rs->portal_to_room == current_heightmap->owner_room->id))
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
                        TR_Sector_SetTweenFloorConfig(room_tween);
                    }
                }
            }
        }    ///END for
    }    ///END for
}

uint32_t TR_Sector_BiggestCorner(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4)
{
    uint32_t m1 = (v1 > v2)?(v1):(v2);
    uint32_t m2 = (v3 > v4)?(v3):(v4);
    return (m1 > m2)?(m1):(m2);
}

int TR_Sector_TranslateFloorData(room_sector_p sector, struct world_s *world)
{
    uint16_t function, function_value, sub_function, trigger_function, operands, cont_bit, end_bit;
    
    int       argn, ret = 0;
    uint16_t *entry, *end_p;
    
    char script[4096], buf[64];

    // Trigger options.
    uint8_t  trigger_mask;
    uint8_t  only_once;
    int8_t   timer_field;


    if(!sector || (sector->fd_index <= 0) || (sector->fd_index >= world->floor_data_size) || !engine_lua)
    {
        return 0;
    }

    /*
     * PARSE FUNCTIONS
     */
    end_p = world->floor_data + world->floor_data_size - 1;
    entry = world->floor_data + sector->fd_index;
    script[0] = 0;
    argn = 0;
    
    do
    {
        // TR_I - TR_II
        //function = (*entry) & 0x00FF;                   // 0b00000000 11111111
        //sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        //TR_III+, but works with TR_I - TR_II
        function       = ((*entry) & 0x001F);             // 0b00000000 00011111
        function_value = ((*entry) & 0x00E0) >> 5;        // 0b00000000 11100000  TR_III+
        sub_function   = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000
        end_bit        = ((*entry) & 0x8000) >> 15;       // 0b10000000 00000000

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

            case TR_FD_FUNC_TRIGGER:          // TRIGGER
                timer_field      =   (*entry) &  0x00FF;
                trigger_mask     =  ((*entry) &  0x3E00) >> 9;
                only_once        =  ((*entry) &  0x0100) >> 8;

                //Con_Printf("TRIGGER: timer - %d, once - %d, mask - %d%d%d%d%d", timer_field, only_once, trigger_mask[0], trigger_mask[1], trigger_mask[2], trigger_mask[3], trigger_mask[4]);
                script[0] = 0;
                argn = 0;
                switch(sub_function)
                {
                    case TR_FD_TRIGTYPE_TRIGGER:
                        // Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_TRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_PAD:
                        // Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_PAD");
                        break;
                    case TR_FD_TRIGTYPE_SWITCH:
                        strcat(script, "create_switch_func(");
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_SWITCH");
                        break;
                    case TR_FD_TRIGTYPE_KEY:
                        strcat(script, "create_keyhole_func(");
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_KEY");
                        break;
                    case TR_FD_TRIGTYPE_PICKUP:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_PICKUP");
                        break;
                    case TR_FD_TRIGTYPE_HEAVY:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVY");
                        break;
                    case TR_FD_TRIGTYPE_ANTIPAD:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_ANTIPAD");
                        break;
                    case TR_FD_TRIGTYPE_COMBAT:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_COMBAT");
                        break;
                    case TR_FD_TRIGTYPE_DUMMY:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_DUMMY");
                        break;
                    case TR_FD_TRIGTYPE_ANTITRIGGER:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_ANTITRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_HEAVYSWITCH:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVYSWITCH");
                        break;
                    case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVYANTITRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_MONKEY:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_MONKEY");
                        break;
                    case TR_FD_TRIGTYPE_SKELETON:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_SKELETON");
                        break;
                    case TR_FD_TRIGTYPE_TIGHTROPE:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_TIGHTROPE");
                        break;
                    case TR_FD_TRIGTYPE_CRAWLDUCK:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_CRAWLDUCK");
                        break;
                    case TR_FD_TRIGTYPE_CLIMB:
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_CLIMB");
                        break;
                }

                do
                {
                    entry++;
                    cont_bit = ((*entry) & 0x8000) >> 15;                       // 0b10000000 00000000
                    trigger_function = (((*entry) & 0x7C00)) >> 10;             // 0b01111100 00000000
                    operands = (*entry) & 0x03FF;                               // 0b00000011 11111111

                    switch(trigger_function)
                    {
                        case TR_FD_TRIGFUNC_OBJECT:                             // ACTIVATE / DEACTIVATE item
                            if(argn == 0)
                            {
                                snprintf(buf, 64, "%d, {", operands);           // switch / keyhole
                            }
                            else if(argn == 1)
                            {
                                snprintf(buf, 64, "%d", operands);
                            }
                            else
                            {
                                snprintf(buf, 64, ", %d", operands);
                            }
                            strcat(script, buf);
                            argn++;
                            break;

                        case TR_FD_TRIGFUNC_CAMERATARGET:          // CAMERA SWITCH
                            {
                                uint8_t cam_index = (*entry) & 0x007F;
                                entry++;
                                uint8_t cam_timer = ((*entry) & 0x00FF);
                                uint8_t cam_once  = ((*entry) & 0x0100) >> 8;
                                uint8_t cam_zoom  = ((*entry) & 0x1000) >> 12;
                                        cont_bit  = ((*entry) & 0x8000) >> 15;                       // 0b10000000 00000000

                                //Con_Printf("CAMERA: index = %d, timer = %d, once = %d, zoom = %d", cam_index, cam_timer, cam_once, cam_zoom);
                            }
                            break;

                        case TR_FD_TRIGFUNC_UWCURRENT:          // UNDERWATER CURRENT
                            //Con_Printf("UNDERWATER CURRENT! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPMAP:          // SET ALTERNATE ROOM
                            //Con_Printf("SET ALTERNATE ROOM! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPON:          // ALTER ROOM FLAGS (paired with 0x05)
                            //Con_Printf("ALTER ROOM FLAGS 0x04! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPOFF:          // ALTER ROOM FLAGS (paired with 0x04)
                            //Con_Printf("ALTER ROOM FLAGS 0x05! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_LOOKAT:          // LOOK AT ITEM
                            //Con_Printf("Look at %d item", operands);
                            break;

                        case TR_FD_TRIGFUNC_ENDLEVEL:          // END LEVEL
                            //Con_Printf("End of level! id = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_PLAYTRACK:          // PLAY CD TRACK
                            //Con_Printf("Play audiotrack id = %d", operands);
                            // operands - track number
                            break;

                        case TR_FD_TRIGFUNC_FLIPEFFECT:          // Various in-game actions.
                            //Con_Printf("Flipeffect id = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_SECRET:          // PLAYSOUND SECRET_FOUND
                            //Con_Printf("Play SECRET[%d] FOUND", operands);
                            break;

                        case TR_FD_TRIGFUNC_BODYBAG:          // UNKNOWN
                            //Con_Printf("BODYBAG id = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLYBY:          // TR4-5: FLYBY CAMERA
                            //Con_Printf("Flyby camera = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_CUTSCENE:          // USED IN TR4-5
                            //Con_Printf("CUTSCENE id = %d", operands);
                            break;

                        case 0x0e:          // UNKNOWN
                            //Con_Printf("TRIGGER: unknown 0x0e, OP = %d", operands);
                            break;

                        case 0x0f:          // UNKNOWN
                            //Con_Printf("TRIGGER: unknown 0x0f, OP = %d", operands);
                            break;
                    };
                }
                while(!cont_bit && entry < end_p);
                
                if(script[0])
                {
                    if((sub_function == TR_FD_TRIGTYPE_SWITCH) || (sub_function == TR_FD_TRIGTYPE_KEY))
                    {
                        snprintf(buf, 64, "}, nil, %d);", trigger_mask);
                        strcat(script, buf);
                        Con_Printf(script);
                        luaL_dostring(engine_lua, script);
                    }
                    script[0] = 0;
                }
                break;

            case TR_FD_FUNC_DEATH:          // KILL LARA
                //Con_Printf("KILL! sub = %d, b3 = %d", sub_function, b3);
                break;

            case TR_FD_FUNC_CLIMB:          // CLIMBABLE WALLS
                //Con_Printf("Climbable walls! sub = %d, b3 = %d", sub_function, b3);
                break;

            case TR_FD_FUNC_MONKEY:          // Climbable ceiling
                //Con_Printf("Climbable ceiling! sub = %d, b3 = %d", sub_function, b3);
                break;

            case TR_FD_FUNC_TRIGGERER_MARK:
                //Con_Printf("Trigger Triggerer (TR4) / MINECART LEFT (TR3), OP = %d", operands);
                break;

            case TR_FD_FUNC_BEETLE_MARK:
                //Con_Printf("Clockwork Beetle mark (TR4) / MINECART RIGHT (TR3), OP = %d", operands);
                break;

            default:                
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
                
                    float overall_adjustment = (float)TR_Sector_BiggestCorner(slope_t10, slope_t11, slope_t12, slope_t13) * TR_METERING_STEP;
                 
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

    if(sector->floor == 32512)
    {
        sector->floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
    }
    if(sector->ceiling == 32512)
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
    for(int anim = 0;anim < model->animation_count;anim++)
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
                                //Sys_DebugLog("anim_transform.txt", "dir[anim = %d, frame = %d, frames = %d]", anim, frame, af->frames_count);
                                break;
                        }
                    }
                    break;
            }
        }
    }
}


void BT_GenEntityRigidBody(entity_p ent)
{
    int i;
    btScalar tr[16];
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;
    if(!ent->bf.model)
    {
        return;
    }

    ent->bt_body = (btRigidBody**)malloc(ent->bf.model->mesh_count * sizeof(btRigidBody*));

    for(i=0;i<ent->bf.model->mesh_count;i++)
    {
        ent->bt_body[i] = NULL;
        cshape = BT_CSfromMesh(ent->bf.model->mesh_tree[i].mesh, true, true, ent->self->collide_flag);
        if(cshape)
        {
            Mat4_Mat4_mul_macro(tr, ent->transform, ent->bf.bone_tags[i].full_transform);
            startTransform.setFromOpenGLMatrix(tr);
            btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
            ent->bt_body[i] = new btRigidBody(0.0, motionState, cshape, localInertia);
            ///@TODO: use switch for COLLISION_GROUP correction
            bt_engine_dynamicsWorld->addRigidBody(ent->bt_body[i], COLLISION_GROUP_CINEMATIC, COLLISION_MASK_ALL);
            ent->bt_body[i]->setUserPointer(ent->self);
        }
    }
}

int TR_IsSectorsIn2SideOfPortal(room_sector_p s1, room_sector_p s2, portal_p p)
{
    if((s1->pos[0] == s2->pos[0]) && (s1->pos[1] != s2->pos[1]) && (fabs(p->norm[1]) > 0.99))
    {
        if(fabs(p->norm[1]) > 0.99)
        {
            btScalar min_x, max_x, min_y, max_y, x;
            max_x = min_x = p->vertex[0];
            for(int i=1;i<p->vertex_count;i++)
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
    }
    else if((s1->pos[0] != s2->pos[0]) && (s1->pos[1] == s2->pos[1]))
    {
        if(fabs(p->norm[0]) > 0.99)
        {
            btScalar min_x, max_x, min_y, max_y, y;
            max_y = min_y = p->vertex[1];
            for(int i=1;i<p->vertex_count;i++)
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
    }
    
    return 0;
}

void TR_Sector_Calculate(struct world_s *world, class VT_Level *tr, long int room_index)
{
    int i, j;
    room_sector_p sector;
    room_p room = world->rooms + room_index;
    tr5_room_t *tr_room = &tr->rooms[room_index];

    /*
     * Sectors loading
     */

    sector = room->sectors;
    for(i=0;i<room->sectors_count;i++,sector++)
    {
        /*
         * Let us fill pointers to sectors above and sectors below
         */

        j = tr_room->sector_list[i].room_below;
        sector->sector_below = NULL;
        if(j >= 0 && j < world->room_count && j != 255)
        {
            sector->sector_below = Room_GetSector(world->rooms + j, sector->pos);
        }   
        j = tr_room->sector_list[i].room_above;
        sector->sector_above = NULL;
        if(j >= 0 && j < world->room_count && j != 255)
        {
            sector->sector_above = Room_GetSector(world->rooms + j, sector->pos);
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
        
        /*if((sector->portal_to_room < 0) && (near_sector != NULL))
        {
            sector->floor = 32512;
            sector->ceiling = 32512;
            sector->floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }*/
        
        if((near_sector != NULL) && (sector->portal_to_room >= 0))
        {
            portal_p p = room->portals;
            for(j=0;j<room->portal_count;j++,p++)
            {
                if((p->norm[2] < 0.01) && ((p->norm[2] > -0.01)))
                {
                    room_sector_p dst = Room_GetSector(p->dest_room, sector->pos);
                    room_sector_p orig_dst = Room_GetSector(engine_world.rooms + sector->portal_to_room, sector->pos);
                    if((dst != NULL) && (dst->portal_to_room < 0) && (dst->floor != 32512) && (dst->ceiling != 32512) && (sector->portal_to_room != p->dest_room->id) && (dst->floor < orig_dst->floor) && TR_IsSectorsIn2SideOfPortal(near_sector, dst, p))
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

void TR_GenWorld(struct world_s *world, class VT_Level *tr)
{
    int32_t i;
    int lua_err, top;
    room_p r;
    base_mesh_p base_mesh;
    char buf[256], map[LEVEL_NAME_MAX_LEN];
    /// white texture data for coloured polygons and debug lines.
    GLubyte whtx[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    Gui_DrawLoadScreen(200);

    world->version = tr->game_version;

    buf[0] = 0;
    strcat(buf, "scripts/level/");
    if(tr->game_version < TR_II)
    {
        strcat(buf, "tr1/");
    }
    else if(tr->game_version < TR_III)
    {
        strcat(buf, "tr2/");
    }
    else if(tr->game_version < TR_IV)
    {
        strcat(buf, "tr3/");
    }
    else if(tr->game_version < TR_V)
    {
        strcat(buf, "tr4/");
    }
    else
    {
        strcat(buf, "tr5/");
    }

    Engine_GetLevelName(map, gameflow_manager.CurrentLevelPath);
    strcat(buf, map);
    strcat(buf, ".lua");

    level_script = luaL_newstate();
    if(level_script != NULL)
    {
        luaL_openlibs(level_script);
        lua_err = luaL_loadfile(level_script, buf);
        lua_pcall(level_script, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(level_script, -1));
            lua_pop(level_script, 1);
            lua_close(level_script);
            level_script = NULL;
        }
    }

    collide_flags_conf = luaL_newstate();
    if(collide_flags_conf != NULL)
    {
        luaL_openlibs(collide_flags_conf);
        lua_err = luaL_loadfile(collide_flags_conf, "scripts/entity/collide_flags.lua");
        lua_pcall(collide_flags_conf, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(collide_flags_conf, -1));
            lua_pop(collide_flags_conf, 1);
            lua_close(collide_flags_conf);
            collide_flags_conf = NULL;
        }
    }

    ent_ID_override = luaL_newstate();
    if(ent_ID_override != NULL)
    {
        luaL_openlibs(ent_ID_override);
        lua_err = luaL_loadfile(ent_ID_override, "scripts/entity/entity_model_ID_override.lua");
        lua_pcall(ent_ID_override, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(ent_ID_override, -1));
            lua_pop(ent_ID_override, 1);
            lua_close(ent_ID_override);
            ent_ID_override = NULL;
        }
    }

    luaL_dofile(engine_lua, "scripts/audio/common_sounds.lua");
    luaL_dofile(engine_lua, "scripts/audio/soundtrack.lua");
    luaL_dofile(engine_lua, "scripts/audio/sample_override.lua");

    Gui_DrawLoadScreen(250);

    world->Character = NULL;
    world->meshes = NULL;
    world->meshs_count = 0;
    world->room_count = 0;
    world->rooms = NULL;
    world->sprites_count = 0;
    world->sprites = NULL;
    world->entity_tree = RB_Init();
    world->entity_tree->rb_compEQ = compEntityEQ;
    world->entity_tree->rb_compLT = compEntityLT;
    world->entity_tree->rb_free_data = RBEntityFree;

    world->items_tree = RB_Init();
    world->items_tree->rb_compEQ = compEntityEQ;
    world->items_tree->rb_compLT = compEntityLT;
    world->items_tree->rb_free_data = RBItemFree;

    /*
     * Generate OGL textures
     */

    top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, "render");
    i = lua_GetScalarField(engine_lua,"texture_border");
    lua_settop(engine_lua, top);
    i = (i < 0)?(0):(i);
    i = (i > 128)?(128):(i);
    world->tex_atlas = BorderedTextureAtlas_Create(i);                    // here is border size
    for (i = 0; i < tr->textile32_count; i++)
    {
        BorderedTextureAtlas_AddPage(world->tex_atlas, tr->textile32[i].pixels);
    }

    Gui_DrawLoadScreen(300);

    for (i = 0; i < tr->sprite_textures_count; i++)
    {
        BorderedTextureAtlas_AddSpriteTexture(world->tex_atlas, tr->sprite_textures + i);
    }

    Gui_DrawLoadScreen(400);

    for (i = 0; i < tr->object_textures_count; i++)
    {
        BorderedTextureAtlas_AddObjectTexture(world->tex_atlas, tr->object_textures + i);
    }

    Gui_DrawLoadScreen(500);

    world->tex_count = (uint32_t) BorderedTextureAtlas_GetNumAtlasPages(world->tex_atlas) + 1;
    world->textures = (GLuint*)malloc(world->tex_count * sizeof(GLuint));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(1, 1);
    BorderedTextureAtlas_CreateTextures(world->tex_atlas, world->textures, 1);

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

    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, whtx);

    //glDisable(GL_TEXTURE_2D); // Why it is here? It is blocking loading screen.

    Gui_DrawLoadScreen(600);

    /*
     * copy sectors floordata
     */
    world->floor_data_size = tr->floor_data_size;
    world->floor_data = tr->floor_data;
    tr->floor_data = NULL;
    tr->floor_data_size = 0;

    /*
     * Copy anim commands
     */
    world->anim_commands_count = tr->anim_commands_count;
    world->anim_commands = tr->anim_commands;
    tr->anim_commands = NULL;
    tr->anim_commands_count = 0;

    /*
     * Generate anim textures
     */
    TR_GenAnimTextures(world, tr);

    Gui_DrawLoadScreen(650);

    /*
     * generate all meshes
     */
    world->meshs_count = tr->meshes_count;
    base_mesh = world->meshes = (base_mesh_p)malloc(world->meshs_count * sizeof(base_mesh_t));
    for(i=0;i<world->meshs_count;i++,base_mesh++)
    {
        TR_GenMesh(world, i, base_mesh, tr);
    }

    /*
     * generate sprites
     */
    TR_GenSprites(world, tr);

    Gui_DrawLoadScreen(700);

    /*
     * generate boxes
     */
    world->room_boxes = NULL;
    world->room_box_count = tr->boxes_count;
    if(world->room_box_count)
    {
        world->room_boxes = (room_box_p)malloc(world->room_box_count * sizeof(room_box_t));
        for(i=0;i<world->room_box_count;i++)
        {
            world->room_boxes[i].overlap_index = tr->boxes[i].overlap_index;
            world->room_boxes[i].true_floor =-tr->boxes[i].true_floor;
            world->room_boxes[i].x_min = tr->boxes[i].xmin;
            world->room_boxes[i].x_max = tr->boxes[i].xmax;
            world->room_boxes[i].y_min =-tr->boxes[i].zmax;
            world->room_boxes[i].y_max =-tr->boxes[i].zmin;
        }
    }

    /*
     * build all rooms
     */
    world->room_count = tr->rooms_count;
    r = world->rooms = (room_p)realloc(world->rooms, world->room_count * sizeof(room_t));
    for(i=0;i<world->room_count;i++,r++)
    {
        TR_GenRoom(i, r, world, tr);
        r->frustum = Frustum_Create();
    }

    Gui_DrawLoadScreen(800);

    /*
     * Build all skeletal models. Must be generated before TR_Sector_Calculate() function.
     */
    TR_GenSkeletalModels(world, tr);

    Gui_DrawLoadScreen(850);

    /*
     * build all moveables
     */
    TR_GenEntities(world, tr);

    r = world->rooms;
    for(i=0;i<world->room_count;i++,r++)
    {
        // Now, when building basic heightmap is finished, we can parse floordata
        // to load actual slant height values into it, plus fill out Lua script on the way.
        // These two tasks are joined here, because it allows to parse through floordata
        // once and for all, and then safely destroy it.
        
        // (Actually, floordata should never be copied into final map, when this is finished.)
        for(int j=0;j<r->sectors_count;j++)
        {
            TR_Sector_TranslateFloorData(r->sectors + j, world);
        }
        // Generate links to the near rooms.
        Room_BuildNearRoomsList(r);
        // basic sectors calculations
        TR_Sector_Calculate(world, tr, i);
    }

    r = world->rooms;
    
#if TR_MESH_ROOM_COLLISION
    for(i=0;i<world->room_count;i++,r++)
    {
        r->bt_body = NULL;
        
        if(r->mesh)
        {
            cshape = BT_CSfromMesh(r->mesh, true, true, COLLISION_TRIMESH);
            
            if(cshape)
            {
                startTransform.setFromOpenGLMatrix(r->transform);
                btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                r->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
                bt_engine_dynamicsWorld->addRigidBody(r->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
                r->bt_body->setUserPointer(room->self);
                r->self->collide_flag = COLLISION_TRIMESH;                   // meshtree
                if(!r->active)
                {
                    Room_Disable(r);
                }
            }
            
            SortPolygonsInMesh(r->mesh);
        }
    }
    
#else

    for(i=0;i<world->room_count;i++,r++)
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

        TR_Sector_GenTweens(r, room_tween);

        // Final step is sending actual sectors to Bullet collision model. We do it here.

        btCollisionShape *cshape = BT_CSfromHeightmap(r->sectors, num_heightmaps, room_tween, num_tweens, true, true);

        if(cshape)
        {
            btVector3 localInertia(0, 0, 0);
            btTransform tr;
            tr.setFromOpenGLMatrix(r->transform);
            btDefaultMotionState* motionState = new btDefaultMotionState(tr);
            r->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
            bt_engine_dynamicsWorld->addRigidBody(r->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
            r->bt_body->setUserPointer(r->self);
            r->self->collide_flag = COLLISION_TRIMESH;                          // meshtree
            if(!r->active)
            {
                Room_Disable(r);
            }
        }

        delete[] room_tween;
        
        if(r->mesh)
        {
            SortPolygonsInMesh(r->mesh);
        }
    }
    
#endif
    
    if(tr->game_version < TR_V)
    {
        r = world->rooms;
        for(i=0;i<world->room_count;i++,r++)
        {
            if(r->active && r->alternate_room)
            {
                Room_Disable(r->alternate_room);
            }
        }
    }

    Gui_DrawLoadScreen(900);

    // Initialize audio.

    Audio_Init(TR_AUDIO_MAX_CHANNELS, tr);

    Gui_DrawLoadScreen(950);
    
    // Find and set skybox.

    switch(tr->game_version)
    {
        case TR_II:
        case TR_II_DEMO:
            world->sky_box = World_FindModelByID(world, TR_ITEM_SKYBOX_TR2);
            break;

        case TR_III:
            world->sky_box = World_FindModelByID(world, TR_ITEM_SKYBOX_TR3);
            break;

        case TR_IV:
        case TR_IV_DEMO:
            world->sky_box = World_FindModelByID(world, TR_ITEM_SKYBOX_TR4);
            break;

        case TR_V:
            world->sky_box = World_FindModelByID(world, TR_ITEM_SKYBOX_TR5);
            break;

        default:
            world->sky_box = NULL;
            break;
    }
    
    // Load entity collision flags and ID overrides from script.

    if(collide_flags_conf)
    {
        lua_close(collide_flags_conf);
        collide_flags_conf = NULL;
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

    // Generate VBOs for meshes.

    for(i=0;i<world->meshs_count;i++)
    {
        if(world->meshes[i].vertex_count)
        {
            Mesh_GenVBO(world->meshes + i);
        }
    }

    Gui_DrawLoadScreen(1000);

    for(i=0;i<world->room_count;i++)
    {
        if((world->rooms[i].mesh) && (world->rooms[i].mesh->vertex_count))
        {
            Mesh_GenVBO(world->rooms[i].mesh);
        }
    }

    for(i=0;i<world->meshs_count;i++)
    {
        SortPolygonsInMesh(world->meshes + i);
    }

    buf[0] = 0;
    
    // Process level script loading.
    ///@FIXME: Re-assign script paths via script itself!

    strcat(buf, "scripts/level/");
    if(tr->game_version < TR_II)
    {
        strcat(buf, "tr1/");
    }
    else if(tr->game_version < TR_III)
    {
        strcat(buf, "tr2/");
    }
    else if(tr->game_version < TR_IV)
    {
        strcat(buf, "tr3/");
    }
    else if(tr->game_version < TR_V)
    {
        strcat(buf, "tr4/");
    }
    else
    {
        strcat(buf, "tr5/");
    }

    Engine_GetLevelName(map, gameflow_manager.CurrentLevelPath);
    strcat(buf, map);
    strcat(buf, "_autoexec.lua");

    luaL_dofile(engine_lua, buf);
    
    // Set loadscreen fader to fade-in state.
    Gui_FadeStart(FADER_LOADSCREEN, TR_FADER_DIR_IN);
}


void TR_GenRoom(size_t room_index, struct room_s *room, struct world_s *world, class VT_Level *tr)
{
    int i, j, top;
    portal_p p;
    room_p r_dest;
    tr5_room_t *tr_room = &tr->rooms[room_index];
    tr_staticmesh_t *tr_static;
    static_mesh_p r_static;
    tr_room_portal_t *tr_portal;
    room_sector_p sector;
    btScalar pos[3];
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;

    room->id = room_index;
    room->active = 1;
    room->portal_count = 0;
    room->portals = NULL;
    room->frustum = NULL;
    room->is_in_r_list = 0;
    room->hide = 0;
    room->max_path = 0;
    room->active_frustums = 0;
    room->containers = NULL;
    room->near_room_list_size = 0;
    room->sprites_count = 0;
    room->sprites = NULL;
    room->flags = tr->rooms[room_index].flags;
    room->reverb_info = tr->rooms[room_index].reverb_info;
    room->extra_param = tr->rooms[room_index].extra_param;
    Mat4_E_macro(room->transform);
    room->transform[12] = tr->rooms[room_index].offset.x;                       // x = x;
    room->transform[13] =-tr->rooms[room_index].offset.z;                       // y =-z;
    room->transform[14] = tr->rooms[room_index].offset.y;                       // z = y;
    room->ambient_lighting[0] = tr->rooms[room_index].light_colour.r * 2;
    room->ambient_lighting[1] = tr->rooms[room_index].light_colour.g * 2;
    room->ambient_lighting[2] = tr->rooms[room_index].light_colour.b * 2;
    room->self = (engine_container_p)malloc(sizeof(engine_container_t));
    room->self->room = room;
    room->self->next = NULL;
    room->self->object = NULL;
    room->self->object_type = OBJECT_ROOM_BASE;

    TR_GenRoomMesh(world, room_index, room, tr);
    
    room->bt_body = NULL;
    /*
     *  let us load static room meshes
     */
    room->static_mesh_count = tr_room->num_static_meshes;
    room->static_mesh = NULL;
    if(room->static_mesh_count)
    {
        room->static_mesh = (static_mesh_p)malloc(room->static_mesh_count * sizeof(static_mesh_t));
    }

    r_static = room->static_mesh;
    for(i=0;i<tr_room->num_static_meshes;i++)
    {
        tr_static = tr->find_staticmesh_id(tr_room->static_meshes[i].object_id);
        if(tr_static == NULL)
        {
            room->static_mesh_count--;
            continue;
        }
        r_static->self = (engine_container_p)malloc(sizeof(engine_container_t));
        r_static->self->room = room;
        r_static->self->next = NULL;
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
        r_static->bv = BV_Create();

        r_static->cbb_min[0] = tr_static->collision_box[0].x;
        r_static->cbb_min[1] =-tr_static->collision_box[0].z;
        r_static->cbb_min[2] = tr_static->collision_box[1].y;
        r_static->cbb_max[0] = tr_static->collision_box[1].x;
        r_static->cbb_max[1] =-tr_static->collision_box[1].z;
        r_static->cbb_max[2] = tr_static->collision_box[0].y;
        vec3_copy(r_static->mesh->bb_min, r_static->cbb_min);
        vec3_copy(r_static->mesh->bb_max, r_static->cbb_max);

        r_static->vbb_min[0] = tr_static->visibility_box[0].x;
        r_static->vbb_min[1] =-tr_static->visibility_box[0].z;
        r_static->vbb_min[2] = tr_static->visibility_box[1].y;
        r_static->vbb_max[0] = tr_static->visibility_box[1].x;
        r_static->vbb_max[1] =-tr_static->visibility_box[1].z;
        r_static->vbb_max[2] = tr_static->visibility_box[0].y;

        r_static->bv->transform = room->static_mesh[i].transform;
        r_static->bv->r = room->static_mesh[i].mesh->R;
        Mat4_E(r_static->transform);
        Mat4_Translate(r_static->transform, r_static->pos);
        Mat4_RotateZ(r_static->transform, r_static->rot[0]);
        r_static->was_rendered = 0;
        BV_InitBox(r_static->bv, r_static->vbb_min, r_static->vbb_max);
        BV_Transform(r_static->bv);

        r_static->self->collide_flag = 0x0000;
        r_static->bt_body = NULL;
        r_static->hide = 0;

        if(collide_flags_conf)
        {
            top = lua_gettop(collide_flags_conf);                                               // save LUA stack
            lua_getglobal(collide_flags_conf, "GetStaticMeshFlags");                            // add to the up of stack LUA's function
            lua_pushinteger(collide_flags_conf, tr->game_version);                              // add to stack first argument
            lua_pushinteger(collide_flags_conf, r_static->object_id);                           // add to stack second argument
            lua_pcall(collide_flags_conf, 2, 2, 0);                                             // call that function
            r_static->self->collide_flag = 0xff & lua_tointeger(collide_flags_conf, -2);        // get returned value
            r_static->hide = lua_tointeger(collide_flags_conf, -1);                             // get returned value
            lua_settop(collide_flags_conf, top);                                                // restore LUA stack
        }

        if(level_script)
        {
            top = lua_gettop(level_script);                                                        // save LUA stack
            lua_getglobal(level_script, "GetStaticMeshFlags");                                     // add to the up of stack LUA's function

            if(lua_isfunction(level_script, -1))                                                   // If function exists...
            {
                lua_pushinteger(level_script, tr->game_version);                                   // add to stack first argument
                lua_pushinteger(level_script, r_static->object_id);                                // add to stack second argument
                lua_pcall(level_script, 2, 2, 0);                                                  // call that function
                r_static->self->collide_flag = 0xff & lua_tointeger(level_script, -2);             // get returned value
                r_static->hide = lua_tointeger(level_script, -1);                                  // get returned value
            }
            lua_settop(level_script, top);                                                         // restore LUA stack
        }

        if(r_static->self->collide_flag != 0x0000)
        {
            cshape = BT_CSfromMesh(r_static->mesh, true, true, r_static->self->collide_flag);
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
    if(room->sprites_count)
    {
        room->sprites = (room_sprite_p)malloc(room->sprites_count * sizeof(room_sprite_t));
        for(i=0;i<room->sprites_count;i++)
        {
            if(tr_room->sprites[i].texture >= 0 && tr_room->sprites[i].texture < world->sprites_count)
            {
                room->sprites[i].sprite = world->sprites + tr_room->sprites[i].texture;
                j = tr_room->sprites[i].vertex;
                TR_vertex_to_arr(room->sprites[i].pos, &tr_room->vertices[j].vertex);
                vec3_add(room->sprites[i].pos, room->sprites[i].pos, room->transform+12);
            }
            else
            {
                room->sprites[i].sprite = NULL;
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
    
    // To avoid manipulating with unnecessary information, we declare temporary
    // heightmap here, which will be operated with sector and floordata parsing,
    // then vertical inbetween polys will be constructed, and Bullet collisional
    // object will be created. Afterwards, we delete heightmap.

    sector = room->sectors;
    for(i=0;i<room->sectors_count;i++,sector++)
    {
        // Filling base sectors information.
        
        sector->index_x = i / room->sectors_y;
        sector->index_y = i % room->sectors_y;

        sector->pos[0] = room->transform[12] + sector->index_x * TR_METERING_SECTORSIZE + 512.0;
        sector->pos[1] = room->transform[13] + sector->index_y * TR_METERING_SECTORSIZE + 512.0;
        sector->pos[2] = 0.5 * (tr_room->y_bottom + tr_room->y_top);
        
        sector->owner_room = room;
        sector->box_index  = tr_room->sector_list[i].box_index;
        
        if(sector->box_index == 65535)
        {
            sector->box_index = -1;
        }
        
        sector->floor    = -256 * (int)tr_room->sector_list[i].floor;
        sector->ceiling  = -256 * (int)tr_room->sector_list[i].ceiling;
        sector->fd_index = tr_room->sector_list[i].fd_index;        ///@FIXME: GET RID OF THIS NONSENSE SOME DAY!
               
        // BUILDING CEILING HEIGHTMAP.
        
        // Penetration config is used later to build inbetween vertical collision polys.
        // If sector's penetration config is a wall, we simply build a vertical plane to
        // isolate this sector from top to bottom. Also, this allows to trick out wall
        // sectors inside another wall sectors to be ignored completely when building
        // collisional mesh.
        // Door penetration config means that we should either ignore sector collision
        // completely (classic door) or ignore one of the triangular sector parts (TR3+).
        
        if(sector->ceiling == 32512)
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_above != 255)
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
        
        if(sector->floor == 32512)
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_below != 255)
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

    for(i=0;i<tr_room->num_lights;i++)
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
    for(j=0;j<room->portal_count;j++,p++)
    {
        tr_portal = &tr_room->portals[j];
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
            pos[0] = 1.0;
            pos[1] = 0.0;
            pos[2] = 0.0;
            Portal_Move(p, pos);
        }

        // Y_MIN
        if((p->norm[1] > 0.999) && (((int)p->centre[1])%2))
        {
            pos[0] = 0.0;
            pos[1] = 1.0;
            pos[2] = 0.0;
            Portal_Move(p, pos);
        }

        // Z_MAX
        if((p->norm[2] <-0.999) && (((int)p->centre[2])%2))
        {
            pos[0] = 0.0;
            pos[1] = 0.0;
            pos[2] =-1.0;
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
    if(tr_room->alternate_room >= 0 && tr_room->alternate_room < tr->rooms_count)
    {
        room->alternate_room = world->rooms + tr_room->alternate_room;
    }
}

/**
 * sprites loading, works correct in TR1 - TR5
 */
void TR_GenSprites(struct world_s *world, class VT_Level *tr)
{
    int i, id;
    sprite_p s;
    tr_sprite_texture_t *tr_st;

    if(tr->sprite_textures_count == 0)
    {
        world->sprites = NULL;
        world->sprites_count = 0;
        return;
    }

    world->sprites_count = tr->sprite_textures_count;
    s = world->sprites = (sprite_p)malloc(world->sprites_count * sizeof(sprite_t));

    for(i=0;i<world->sprites_count;i++,s++)
    {
        tr_st = &tr->sprite_textures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        BorderedTextureAtlas_GetSpriteCoordinates(world->tex_atlas, i, &s->texture, s->tex_coord);
        s->flag = 0x00;
        s->id = 0;
    }

    for(i=0;i<tr->sprite_sequences_count;i++)
    {
        if(tr->sprite_sequences[i].offset >= 0 && tr->sprite_sequences[i].offset < world->sprites_count)
        {
            id = tr->sprite_sequences[i].object_id;
            world->sprites[tr->sprite_sequences[i].offset].id = id;
        }
    }
}

/**   Animated textures loading.
  *   Natively, animated textures stored as a stream of bitu16s, which
  *   is then parsed on the fly. What we do is parse this stream to the
  *   proper structures to be used later within renderer.
  */
void TR_GenAnimTextures(struct world_s *world, class VT_Level *tr)
{
    uint16_t *pointer;
    uint16_t  i, j, num_sequences, num_uvrotates;
    uint16_t  block_size = tr->animated_textures_count; // This is actually whole anim textures block size.
    int32_t   uvrotate_script = 0;

    pointer       = tr->animated_textures;
    num_uvrotates = tr->animated_textures_uv_count;

    num_sequences = *(pointer++);   // First word in a stream is sequence count.

    world->anim_sequences_count = num_sequences;
    world->anim_sequences = (anim_seq_p)malloc(num_sequences * sizeof(anim_seq_t));
    memset(world->anim_sequences, 0, sizeof(anim_seq_t) * num_sequences);   // Reset all structure.

    for(i = 0; i < num_sequences; i++)
    {
        world->anim_sequences[i].frame_count = *(pointer++) + 1;
        world->anim_sequences[i].frame_list  =  (uint32_t*)malloc(world->anim_sequences[i].frame_count * sizeof(uint32_t));

        // Fill up new sequence with frame list.
        world->anim_sequences[i].type          = TR_ANIMTEXTURE_FORWARD;
        world->anim_sequences[i].type_flag     = false; // Needed for proper reverse-type start-up.
        world->anim_sequences[i].frame_rate    = 0.05;  // Should be passed as 1 / FPS.
        world->anim_sequences[i].frame_time    = 0.0;   // Reset frame time to initial state.
        world->anim_sequences[i].current_frame = 0;     // Reset current frame to zero.

        for(int j = 0; j < world->anim_sequences[i].frame_count; j++)
        {
            world->anim_sequences[i].frame_list[j] = *(pointer++);  // Add one frame.
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

        if((i < num_uvrotates) && (uvrotate_script != 0))
        {
            world->anim_sequences[i].frame_lock       = true;
            world->anim_sequences[i].uvrotate         = true;
            world->anim_sequences[i].uvrotate_flag    = false;
            world->anim_sequences[i].uvrotate_time    = 0.0;

            if(uvrotate_script > 0)
            {
                world->anim_sequences[i].uvrotate_type    = TR_ANIMTEXTURE_UVROTATE_FORWARD;
                world->anim_sequences[i].uvrotate_speed   = (btScalar)(uvrotate_script);
            }
            else
            {
                world->anim_sequences[i].uvrotate_type    = TR_ANIMTEXTURE_UVROTATE_BACKWARD;
                world->anim_sequences[i].uvrotate_speed   = (btScalar)abs(uvrotate_script);
            }


            // Get texture height and divide it in half.
            // This way, we get a reference value which is used to identify
            // if scrolling is completed or not.

            world->anim_sequences[i].uvrotate_max     = (BorderedTextureAtlas_GetTextureHeight(world->tex_atlas, world->anim_sequences[i].frame_list[0]) / 2);
        }
    } // end for(i = 0; i < num_sequences; i++)
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
    int i, j;

    polygon->anim_id = 0;                           // Reset to 0 by default.
    tex_index = tex_index & TR_TEXTURE_INDEX_MASK;  ///@FIXME: Is it really needed?

    for(i = 0; i < world->anim_sequences_count; i++)
    {
        for(j = 0; j < world->anim_sequences[i].frame_count; j++)
        {
            if(world->anim_sequences[i].frame_list[j] == tex_index)
            {
                // If we have found assigned texture ID in animation texture lists,
                // we assign corresponding animation sequence to this polygon,
                // additionally specifying frame offset.
                polygon->anim_id      = i + 1;  // Animation sequence ID.
                polygon->anim_offset  = j;      // Animation frame offset.
                return true;
            }
        }
    }

    return false;   // No such TexInfo found in animation textures lists.
}


void SortPolygonsInMesh(struct base_mesh_s *mesh)
{
    int i, j;
    polygon_p buf;

    if(mesh->transparancy_flags == MESH_FULL_TRANSPERENCY)
    {
        return;
    }

    if(mesh->transparancy_flags == MESH_FULL_OPAQUE)
    {
        for(i=0;i<mesh->poly_count;i++)
        {
            Polygon_Clear(mesh->polygons + i);
        }
        mesh->poly_count = 0;
        free(mesh->polygons);
        mesh->polygons = NULL;
        return;
    }

    buf = (polygon_p)malloc(mesh->transparancy_count * sizeof(polygon_t));

    for(i=0,j=0;i<mesh->poly_count;i++)
    {
        if(mesh->polygons[i].transparency > 1)
        {
            Polygon_Copy(buf+j, mesh->polygons + i);
            j++;
        }
        else
        {
            Polygon_Clear(mesh->polygons + i);
        }
    }

    free(mesh->polygons);
    mesh->poly_count = mesh->transparancy_count;
    mesh->polygons = buf;
}

void TR_GenMesh(struct world_s *world, size_t mesh_index, struct base_mesh_s *mesh, class VT_Level *tr)
{
    int i, col;
    tr4_mesh_t *tr_mesh;
    tr4_face4_t *face4;
    tr4_face3_t *face3;
    tr4_object_texture_t *tex;
    polygon_p p;
    btScalar *t, n;
    vertex_p vertex;

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
    mesh->transparancy_flags = 0;
    mesh->transparancy_count = 0;
    mesh->skin_map = NULL;
    mesh->num_texture_pages = (uint32_t)BorderedTextureAtlas_GetNumAtlasPages(world->tex_atlas) + 1;
    mesh->elements = NULL;
    mesh->element_count_per_texture = NULL;
    mesh->vbo_index_array = 0;
    mesh->vbo_vertex_array = 0;
    mesh->uses_vertex_colors = 0;

    mesh->vertex_count = tr_mesh->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(i=0;i<mesh->vertex_count;i++,vertex++)
    {
        TR_vertex_to_arr(vertex->position, &tr_mesh->vertices[i]);
        vec3_set_zero(vertex->normal);                                          // paranoid
    }

    mesh->poly_count = tr_mesh->num_textured_triangles + tr_mesh->num_coloured_triangles + tr_mesh->num_textured_rectangles + tr_mesh->num_coloured_rectangles;
    p = mesh->polygons = Polygon_CreateArray(mesh->poly_count);

    /*
     * textured triangles
     */
    for(i=0;i<tr_mesh->num_textured_triangles;i++,p++)
    {
        face3 = &tr_mesh->textured_triangles[i];
        tex = &tr->object_textures[face3->texture & TR_TEXTURE_INDEX_MASK];

        Polygon_Resize(p, 3);

        //p->double_side = face3->texture >> 15;    // CORRECT, BUT WRONG IN TR3-5
        if(tr->game_version < TR_III)
        {
            p->double_side = false;
        }
        else
        {
            p->double_side = true;
        }

        SetAnimTexture(p, face3->texture & TR_TEXTURE_INDEX_MASK, world);
        if(p->anim_id > 0)
        {
            if(tex->transparency_flags == 0)
            {
                tex->transparency_flags = BM_ANIMATED_TEX;
            }
            else if(tex->transparency_flags == 1)
            {
                tex->transparency_flags = BM_MULTIPLY;
            }
        }
        if(tex->transparency_flags > 1)
        {
            mesh->transparancy_count++;
        }
        p->transparency = tex->transparency_flags;

        TR_vertex_to_arr(p->vertices[0].position, &tr_mesh->vertices[face3->vertices[0]]);
        TR_vertex_to_arr(p->vertices[1].position, &tr_mesh->vertices[face3->vertices[1]]);
        TR_vertex_to_arr(p->vertices[2].position, &tr_mesh->vertices[face3->vertices[2]]);
        Polygon_FindNormale(p);
        t = mesh->vertices[face3->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[2]].normal; vec3_add(t, t, p->plane);

        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[0].base_color[0] = 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[1] = 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[2] = 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[3] = 1.0f;

            p->vertices[1].base_color[0] = 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[1] = 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[2] = 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[3] = 1.0f;

            p->vertices[2].base_color[0] = 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[1] = 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[2] = 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[3] = 1.0f;

            vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
            vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
            vec4_copy(p->vertices[2].color, p->vertices[2].base_color);

            mesh->uses_vertex_colors = 1;
        }
        else
        {
            vec4_set_one(p->vertices[0].base_color);         vec4_set_one(p->vertices[0].color);
            vec4_set_one(p->vertices[1].base_color);         vec4_set_one(p->vertices[1].color);
            vec4_set_one(p->vertices[2].base_color);         vec4_set_one(p->vertices[2].color);
        }

        BorderedTextureAtlas_GetCoordinates(world->tex_atlas, face3->texture & TR_TEXTURE_INDEX_MASK, 0, p);
    }

    /*
     * coloured triangles
     */
    for(i=0;i<tr_mesh->num_coloured_triangles;i++,p++)
    {
        face3 = &tr_mesh->coloured_triangles[i];
        col = face3->texture & 0xff;
        Polygon_Resize(p, 3);
        p->tex_index = (uint32_t)BorderedTextureAtlas_GetNumAtlasPages(world->tex_atlas);
        p->transparency = 0;
        p->anim_id = 0;

        TR_vertex_to_arr(p->vertices[0].position, &tr_mesh->vertices[face3->vertices[0]]);
        TR_vertex_to_arr(p->vertices[1].position, &tr_mesh->vertices[face3->vertices[1]]);
        TR_vertex_to_arr(p->vertices[2].position, &tr_mesh->vertices[face3->vertices[2]]);
        Polygon_FindNormale(p);
        t = mesh->vertices[face3->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[2]].normal; vec3_add(t, t, p->plane);

        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[0].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[3] = (float)1.0;

            p->vertices[1].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[3] = (float)1.0;

            p->vertices[2].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face3->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[3] = (float)1.0;
        }
        else
        {
            p->vertices[0].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[0].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[0].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[0].base_color[3] = (float)1.0;

            p->vertices[1].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[1].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[1].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[1].base_color[3] = (float)1.0;

            p->vertices[2].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[2].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[2].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[2].base_color[3] = (float)1.0;

        }

        vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
        vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
        vec4_copy(p->vertices[2].color, p->vertices[2].base_color);

        p->vertices[0].tex_coord[0] = 0.0;
        p->vertices[0].tex_coord[1] = 0.0;
        p->vertices[1].tex_coord[0] = 1.0;
        p->vertices[1].tex_coord[1] = 0.0;
        p->vertices[2].tex_coord[0] = 1.0;
        p->vertices[2].tex_coord[1] = 1.0;

        mesh->uses_vertex_colors = 1;
    }

    /*
     * textured rectangles
     */
    for(i=0;i<tr_mesh->num_textured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->textured_rectangles[i];
        tex = &tr->object_textures[face4->texture & TR_TEXTURE_INDEX_MASK];
        Polygon_Resize(p, 4);

        //p->double_side = face3->texture >> 15;    // CORRECT, BUT WRONG IN TR3-5
        if(tr->game_version < TR_III)
        {
            p->double_side = false;
        }
        else
        {
            p->double_side = true;
        }

        SetAnimTexture(p, face4->texture & TR_TEXTURE_INDEX_MASK, world);
        if(p->anim_id > 0)
        {
            if(tex->transparency_flags == 0)
            {
                tex->transparency_flags = BM_ANIMATED_TEX;
            }
            else if(tex->transparency_flags == 1)
            {
                tex->transparency_flags = BM_MULTIPLY;
            }
        }
        if(tex->transparency_flags > 1)
        {
            mesh->transparancy_count++;
        }
        p->transparency = tex->transparency_flags;

        TR_vertex_to_arr(p->vertices[0].position, &tr_mesh->vertices[face4->vertices[0]]);
        TR_vertex_to_arr(p->vertices[1].position, &tr_mesh->vertices[face4->vertices[1]]);
        TR_vertex_to_arr(p->vertices[2].position, &tr_mesh->vertices[face4->vertices[2]]);
        TR_vertex_to_arr(p->vertices[3].position, &tr_mesh->vertices[face4->vertices[3]]);
        Polygon_FindNormale(p);
        t = mesh->vertices[face4->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[2]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[3]].normal; vec3_add(t, t, p->plane);

        vec4_set_one(p->vertices[0].base_color);         vec4_set_one(p->vertices[0].color);
        vec4_set_one(p->vertices[1].base_color);         vec4_set_one(p->vertices[1].color);
        vec4_set_one(p->vertices[2].base_color);         vec4_set_one(p->vertices[2].color);
        vec4_set_one(p->vertices[3].base_color);         vec4_set_one(p->vertices[3].color);

        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[0].base_color[0] = 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[1] = 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[2] = 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[3] = 1.0f;

            p->vertices[1].base_color[0] = 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[1] = 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[2] = 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[3] = 1.0f;

            p->vertices[2].base_color[0] = 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[1] = 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[2] = 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[3] = 1.0f;

            p->vertices[3].base_color[0] = 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[1] = 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[2] = 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[3] = 1.0f;

            vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
            vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
            vec4_copy(p->vertices[2].color, p->vertices[2].base_color);
            vec4_copy(p->vertices[3].color, p->vertices[3].base_color);

            mesh->uses_vertex_colors = 1;
        }
        else
        {
            vec4_set_one(p->vertices[0].base_color);         vec4_set_one(p->vertices[0].color);
            vec4_set_one(p->vertices[1].base_color);         vec4_set_one(p->vertices[1].color);
            vec4_set_one(p->vertices[2].base_color);         vec4_set_one(p->vertices[2].color);
            vec4_set_one(p->vertices[3].base_color);         vec4_set_one(p->vertices[3].color);
        }


        BorderedTextureAtlas_GetCoordinates(world->tex_atlas, face4->texture & TR_TEXTURE_INDEX_MASK, 0, p);
    }

    /*
     * coloured rectangles
     */
    for(i=0;i<tr_mesh->num_coloured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->coloured_rectangles[i];
        col = face4->texture & 0xff;
        Polygon_Resize(p, 4);
        p->tex_index = (uint32_t)BorderedTextureAtlas_GetNumAtlasPages(world->tex_atlas);
        p->transparency = 0;
        p->anim_id = 0;

        TR_vertex_to_arr(p->vertices[0].position, &tr_mesh->vertices[face4->vertices[0]]);
        TR_vertex_to_arr(p->vertices[1].position, &tr_mesh->vertices[face4->vertices[1]]);
        TR_vertex_to_arr(p->vertices[2].position, &tr_mesh->vertices[face4->vertices[2]]);
        TR_vertex_to_arr(p->vertices[3].position, &tr_mesh->vertices[face4->vertices[3]]);
        Polygon_FindNormale(p);
        t = mesh->vertices[face4->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[2]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[3]].normal; vec3_add(t, t, p->plane);

        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[0].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[0]] / (8192.0f));
            p->vertices[0].base_color[3] = (float)1.0;

            p->vertices[1].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[1]] / (8192.0f));
            p->vertices[1].base_color[3] = (float)1.0;

            p->vertices[2].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[2]] / (8192.0f));
            p->vertices[2].base_color[3] = (float)1.0;

            p->vertices[3].base_color[0] = (float)(tr->palette.colour[col].r / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[1] = (float)(tr->palette.colour[col].g / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[2] = (float)(tr->palette.colour[col].b / 255.0)
                * 1.0f - (tr_mesh->lights[face4->vertices[3]] / (8192.0f));
            p->vertices[3].base_color[3] = (float)1.0;
        }
        else
        {
            p->vertices[0].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[0].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[0].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[0].base_color[3] = (float)1.0;

            p->vertices[1].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[1].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[1].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[1].base_color[3] = (float)1.0;

            p->vertices[2].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[2].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[2].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[2].base_color[3] = (float)1.0;

            p->vertices[3].base_color[0] = (float)tr->palette.colour[col].r / 255.0;
            p->vertices[3].base_color[1] = (float)tr->palette.colour[col].g / 255.0;
            p->vertices[3].base_color[2] = (float)tr->palette.colour[col].b / 255.0;
            p->vertices[3].base_color[3] = (float)1.0;
        }

        vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
        vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
        vec4_copy(p->vertices[2].color, p->vertices[2].base_color);
        vec4_copy(p->vertices[3].color, p->vertices[3].base_color);

        p->vertices[0].tex_coord[0] = 0.0;
        p->vertices[0].tex_coord[1] = 0.0;
        p->vertices[1].tex_coord[0] = 1.0;
        p->vertices[1].tex_coord[1] = 0.0;
        p->vertices[2].tex_coord[0] = 1.0;
        p->vertices[2].tex_coord[1] = 1.0;
        p->vertices[3].tex_coord[0] = 0.0;
        p->vertices[3].tex_coord[1] = 1.0;

        mesh->uses_vertex_colors = 1;
    }

    /*
     * let us normalise normales %)
     */
    vertex = mesh->vertices;
    p = mesh->polygons;
    for(i=0;i<mesh->vertex_count;i++,vertex++)
    {
        vec3_norm(vertex->normal, n);
    }

    /*
     * triangles
     */
    for(i=0;i<tr_mesh->num_textured_triangles;i++,p++)
    {
        face3 = &tr_mesh->textured_triangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face3->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face3->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face3->vertices[2]].normal);
    }

    for(i=0;i<tr_mesh->num_coloured_triangles;i++,p++)
    {
        face3 = &tr_mesh->coloured_triangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face3->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face3->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face3->vertices[2]].normal);
    }

    /*
     * rectangles
     */
    for(i=0;i<tr_mesh->num_textured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->textured_rectangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face4->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face4->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face4->vertices[2]].normal);
        vec3_copy(p->vertices[3].normal, mesh->vertices[face4->vertices[3]].normal);
    }

    for(i=0;i<tr_mesh->num_coloured_rectangles;i++,p++)
    {
        face4 = &tr_mesh->coloured_rectangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face4->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face4->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face4->vertices[2]].normal);
        vec3_copy(p->vertices[3].normal, mesh->vertices[face4->vertices[3]].normal);
    }

    if(mesh->poly_count == mesh->transparancy_count)
    {
        mesh->transparancy_flags = MESH_FULL_TRANSPERENCY;
    }
    else if(mesh->transparancy_count == 0)
    {
        mesh->transparancy_flags = MESH_FULL_OPAQUE;
    }
    else
    {
        mesh->transparancy_flags = MESH_PART_TRANSPERENCY;
    }

    BaseMesh_FindBB(mesh);

    mesh->vertex_count = 0;
    Mesh_GenFaces(mesh);
}


void TR_GenRoomMesh(struct world_s *world, size_t room_index, struct room_s *room, class VT_Level *tr)
{
    int i;
    tr5_room_t *tr_room;
    tr4_face4_t *face4;
    tr4_face3_t *face3;
    tr4_object_texture_t *tex;
    polygon_p p;
    base_mesh_p mesh;
    btScalar *t, n;
    vertex_p vertex;

    tr_room = &tr->rooms[room_index];

    i = tr_room->num_triangles + tr_room->num_rectangles;
    if(i == 0)
    {
        room->mesh = NULL;
        return;
    }

    mesh = room->mesh = (base_mesh_p)malloc(sizeof(base_mesh_t));
    mesh->id = room_index;
    mesh->num_texture_pages = (uint32_t)BorderedTextureAtlas_GetNumAtlasPages(world->tex_atlas) + 1;
    mesh->elements = NULL;
    mesh->element_count_per_texture = NULL;
    mesh->centre[0] = 0.0;
    mesh->centre[1] = 0.0;
    mesh->centre[2] = 0.0;
    mesh->R = 0.0;
    mesh->transparancy_flags = 0;
    mesh->transparancy_count = 0;
    mesh->skin_map = NULL;
    mesh->vbo_index_array = 0;
    mesh->vbo_vertex_array = 0;
    mesh->uses_vertex_colors = 1; // This is implicitly true on room meshes

    mesh->vertex_count = tr_room->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(i=0;i<mesh->vertex_count;i++,vertex++)
    {
        TR_vertex_to_arr(vertex->position, &tr_room->vertices[i].vertex);
        vec3_set_zero(vertex->normal);                                               // paranoid
    }

    mesh->poly_count = tr_room->num_triangles + tr_room->num_rectangles;
    p = mesh->polygons = Polygon_CreateArray(mesh->poly_count);

    /*
     * triangles
     */
    for(i=0;i<tr_room->num_triangles;i++,p++)
    {
        face3 = &tr_room->triangles[i];
        tex = &tr->object_textures[face3->texture & TR_TEXTURE_INDEX_MASK];
        SetAnimTexture(p, face3->texture & TR_TEXTURE_INDEX_MASK, world);
        if(p->anim_id > 0)
        {
            if(tex->transparency_flags == 0)
            {
                tex->transparency_flags = BM_ANIMATED_TEX;
            }
            else if(tex->transparency_flags == 1)
            {
                tex->transparency_flags = BM_MULTIPLY;
            }
        }
        if(tex->transparency_flags > 1)
        {
            mesh->transparancy_count++;
        }
        Polygon_Resize(p, 3);

        p->transparency = tex->transparency_flags;

        TR_vertex_to_arr(p->vertices[0].position, &tr_room->vertices[face3->vertices[0]].vertex);
        TR_vertex_to_arr(p->vertices[1].position, &tr_room->vertices[face3->vertices[1]].vertex);
        TR_vertex_to_arr(p->vertices[2].position, &tr_room->vertices[face3->vertices[2]].vertex);
        Polygon_FindNormale(p);
        t = mesh->vertices[face3->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face3->vertices[2]].normal; vec3_add(t, t, p->plane);
        vec3_copy(p->vertices[0].normal, p->plane);
        vec3_copy(p->vertices[1].normal, p->plane);
        vec3_copy(p->vertices[2].normal, p->plane);

        TR_color_to_arr(p->vertices[0].base_color, &tr_room->vertices[face3->vertices[0]].colour);
        TR_color_to_arr(p->vertices[1].base_color, &tr_room->vertices[face3->vertices[1]].colour);
        TR_color_to_arr(p->vertices[2].base_color, &tr_room->vertices[face3->vertices[2]].colour);

        vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
        vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
        vec4_copy(p->vertices[2].color, p->vertices[2].base_color);

        BorderedTextureAtlas_GetCoordinates(world->tex_atlas, face3->texture & TR_TEXTURE_INDEX_MASK, 0, p);
    }

    /*
     * rectangles
     */
    for(i=0;i<tr_room->num_rectangles;i++,p++)
    {
        face4 = &tr_room->rectangles[i];
        tex = &tr->object_textures[face4->texture & TR_TEXTURE_INDEX_MASK];
        SetAnimTexture(p, face4->texture & TR_TEXTURE_INDEX_MASK, world);
        if(p->anim_id > 0)
        {
            if(tex->transparency_flags == 0)
            {
                tex->transparency_flags = BM_ANIMATED_TEX;
            }
            else if(tex->transparency_flags == 1)
            {
                tex->transparency_flags = BM_MULTIPLY;
            }
        }
        if(tex->transparency_flags > 1)
        {
            mesh->transparancy_count++;
        }
        Polygon_Resize(p, 4);

        p->transparency = tex->transparency_flags;
        
        TR_vertex_to_arr(p->vertices[0].position, &tr_room->vertices[face4->vertices[0]].vertex);
        TR_vertex_to_arr(p->vertices[1].position, &tr_room->vertices[face4->vertices[1]].vertex);
        TR_vertex_to_arr(p->vertices[2].position, &tr_room->vertices[face4->vertices[2]].vertex);
        TR_vertex_to_arr(p->vertices[3].position, &tr_room->vertices[face4->vertices[3]].vertex);
        Polygon_FindNormale(p);
        t = mesh->vertices[face4->vertices[0]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[1]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[2]].normal; vec3_add(t, t, p->plane);
        t = mesh->vertices[face4->vertices[3]].normal; vec3_add(t, t, p->plane);
        vec3_copy(p->vertices[0].normal, p->plane);
        vec3_copy(p->vertices[1].normal, p->plane);
        vec3_copy(p->vertices[2].normal, p->plane);
        vec3_copy(p->vertices[3].normal, p->plane);

        TR_color_to_arr(p->vertices[0].base_color, &tr_room->vertices[face4->vertices[0]].colour);
        TR_color_to_arr(p->vertices[1].base_color, &tr_room->vertices[face4->vertices[1]].colour);
        TR_color_to_arr(p->vertices[2].base_color, &tr_room->vertices[face4->vertices[2]].colour);
        TR_color_to_arr(p->vertices[3].base_color, &tr_room->vertices[face4->vertices[3]].colour);

        vec4_copy(p->vertices[0].color, p->vertices[0].base_color);
        vec4_copy(p->vertices[1].color, p->vertices[1].base_color);
        vec4_copy(p->vertices[2].color, p->vertices[2].base_color);
        vec4_copy(p->vertices[3].color, p->vertices[3].base_color);

        BorderedTextureAtlas_GetCoordinates(world->tex_atlas, face4->texture & TR_TEXTURE_INDEX_MASK, 0, p);
    }

    /*
     * let us normalise normales %)
     */

    vertex = mesh->vertices;
    p = mesh->polygons;
    for(i=0;i<mesh->vertex_count;i++,vertex++)
    {
        vec3_norm(vertex->normal, n);
    }

    /*
     * triangles
     */
    for(i=0;i<tr_room->num_triangles;i++,p++)
    {
        face3 = &tr_room->triangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face3->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face3->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face3->vertices[2]].normal);
    }

    /*
     * rectangles
     */
    for(i=0;i<tr_room->num_rectangles;i++,p++)
    {
        face4 = &tr_room->rectangles[i];
        vec3_copy(p->vertices[0].normal, mesh->vertices[face4->vertices[0]].normal);
        vec3_copy(p->vertices[1].normal, mesh->vertices[face4->vertices[1]].normal);
        vec3_copy(p->vertices[2].normal, mesh->vertices[face4->vertices[2]].normal);
        vec3_copy(p->vertices[3].normal, mesh->vertices[face4->vertices[3]].normal);
    }

    if(mesh->poly_count == mesh->transparancy_count)
    {
        mesh->transparancy_flags = MESH_FULL_TRANSPERENCY;
    }
    else if(mesh->transparancy_count == 0)
    {
        mesh->transparancy_flags = MESH_FULL_OPAQUE;
    }
    else
    {
        mesh->transparancy_flags = MESH_PART_TRANSPERENCY;
    }

    mesh->vertex_count = 0;
    Mesh_GenFaces(mesh);
}


long int TR_GetOriginalAnimationFrameOffset(long int offset, long int anim, class VT_Level *tr)
{
    tr_animation_t *tr_animation;

    if(anim < 0 || anim >= tr->animations_count)
    {
        return -1;
    }

    tr_animation = &tr->animations[anim];
    if(anim == tr->animations_count - 1)
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


void TR_GenSkeletalModel(struct world_s *world, size_t model_num, struct skeletal_model_s *model, class VT_Level *tr)
{
    int i, j, k, l, l_start;
    tr_moveable_t *tr_moveable;
    tr_animation_t *tr_animation;

    uint32_t frame_offset, frame_step;
    uint16_t *frame, temp1, temp2;              ///@FIXME: "frame" set, but not used
    float ang;
    btScalar rot[3];

    bone_tag_p bone_tag;
    bone_frame_p bone_frame;
    mesh_tree_tag_p tree_tag;
    animation_frame_p anim;

    tr_moveable = &tr->moveables[model_num];                                    // original tr structure
    model->collision_map = (uint16_t*)malloc(model->mesh_count * sizeof(uint16_t));
    model->collision_map_size = model->mesh_count;
    for(i=0;i<model->mesh_count;i++)
    {
        model->collision_map[i] = i;
    }

    model->mesh_tree = (mesh_tree_tag_p)malloc(model->mesh_count * sizeof(mesh_tree_tag_t));
    tree_tag = model->mesh_tree;
    tree_tag->mesh2 = NULL;
    for(k=0;k<model->mesh_count;k++,tree_tag++)
    {
        tree_tag->mesh = model->mesh_offset + k;
        tree_tag->mesh2 = NULL;
        tree_tag->flag = 0x00;
        vec3_set_zero(tree_tag->offset);
        if(k == 0)
        {
            tree_tag->flag = 0x02;
            vec3_set_zero(tree_tag->offset);
        }
        else
        {
            uint32_t *tr_mesh_tree = tr->mesh_tree_data + tr_moveable->mesh_tree_index + (k-1)*4;
            tree_tag->flag = tr_mesh_tree[0];
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
        model->animations->frames = (bone_frame_p)malloc(model->animations->frames_count * sizeof(bone_frame_t));
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
        for(k=0;k<bone_frame->bone_tag_count;k++)
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
    model->animations = (animation_frame_p)malloc(model->animation_count * sizeof(animation_frame_t));
    anim = model->animations;
    for(i=0;i<model->animation_count;i++,anim++)
    {
        tr_animation = &tr->animations[tr_moveable->animation_index+i];
        frame_offset = tr_animation->frame_offset / 2;
        l_start = 0x09;
        if(tr->game_version == TR_I || tr->game_version == TR_I_DEMO || tr->game_version == TR_I_UB)
        {
            l_start = 0x0A;
        }
        frame_step = tr_animation->frame_size;

        //Sys_DebugLog(LOG_FILENAME, "frame_step = %d", frame_step);
        anim->id = i;
        anim->next_anim = NULL;
        anim->next_frame = 0;
        anim->original_frame_rate = tr_animation->frame_rate;
        anim->accel_hi = tr_animation->accel_hi;
        anim->accel_hi2 = tr_animation->accel_hi2;
        anim->accel_lo = tr_animation->accel_lo;
        anim->accel_lo2 = tr_animation->accel_lo2;
        anim->speed = tr_animation->speed;
        anim->speed2 = tr_animation->speed2;
        anim->anim_command = tr_animation->anim_command;
        anim->num_anim_commands = tr_animation->num_anim_commands;
        anim->state_id = tr_animation->state_id;
        anim->unknown = tr_animation->unknown;
        anim->unknown2 = tr_animation->unknown2;
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
        anim->frames = (bone_frame_p)malloc(anim->frames_count * sizeof(bone_frame_t));

        /*
         * let us begin to load animations
         */
        bone_frame = anim->frames;
        frame = tr->frame_data + frame_offset;
        for(j=0;j<anim->frames_count;j++,bone_frame++,frame_offset+=frame_step)
        {
            frame = tr->frame_data + frame_offset;
            bone_frame->bone_tag_count = model->mesh_count;
            bone_frame->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            vec3_set_zero(bone_frame->pos);
            vec3_set_zero(bone_frame->move);
            bone_frame->v_Horizontal = 0.0;
            bone_frame->v_Vertical = 0.0;
            bone_frame->command = 0x00;
            TR_GetBFrameBB_Pos(tr, frame_offset, bone_frame);

            if(frame_offset < 0 || frame_offset >= tr->frame_data_size)
            {
                //Con_Printf("Bad frame offset");
                for(k=0;k<bone_frame->bone_tag_count;k++)
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
                l = l_start;
                for(k=0;k<bone_frame->bone_tag_count;k++)
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
    GenerateAnimCommandsTransform(model);
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
    for(i=0;i<model->animation_count;i++,anim++)
    {
        anim->state_change_count = 0;
        anim->state_change = NULL;

        tr_animation = &tr->animations[tr_moveable->animation_index+i];
        j = (int)tr_animation->next_animation - (int)tr_moveable->animation_index;
        j &= 0x7fff;
        if(j >= 0 && j < model->animation_count)
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

            for(j=0;j<tr_animation->num_state_changes;j++,sch_p++)
            {
                tr_state_change_t *tr_sch;
                tr_sch = &tr->state_changes[j+tr_animation->state_change_offset];
                sch_p->id = tr_sch->state_id;
                sch_p->anim_dispath = NULL;
                sch_p->anim_dispath_count = 0;
                for(l=0;l<tr_sch->num_anim_dispatches;l++)
                {
                    tr_anim_dispatch_t *tr_adisp = &tr->anim_dispatches[tr_sch->anim_dispatch+l];
                    int next_anim = tr_adisp->next_animation & 0x7fff;
                    int next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                    if((next_anim_ind >= 0) &&(next_anim_ind < model->animation_count))
                    {
                        sch_p->anim_dispath_count++;
                        sch_p->anim_dispath = (anim_dispath_p)realloc(sch_p->anim_dispath, sch_p->anim_dispath_count * sizeof(anim_dispath_t));

                        anim_dispath_p adsp = sch_p->anim_dispath + sch_p->anim_dispath_count - 1;
                        int next_frames_count = model->animations[next_anim - tr_moveable->animation_index].frames_count;
                        int next_frame = tr_adisp->next_frame - tr->animations[next_anim].frame_start;

                        int low  = tr_adisp->low  - tr_animation->frame_start;
                        int high = tr_adisp->high - tr_animation->frame_start;

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

        bone_frame->centre[0] = (bone_frame->bb_min[0] + bone_frame->bb_max[0]) / 2.0;
        bone_frame->centre[1] = (bone_frame->bb_min[1] + bone_frame->bb_max[1]) / 2.0;
        bone_frame->centre[2] = (bone_frame->bb_min[2] + bone_frame->bb_max[2]) / 2.0;
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

        bone_frame->centre[0] = 0.0;
        bone_frame->centre[1] = 0.0;
        bone_frame->centre[2] = 0.0;
    }
}

void TR_GenSkeletalModels(struct world_s *world, class VT_Level *tr)
{
    uint32_t i, m_offset;
    skeletal_model_p smodel;
    tr_moveable_t *tr_moveable;

    world->skeletal_model_count = tr->moveables_count;
    smodel = world->skeletal_models = (skeletal_model_p)malloc(world->skeletal_model_count * sizeof(skeletal_model_t));

    for(i=0;i<world->skeletal_model_count;i++,smodel++)
    {
        tr_moveable = &tr->moveables[i];
        smodel->id = tr_moveable->object_id;
        smodel->mesh_count = tr_moveable->num_meshes;
        m_offset = tr->mesh_indices[tr_moveable->starting_mesh];
        smodel->mesh_offset = world->meshes + m_offset;                         // base mesh offset
        TR_GenSkeletalModel(world, i, smodel, tr);
        SkeletonModel_FillTransparancy(smodel);
    }
}


void TR_GenEntities(struct world_s *world, class VT_Level *tr)
{
    int i, j, top;

    tr2_item_t *tr_item;
    entity_p entity;

    for(i=0;i<tr->items_count;i++)
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
        if(tr_item->room >= 0 && tr_item->room < world->room_count)
        {
            entity->self->room = world->rooms + tr_item->room;
        }
        else
        {
            entity->self->room = NULL;
        }

        entity->activation_mask  = (tr_item->flags & 0x3E00) >> 9;  ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
        entity->OCB              =  tr_item->ocb;

        entity->self->collide_flag = 0x0000;
        entity->anim_flags = 0x0000;
        entity->flags = 0x00000000;
        entity->move_type = 0x0000;
        entity->bf.current_animation = 0;
        entity->bf.current_frame = 0;
        entity->bf.frame_time = 0.0;
        entity->inertia = 0.0;
        entity->move_type = 0;

        entity->bf.model = World_FindModelByID(world, tr_item->object_id);

        if(ent_ID_override)
        {
            if(entity->bf.model == NULL)
            {
                top = lua_gettop(ent_ID_override);                                         // save LUA stack
                lua_getglobal(ent_ID_override, "GetOverridedID");                          // add to the up of stack LUA's function
                lua_pushinteger(ent_ID_override, tr->game_version);                        // add to stack first argument
                lua_pushinteger(ent_ID_override, tr_item->object_id);                      // add to stack second argument
                lua_pcall(ent_ID_override, 2, 1, 0);                                       // call that function
                entity->bf.model = World_FindModelByID(world, lua_tointeger(ent_ID_override, -1));
                lua_settop(ent_ID_override, top);                                          // restore LUA stack
            }

            top = lua_gettop(ent_ID_override);                                         // save LUA stack
            lua_getglobal(ent_ID_override, "GetOverridedAnim");                        // add to the up of stack LUA's function
            lua_pushinteger(ent_ID_override, tr->game_version);                        // add to stack first argument
            lua_pushinteger(ent_ID_override, tr_item->object_id);                      // add to stack second argument
            lua_pcall(ent_ID_override, 2, 1, 0);                                       // call that function

            int replace_anim_id = lua_tointeger(ent_ID_override, -1);

            if(replace_anim_id > 0)
            {
                skeletal_model_s* replace_anim_model = World_FindModelByID(world, replace_anim_id);

                entity->bf.model->animations = replace_anim_model->animations;
                entity->bf.model->animation_count = replace_anim_model->animation_count;
            }

            lua_settop(ent_ID_override, top);
        }

        if(entity->bf.model == NULL)
        {
            // SPRITE LOADING
            sprite_p sp = World_FindSpriteByID(tr_item->object_id, world);
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

        if(tr->game_version < TR_II && tr_item->object_id == 83)
        {
            Entity_Clear(entity);                                               // skip PSX save model
            free(entity);
            continue;
        }

        entity->bf.bone_tag_count = entity->bf.model->mesh_count;
        entity->bf.bone_tags = (ss_bone_tag_p)malloc(entity->bf.bone_tag_count * sizeof(ss_bone_tag_t));
        for(j=0;j<entity->bf.bone_tag_count;j++)
        {
            entity->bf.bone_tags[j].flag = entity->bf.model->mesh_tree[j].flag;
            entity->bf.bone_tags[j].overrided = entity->bf.model->mesh_tree[j].overrided;
            entity->bf.bone_tags[j].mesh = entity->bf.model->mesh_tree[j].mesh;
            entity->bf.bone_tags[j].mesh2 = entity->bf.model->mesh_tree[j].mesh2;

            vec3_copy(entity->bf.bone_tags[j].offset, entity->bf.model->mesh_tree[j].offset);
            vec4_set_zero(entity->bf.bone_tags[j].qrotate);
            Mat4_E_macro(entity->bf.bone_tags[j].transform);
            Mat4_E_macro(entity->bf.bone_tags[j].full_transform);
        }

        if(0 == tr_item->object_id)                                             // Lara is unical model
        {
            skeletal_model_p tmp, LM;                                           // LM - Lara Model

            entity->move_type = MOVE_ON_FLOOR;
            world->Character = entity;
            entity->self->collide_flag = ENTITY_ACTOR_COLLISION;
            entity->bf.model->hide = 0;
            entity->flags = ENTITY_IS_ACTIVE | ENTITY_CAN_TRIGGER;
            LM = (skeletal_model_p)entity->bf.model;
            BV_InitBox(entity->bv, NULL, NULL);

            top = lua_gettop(engine_lua);
            lua_pushinteger(engine_lua, entity->id);
            lua_setglobal(engine_lua, "player");
            lua_settop(engine_lua, top);

            switch(tr->game_version)
            {
                case TR_I:
                    if(gameflow_manager.CurrentLevelID == 0)
                    {
                        LM = World_FindModelByID(world, TR_ITEM_LARA_SKIN_ALTERNATE_TR1);
                        if(LM)
                        {
                            // In TR1, Lara has unified head mesh for all her alternate skins.
                            // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                            SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count - 1);
                        }
                    }
                    break;

                case TR_III:
                    LM = World_FindModelByID(world, TR_ITEM_LARA_SKIN_TR3);
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        tmp = World_FindModelByID(world, 11);                   // moto / quadro cycle animations
                        if(tmp)
                        {
                            SkeletonCopyMeshes(tmp->mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        }
                    }
                    break;

                case TR_IV:
                case TR_IV_DEMO:
                case TR_V:
                    LM = World_FindModelByID(world, TR_ITEM_LARA_SKIN_TR45);                         // base skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    LM = World_FindModelByID(world, TR_ITEM_LARA_SKIN_JOINTS_TR45);                         // skin skeleton meshes
                    if(LM)
                    {
                        SkeletonCopyMeshes2(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    FillSkinnedMeshMap(&world->skeletal_models[0]);
                    break;
            };

            for(j=0;j<entity->bf.bone_tag_count;j++)
            {
                entity->bf.bone_tags[j].mesh = entity->bf.model->mesh_tree[j].mesh;
                entity->bf.bone_tags[j].mesh2 = entity->bf.model->mesh_tree[j].mesh2;
            }
            Entity_SetAnimation(world->Character, TR_ANIMATION_LARA_STAY_IDLE, 0);
            BT_GenEntityRigidBody(entity);
            Character_Create(entity, 128.0, 60.0, 780.0);
            entity->character->state_func = State_Control_Lara;
            continue;
        }

        Entity_SetAnimation(entity, 0, 0);                                      // Set zero animation and zero frame

        entity->self->collide_flag = 0x0000;
        entity->bf.model->hide = 0;

        if(collide_flags_conf)
        {
            top = lua_gettop(collide_flags_conf);                                               // save LUA stack
            lua_getglobal(collide_flags_conf, "GetEntityFlags");                                // add to the up of stack LUA's function
            lua_pushinteger(collide_flags_conf, tr->game_version);                              // add to stack first argument
            lua_pushinteger(collide_flags_conf, tr_item->object_id);                            // add to stack second argument
            lua_pcall(collide_flags_conf, 2, 2, 0);                                             // call that function
            entity->self->collide_flag = 0xff & lua_tointeger(collide_flags_conf, -2);          // get returned value
            entity->bf.model->hide = lua_tointeger(collide_flags_conf, -1);                     // get returned value
            lua_settop(collide_flags_conf, top);                                                // restore LUA stack
        }

        if(level_script)
        {
            top = lua_gettop(level_script);                                             // save LUA stack
            lua_getglobal(level_script, "GetEntityFlags");                              // add to the up of stack LUA's function

            if(lua_isfunction(level_script, -1))                                        // If function exists...
            {
                lua_pushinteger(level_script, tr->game_version);                        // add to stack first argument
                lua_pushinteger(level_script, tr_item->object_id);                      // add to stack second argument
                lua_pcall(level_script, 2, 2, 0);                                       // call that function
                entity->self->collide_flag = 0xff & lua_tointeger(level_script, -2);    // get returned value
                entity->bf.model->hide = lua_tointeger(level_script, -1);                  // get returned value
            }
            lua_settop(level_script, top);                                              // restore LUA stack
        }

        if(entity->self->collide_flag != 0x0000)
        {
            BT_GenEntityRigidBody(entity);
        }

        BV_InitBox(entity->bv, NULL, NULL);
        Entity_RebuildBV(entity);
        Room_AddEntity(entity->self->room, entity);
        World_AddEntity(world, entity);
    }
}


btCollisionShape *BT_CSfromHeightmap(struct room_sector_s *heightmap, int heightmap_size, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(int i = 0; i < heightmap_size; i++)
    {
        if( (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {   
            if( (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {                
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
                
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
            else
            {                
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         true);
                    cnt++;
                }
                
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
        }
        
        if( (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {   
            if( (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {                
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }
                
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }
                
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }
            }
        }
    }
    
    for(int i=0; i<tweens_size; i++)
    {   
        
        if(tweens[i].ceiling_tween_type == TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT)
        {
            trimesh->addTriangle(tweens[i].ceiling_corners[0],
                                 tweens[i].ceiling_corners[1],
                                 tweens[i].ceiling_corners[3],
                                 true);
            cnt++;
        }
        if(tweens[i].ceiling_tween_type == TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT)
        {
            trimesh->addTriangle(tweens[i].ceiling_corners[2],
                                 tweens[i].ceiling_corners[1],
                                 tweens[i].ceiling_corners[3],
                                 true);
            cnt++;
        }
        else if(tweens[i].ceiling_tween_type == TR_SECTOR_TWEEN_TYPE_QUAD)
        {
            trimesh->addTriangle(tweens[i].ceiling_corners[0],
                                 tweens[i].ceiling_corners[1],
                                 tweens[i].ceiling_corners[3],
                                 true);
            trimesh->addTriangle(tweens[i].ceiling_corners[2],
                                 tweens[i].ceiling_corners[1],
                                 tweens[i].ceiling_corners[3],
                                 true);
            cnt += 2;
        }
        
        
        if(tweens[i].floor_tween_type == TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT)
        {
            trimesh->addTriangle(tweens[i].floor_corners[0],
                                 tweens[i].floor_corners[1],
                                 tweens[i].floor_corners[3],
                                 true);
            cnt++;
        }
        else if(tweens[i].floor_tween_type == TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT)
        {
            trimesh->addTriangle(tweens[i].floor_corners[2],
                                 tweens[i].floor_corners[1],
                                 tweens[i].floor_corners[3],
                                 true);
            cnt++;
        }
        else if(tweens[i].floor_tween_type == TR_SECTOR_TWEEN_TYPE_QUAD)
        {
            trimesh->addTriangle(tweens[i].floor_corners[0],
                                 tweens[i].floor_corners[1],
                                 tweens[i].floor_corners[3],
                                 true);
            trimesh->addTriangle(tweens[i].floor_corners[2],
                                 tweens[i].floor_corners[1],
                                 tweens[i].floor_corners[3],
                                 true);
            cnt += 2;
        }
    }

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    return ret;
}


btCollisionShape *BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, int cflag)
{
    uint32_t i, j, cnt = 0;
    polygon_p p;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    btVector3 v0, v1, v2;
    bounding_volume_p bv;

    switch(cflag)
    {
        default:
        case COLLISION_TRIMESH:
            p = mesh->polygons;
            for(i=0;i<mesh->poly_count;i++,p++)
            {
                if(Polygon_IsBroken(p))
                {
                    continue;
                }

                for(j=1;j<p->vertex_count-1;j++)
                {
                    vec3_copy(v0.m_floats, p->vertices[j + 1].position);
                    vec3_copy(v1.m_floats, p->vertices[j].position);
                    vec3_copy(v2.m_floats, p->vertices[0].position);
                    trimesh->addTriangle(v0, v1, v2, true);
                }
                cnt ++;
            }

            if(cnt == 0)
            {
                delete trimesh;
                return NULL;
            }

            ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
            break;

        /*case COLLISION_SPHERE:
            ret = new btSphereShape(mesh->R);
            break;*/

        case COLLISION_BOX:                                                     // the box with deviated centre
            bv = BV_Create();
            BV_InitBox(bv, mesh->bb_min, mesh->bb_max);
            p = bv->base_polygons;
            for(i=0;i<6;i++,p++)
            {
                if(Polygon_IsBroken(p))
                {
                    continue;
                }
                for(j=1;j<p->vertex_count-1;j++)
                {
                    vec3_copy(v0.m_floats, p->vertices[0].position);
                    vec3_copy(v1.m_floats, p->vertices[j].position);
                    vec3_copy(v2.m_floats, p->vertices[j + 1].position);
                    trimesh->addTriangle(v0, v1, v2, true);
                }
                cnt ++;
            }

            if(cnt == 0)                                                        // fixed: without that condition engine may easily crash
            {
                delete trimesh;
                return NULL;
            }

            ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
            BV_Clear(bv);
            free(bv);
            break;

        case COLLISION_NONE:
            ret = NULL;
            break;
    };

    return ret;
}
