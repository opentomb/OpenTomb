
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "core/lua.h"
#include "core/system.h"
#include "core/vmath.h"
#include "core/gl_util.h"
#include "core/gl_text.h"
#include "core/console.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/render.h"
#include "render/frustum.h"
#include "render/bordered_texture_atlas.h"
#include "render/shader_description.h"
#include "audio/audio.h"
#include "script/script.h"
#include "physics/physics.h"
#include "vt/vt_level.h"

#include "room.h"
#include "trigger.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "character_controller.h"
#include "engine.h"
#include "entity.h"
#include "inventory.h"
#include "resource.h"


typedef struct fd_command_s
{
    uint16_t    function : 5;               // 0b00000000 00011111
    uint16_t    function_value : 3;         // 0b00000000 11100000  TR_III+
    uint16_t    sub_function : 7;           // 0b01111111 00000000
    uint16_t    end_bit : 1;                // 0b10000000 00000000
}fd_command_t, *fd_command_p;

typedef struct fd_trigger_head_s
{
    uint16_t    timer : 8;                  // 0b00000000 11111111   Used as common parameter for some commands.
    uint16_t    once : 1;                   // 0b00000001 00000000
    uint16_t    mask : 5;                   // 0b00111110 00000000
    uint16_t    uncnown : 2;
}fd_trigger_head_t, *fd_trigger_head_p;

typedef struct fd_trigger_function_s
{
    uint16_t    operands : 10;              // 0b00000011 11111111
    uint16_t    function : 5;               // 0b01111100 00000000
    uint16_t    cont_bit : 1;               // 0b10000000 00000000
}fd_trigger_function_t, *fd_trigger_function_p;

typedef struct fd_slope_s
{
    uint16_t    slope_t10 : 4;              // 0b00000000 00001111
    uint16_t    slope_t11 : 4;              // 0b00000000 11110000
    uint16_t    slope_t12 : 4;              // 0b00001111 00000000
    uint16_t    slope_t13 : 4;              // 0b11110000 00000000
}fd_slope_t, *fd_slope_p;


/*
 * Helper functions to convert legacy TR structs to native OpenTomb structs.
 */
__inline void TR_vertex_to_arr(float v[3], tr5_vertex_t *tr_v)
{
    v[0] = tr_v->x;
    v[1] =-tr_v->z;
    v[2] = tr_v->y;
}


__inline void TR_color_to_arr(float v[4], tr5_colour_t *tr_c)
{
    v[0] = tr_c->r * 2;
    v[1] = tr_c->g * 2;
    v[2] = tr_c->b * 2;
    v[3] = tr_c->a * 2;
}

/*
 * Functions for getting various parameters from legacy TR structs.
 */
int32_t  TR_GetNumAnimationsForMoveable(class VT_Level *tr, size_t moveable_ind);
int      TR_GetNumFramesForAnimation(class VT_Level *tr, size_t animation_ind);
void     TR_SkeletalModelInterpolateFrames(skeletal_model_p models);

// Main functions which are used to translate legacy TR floor data
// to native OpenTomb structs.
uint32_t Res_Sector_BiggestCorner(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
void     Res_Sector_SetTweenFloorConfig(struct sector_tween_s *tween);
void     Res_Sector_SetTweenCeilingConfig(struct sector_tween_s *tween);
struct room_sector_s *Res_Sector_GetPortalSectorTarget(struct room_sector_s *s);
bool     Res_Sector_Is2SidePortals(struct room_sector_s *s1, struct room_sector_s *s2);
bool     Res_Sector_IsWall(struct room_sector_s *wall_sector, struct room_sector_s *near_sector);
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
/// is_static - rooms flipping do not change collision geometry

uint32_t Res_Sector_BiggestCorner(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
{
    v1 = (v1 > v2) ? (v1) : (v2);
    v2 = (v3 > v4) ? (v3) : (v4);
    return (v1 > v2) ? (v1) : (v2);
}


void Res_Sector_SetTweenFloorConfig(struct sector_tween_s *tween)
{
    tween->floor_tween_inverted = 0x00;

    if(((tween->floor_corners[1][2] > tween->floor_corners[0][2]) && (tween->floor_corners[3][2] > tween->floor_corners[2][2])) ||
       ((tween->floor_corners[1][2] < tween->floor_corners[0][2]) && (tween->floor_corners[3][2] < tween->floor_corners[2][2])))
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

void Res_Sector_SetTweenCeilingConfig(struct sector_tween_s *tween)
{
    tween->ceiling_tween_inverted = 0x00;

    if(((tween->ceiling_corners[1][2] > tween->ceiling_corners[0][2]) && (tween->ceiling_corners[3][2] > tween->ceiling_corners[2][2])) ||
       ((tween->ceiling_corners[1][2] < tween->ceiling_corners[0][2]) && (tween->ceiling_corners[3][2] < tween->ceiling_corners[2][2])))
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


struct room_sector_s *Res_Sector_GetPortalSectorTarget(struct room_sector_s *s)
{
    if(s->portal_to_room)
    {
        s = Room_GetSectorRaw(s->portal_to_room->real_room, s->pos);
    }
    return s;
}


bool Res_Sector_Is2SidePortals(struct room_sector_s *s1, struct room_sector_s *s2)
{
    s1 = Res_Sector_GetPortalSectorTarget(s1);
    s2 = Res_Sector_GetPortalSectorTarget(s2);

    room_sector_p s1p = Room_GetSectorRaw(s2->owner_room, s1->pos);
    room_sector_p s2p = Room_GetSectorRaw(s1->owner_room, s2->pos);

    return (s1p->portal_to_room && s2p->portal_to_room &&
            (s1p->portal_to_room->real_room == s1->owner_room->real_room) &&
            (s2p->portal_to_room->real_room == s2->owner_room->real_room));
}


bool Res_Sector_IsWall(struct room_sector_s *wall_sector, struct room_sector_s *near_sector)
{
    if(!wall_sector->portal_to_room && !near_sector->portal_to_room && (wall_sector->floor_penetration_config == TR_PENETRATION_CONFIG_WALL))
    {
        return true;
    }

    if(!near_sector->portal_to_room && wall_sector->portal_to_room && (near_sector->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
    {
        wall_sector = Room_GetSectorRaw(wall_sector->portal_to_room->real_room, wall_sector->pos);
        if((wall_sector->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) || (!Res_Sector_Is2SidePortals(near_sector, wall_sector)))
        {
            return true;
        }
    }

    return false;
}


bool Res_Sector_IsTweenAlterable(struct room_sector_s *s1, struct room_sector_s *s2)
{
    if(s1->portal_to_room)
    {
        return s2->owner_room->alternate_room_next || s2->owner_room->alternate_room_prev ||
               s1->portal_to_room->alternate_room_next || s1->portal_to_room->alternate_room_prev;
    }
    else if(s2->portal_to_room)
    {
        return s1->owner_room->alternate_room_next || s1->owner_room->alternate_room_prev ||
               s2->portal_to_room->alternate_room_next || s2->portal_to_room->alternate_room_prev;
    }

    return false;
}


void Res_Sector_GenXTween(sector_tween_s *room_tween, room_sector_p current_heightmap, room_sector_p next_heightmap)
{
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

    if((next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) || (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))                                                           // Init X-plane tween [ | ]
    {
        if(Res_Sector_IsWall(next_heightmap, current_heightmap))
        {
            room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
            room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[0][2];
            room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[1][2];
            room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
            Res_Sector_SetTweenFloorConfig(room_tween);
            room_tween->floor_tween_inverted = 0x01;
            room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
        }
        else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
        {
            room_tween->floor_corners[0][2] = next_heightmap->floor_corners[3][2];
            room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[3][2];
            room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[2][2];
            room_tween->floor_corners[3][2] = next_heightmap->floor_corners[2][2];
            Res_Sector_SetTweenFloorConfig(room_tween);
            room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
        }
        else
        {
            /************************** SECTION WITH DROPS CALCULATIONS **********************/
            if((!current_heightmap->portal_to_room && !next_heightmap->portal_to_room) || Res_Sector_Is2SidePortals(current_heightmap, next_heightmap))
            {
                current_heightmap = Res_Sector_GetPortalSectorTarget(current_heightmap);
                next_heightmap    = Res_Sector_GetPortalSectorTarget(next_heightmap);
                if(!current_heightmap->portal_to_room && !next_heightmap->portal_to_room && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                {
                    if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->floor_tween_inverted = 0x01;
                    }
                    if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                        Res_Sector_SetTweenCeilingConfig(room_tween);
                    }
                }
            }
        }
    }
}


void Res_Sector_GenYTween(sector_tween_s *room_tween, room_sector_p current_heightmap, room_sector_p next_heightmap)
{
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
            room_tween->floor_tween_inverted = 0x01;
            room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
        }
        else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
        {
            room_tween->floor_corners[0][2] = next_heightmap->floor_corners[0][2];
            room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[0][2];
            room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[3][2];
            room_tween->floor_corners[3][2] = next_heightmap->floor_corners[3][2];
            Res_Sector_SetTweenFloorConfig(room_tween);
            room_tween->ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
        }
        else
        {
            /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
            if((!current_heightmap->portal_to_room && !next_heightmap->portal_to_room) || Res_Sector_Is2SidePortals(current_heightmap, next_heightmap))
            {
                current_heightmap = Res_Sector_GetPortalSectorTarget(current_heightmap);
                next_heightmap    = Res_Sector_GetPortalSectorTarget(next_heightmap);
                if(!current_heightmap->portal_to_room && !next_heightmap->portal_to_room && (current_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL) && (next_heightmap->floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
                {
                    if((current_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->floor_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                        Res_Sector_SetTweenFloorConfig(room_tween);
                        room_tween->floor_tween_inverted = 0x01;
                    }
                    if((current_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID) || (next_heightmap->ceiling_penetration_config == TR_PENETRATION_CONFIG_SOLID))
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                        Res_Sector_SetTweenCeilingConfig(room_tween);
                    }
                }
            }
        }
    }
}


///@TODO: resolve floor >> ceiling case
int  Res_Sector_GenStaticTweens(struct room_s *room, struct sector_tween_s *room_tween)
{
    int ret = 0;
    for(uint16_t h = 0; h < room->sectors_y - 1; h++)
    {
        for(uint16_t w = 0; w < room->sectors_x - 1; w++)
        {
            // Init X-plane tween [ | ]
            room_sector_p current_heightmap = room->content->sectors + (w * room->sectors_y + h);
            room_sector_p next_heightmap    = current_heightmap + 1;
            if((w > 0) && !Res_Sector_IsTweenAlterable(current_heightmap, next_heightmap))
            {
                Res_Sector_GenXTween(room_tween, current_heightmap, next_heightmap);
                if((room_tween->floor_tween_type != TR_SECTOR_TWEEN_TYPE_NONE) ||
                   (room_tween->ceiling_tween_type != TR_SECTOR_TWEEN_TYPE_NONE))
                {
                    room_tween++;
                    ret++;
                }
            }

            current_heightmap = room->content->sectors + (w * room->sectors_y + h);
            next_heightmap    = room->content->sectors + ((w + 1) * room->sectors_y + h);
            if((h > 0) && !Res_Sector_IsTweenAlterable(current_heightmap, next_heightmap))
            {
                Res_Sector_GenYTween(room_tween, current_heightmap, next_heightmap);
                if((room_tween->floor_tween_type != TR_SECTOR_TWEEN_TYPE_NONE) ||
                   (room_tween->ceiling_tween_type != TR_SECTOR_TWEEN_TYPE_NONE))
                {
                    room_tween++;
                    ret++;
                }
            }
        }
    }

    return ret;
}


int  Res_Sector_GenDynamicTweens(struct room_s *room, struct sector_tween_s *room_tween)
{
    int ret = 0;
    for(uint16_t h = 0; h < room->sectors_y - 1; h++)
    {
        for(uint16_t w = 0; w < room->sectors_x - 1; w++)
        {
            // Init X-plane tween [ | ]
            room_sector_p current_heightmap = room->content->sectors + (w * room->sectors_y + h);
            room_sector_p next_heightmap    = current_heightmap + 1;
            if((w > 0) && Res_Sector_IsTweenAlterable(current_heightmap, next_heightmap))
            {
                Res_Sector_GenXTween(room_tween, current_heightmap, next_heightmap);
                if((room_tween->floor_tween_type != TR_SECTOR_TWEEN_TYPE_NONE) ||
                   (room_tween->ceiling_tween_type != TR_SECTOR_TWEEN_TYPE_NONE))
                {
                    room_tween++;
                    ret++;
                }
            }

            current_heightmap = room->content->sectors + (w * room->sectors_y + h);
            next_heightmap    = room->content->sectors + ((w + 1) * room->sectors_y + h);
            if((h > 0) && Res_Sector_IsTweenAlterable(current_heightmap, next_heightmap))
            {
                Res_Sector_GenYTween(room_tween, current_heightmap, next_heightmap);
                if((room_tween->floor_tween_type != TR_SECTOR_TWEEN_TYPE_NONE) ||
                   (room_tween->ceiling_tween_type != TR_SECTOR_TWEEN_TYPE_NONE))
                {
                    room_tween++;
                    ret++;
                }
            }
        }
    }

    return ret;
}


int Res_Sector_In2SideOfPortal(struct room_sector_s *s1, struct room_sector_s *s2, struct portal_s *p)
{
    if((s1->pos[0] == s2->pos[0]) && (s1->pos[1] != s2->pos[1]) && (fabs(p->norm[1]) > 0.99))
    {
        float min_x, max_x, min_y, max_y, x;
        max_x = min_x = p->vertex[0];
        for(uint16_t i = 1; i < p->vertex_count; i++)
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
        float min_x, max_x, min_y, max_y, y;
        max_y = min_y = p->vertex[1];
        for(uint16_t i = 1; i < p->vertex_count; i++)
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


void Res_RoomLightCalculate(struct light_s *light, struct tr5_room_light_s *tr_light)
{
    switch(tr_light->light_type)
    {
        case 0:
            light->light_type = LT_SUN;
            break;

        case 1:
            light->light_type = LT_POINT;
            break;

        case 2:
            light->light_type = LT_SPOTLIGHT;
            break;

        case 3:
            light->light_type = LT_SHADOW;
            break;

        default:
            light->light_type = LT_NULL;
            break;
    }

    TR_vertex_to_arr(light->pos, &tr_light->pos);
    light->pos[3] = 1.0f;

    if(light->light_type == LT_SHADOW)
    {
        light->colour[0] = -(tr_light->color.r / 255.0f) * tr_light->intensity;
        light->colour[1] = -(tr_light->color.g / 255.0f) * tr_light->intensity;
        light->colour[2] = -(tr_light->color.b / 255.0f) * tr_light->intensity;
        light->colour[3] = 1.0f;
    }
    else
    {
        light->colour[0] = (tr_light->color.r / 255.0f) * tr_light->intensity;
        light->colour[1] = (tr_light->color.g / 255.0f) * tr_light->intensity;
        light->colour[2] = (tr_light->color.b / 255.0f) * tr_light->intensity;
        light->colour[3] = 1.0f;
    }

    light->inner = tr_light->r_inner;
    light->outer = tr_light->r_outer;
    light->length = tr_light->length;
    light->cutoff = tr_light->cutoff;

    light->falloff = 0.001f / light->outer;
}


void Res_RoomSectorsCalculate(struct room_s *rooms, uint32_t rooms_count, uint32_t room_index, class VT_Level *tr)
{
    room_sector_p sector;
    room_p room = rooms + room_index;
    tr5_room_t *tr_room = &tr->rooms[room_index];

    /*
     * Sectors loading
     */
    sector = room->content->sectors;
    for(uint32_t i = 0; i < room->sectors_count; i++, sector++)
    {
        /*
         * Let us fill pointers to sectors above and sectors below
         */
        uint8_t rp = tr_room->sector_list[i].room_below;
        sector->room_below = NULL;
        if(rp < rooms_count && rp != 255)
        {
            sector->room_below = rooms + rp;
        }
        rp = tr_room->sector_list[i].room_above;
        sector->room_above = NULL;
        if(rp < rooms_count && rp != 255)
        {
            sector->room_above = rooms + rp;
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

        if(near_sector && sector->portal_to_room)
        {
            portal_p p = room->content->portals;
            for(uint16_t j = 0; j < room->content->portals_count; j++, p++)
            {
                if((p->norm[2] < 0.01) && ((p->norm[2] > -0.01)))
                {
                    room_sector_p dst = Room_GetSectorRaw(p->dest_room, sector->pos);
                    room_sector_p orig_dst = Room_GetSectorRaw(sector->portal_to_room, sector->pos);
                    if(dst && !dst->portal_to_room && (dst->floor != TR_METERING_WALLHEIGHT) && (dst->ceiling != TR_METERING_WALLHEIGHT) && (sector->portal_to_room != p->dest_room) && (dst->floor < orig_dst->floor) && Res_Sector_In2SideOfPortal(near_sector, dst, p))
                    {
                        sector->portal_to_room = p->dest_room;
                        orig_dst = dst;
                    }
                }
            }
        }
    }
}


int Res_Sector_TranslateFloorData(struct room_s *rooms, uint32_t rooms_count, struct room_sector_s *sector, class VT_Level *tr)
{
    int ret = 0;

    if(!sector || (sector->trig_index <= 0) || (sector->trig_index >= tr->floor_data_size))
    {
        return ret;
    }

    uint16_t *entry = tr->floor_data + sector->trig_index;
    size_t max_offset = tr->floor_data_size;
    size_t current_offset = sector->trig_index;
    fd_command_t fd_command;

    sector->flags = 0;  // Clear sector flags before parsing.
    do
    {
        fd_command = *((fd_command_p)entry);
        entry++;
        current_offset++;
        switch(fd_command.function)
        {
            case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                if(fd_command.sub_function == 0x00)
                {
                    if(*entry < rooms_count)
                    {
                        sector->portal_to_room = rooms + *entry;
                        sector->floor_penetration_config   = TR_PENETRATION_CONFIG_GHOST;
                        sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_GHOST;
                    }
                    entry ++;
                    current_offset++;
                }
                break;

            case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                if(fd_command.sub_function == 0x00)
                {
                    int8_t raw_y_slant =  (*entry & 0x00FF);
                    int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                    sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
                    sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;

                    if(raw_x_slant > 0)
                    {
                        sector->floor_corners[2][2] -= ((float)raw_x_slant * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= ((float)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->floor_corners[0][2] -= (fabs((float)raw_x_slant) * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= (fabs((float)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->floor_corners[0][2] -= ((float)raw_y_slant * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= ((float)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->floor_corners[1][2] -= (fabs((float)raw_y_slant) * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= (fabs((float)raw_y_slant) * TR_METERING_STEP);
                    }

                    entry++;
                    current_offset++;
                }
                break;

            case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                if(fd_command.sub_function == 0x00)
                {
                    int8_t raw_y_slant =  (*entry & 0x00FF);
                    int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                    sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
                    sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;

                    if(raw_x_slant > 0)
                    {
                        sector->ceiling_corners[3][2] += ((float)raw_x_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += ((float)raw_x_slant * TR_METERING_STEP);
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector->ceiling_corners[1][2] += (fabs((float)raw_x_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[0][2] += (fabs((float)raw_x_slant) * TR_METERING_STEP);
                    }

                    if(raw_y_slant > 0)
                    {
                        sector->ceiling_corners[1][2] += ((float)raw_y_slant * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += ((float)raw_y_slant * TR_METERING_STEP);
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector->ceiling_corners[0][2] += (fabs((float)raw_y_slant) * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += (fabs((float)raw_y_slant) * TR_METERING_STEP);
                    }

                    entry++;
                    current_offset++;
                }
                break;

            case TR_FD_FUNC_TRIGGER:          // TRIGGERS
                {
                    fd_trigger_head_t fd_trigger_head = *((fd_trigger_head_p)entry);

                    if(sector->trigger == NULL)
                    {
                        sector->trigger = (trigger_header_p)malloc(sizeof(trigger_header_t));
                    }
                    else
                    {
                        Con_AddLine("SECTOR HAS TWO OR MORE TRIGGERS!!!", FONTSTYLE_CONSOLE_WARNING);
                    }
                    sector->trigger->commands = NULL;
                    sector->trigger->function_value = fd_command.function_value;
                    sector->trigger->sub_function = fd_command.sub_function;
                    sector->trigger->mask = fd_trigger_head.mask;
                    sector->trigger->timer = fd_trigger_head.timer;
                    sector->trigger->once = fd_trigger_head.once;

                    // Now parse operand chain for trigger function!
                    fd_trigger_function_t fd_trigger_function;
                    do
                    {
                        trigger_command_p command = (trigger_command_p)malloc(sizeof(trigger_command_t));
                        trigger_command_p *last_command_ptr = &sector->trigger->commands;
                        command->next = NULL;
                        while(*last_command_ptr)
                        {
                            last_command_ptr = &((*last_command_ptr)->next);
                        }
                        *last_command_ptr = command;

                        entry++;
                        current_offset++;
                        fd_trigger_function = *((fd_trigger_function_p)entry);
                        command->function = fd_trigger_function.function;
                        command->operands = fd_trigger_function.operands;
                        command->unused = 0;
                        command->once = 0;
                        command->camera.index = 0;
                        command->camera.timer = 0;
                        command->camera.move = 0;
                        command->camera.unused = 0;

                        switch(command->function)
                        {
                            case TR_FD_TRIGFUNC_SET_CAMERA:
                                {
                                    command->camera.index = (*entry) & 0x007F;
                                    entry++;
                                    current_offset++;
                                    command->camera.timer = ((*entry) & 0x00FF);
                                    command->once         = ((*entry) & 0x0100) >> 8;
                                    command->camera.move  = ((*entry) & 0x1000) >> 12;
                                    fd_trigger_function.cont_bit  = ((*entry) & 0x8000) >> 15;
                                }
                                break;

                            case TR_FD_TRIGFUNC_FLYBY:
                                {
                                    entry++;
                                    current_offset++;
                                    command->once  = ((*entry) & 0x0100) >> 8;
                                    fd_trigger_function.cont_bit  = ((*entry) & 0x8000) >> 15;
                                }
                                break;

                            case TR_FD_TRIGFUNC_OBJECT:
                            case TR_FD_TRIGFUNC_UWCURRENT:
                            case TR_FD_TRIGFUNC_FLIPMAP:
                            case TR_FD_TRIGFUNC_FLIPON:
                            case TR_FD_TRIGFUNC_FLIPOFF:
                            case TR_FD_TRIGFUNC_SET_TARGET:
                            case TR_FD_TRIGFUNC_ENDLEVEL:
                            case TR_FD_TRIGFUNC_PLAYTRACK:
                            case TR_FD_TRIGFUNC_FLIPEFFECT:
                            case TR_FD_TRIGFUNC_SECRET:
                            case TR_FD_TRIGFUNC_CLEARBODIES:
                            case TR_FD_TRIGFUNC_CUTSCENE:
                                break;

                            default: // UNKNOWN!
                                break;
                        };
                    }
                    while(!fd_trigger_function.cont_bit && (current_offset < max_offset));
                }
                break;

            case TR_FD_FUNC_DEATH:
                sector->flags |= SECTOR_FLAG_DEATH;
                break;

            case TR_FD_FUNC_CLIMB:
                // First 4 sector flags are similar to subfunction layout.
                sector->flags |= (uint32_t)fd_command.sub_function;
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
                if( (fd_command.function >= TR_FD_FUNC_FLOORTRIANGLE_NW) &&
                    (fd_command.function <= TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE) )
                {
                    fd_slope_t fd_slope = *((fd_slope_p)entry);
                    float overall_adjustment = (float)Res_Sector_BiggestCorner(fd_slope.slope_t10, fd_slope.slope_t11, fd_slope.slope_t12, fd_slope.slope_t13) * TR_METERING_STEP;
                    entry++;
                    current_offset++;

                    if( (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NW)           ||
                        (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW) ||
                        (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)  )
                    {
                        sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NW;

                        sector->floor_corners[0][2] -= overall_adjustment - ((float)fd_slope.slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= overall_adjustment - ((float)fd_slope.slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= overall_adjustment - ((float)fd_slope.slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= overall_adjustment - ((float)fd_slope.slope_t11 * TR_METERING_STEP);

                        if(fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NE)           ||
                             (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW) ||
                             (fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)  )
                    {
                        sector->floor_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NE;

                        sector->floor_corners[0][2] -= overall_adjustment - ((float)fd_slope.slope_t12 * TR_METERING_STEP);
                        sector->floor_corners[1][2] -= overall_adjustment - ((float)fd_slope.slope_t13 * TR_METERING_STEP);
                        sector->floor_corners[2][2] -= overall_adjustment - ((float)fd_slope.slope_t10 * TR_METERING_STEP);
                        sector->floor_corners[3][2] -= overall_adjustment - ((float)fd_slope.slope_t11 * TR_METERING_STEP);

                        if(fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(fd_command.function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NW)           ||
                             (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW) ||
                             (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)  )
                    {
                        sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NW;

                        sector->ceiling_corners[0][2] += overall_adjustment - (float)(fd_slope.slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1][2] += overall_adjustment - (float)(fd_slope.slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += overall_adjustment - (float)(fd_slope.slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += overall_adjustment - (float)(fd_slope.slope_t12 * TR_METERING_STEP);

                        if(fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_B;
                        }
                        else
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
                        }
                    }
                    else if( (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NE)           ||
                             (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW) ||
                             (fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)  )
                    {
                        sector->ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NE;

                        sector->ceiling_corners[0][2] += overall_adjustment - (float)(fd_slope.slope_t11 * TR_METERING_STEP);
                        sector->ceiling_corners[1][2] += overall_adjustment - (float)(fd_slope.slope_t10 * TR_METERING_STEP);
                        sector->ceiling_corners[2][2] += overall_adjustment - (float)(fd_slope.slope_t13 * TR_METERING_STEP);
                        sector->ceiling_corners[3][2] += overall_adjustment - (float)(fd_slope.slope_t12 * TR_METERING_STEP);

                        if(fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW)
                        {
                            sector->ceiling_penetration_config = TR_PENETRATION_CONFIG_DOOR_VERTICAL_A;
                        }
                        else if(fd_command.function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)
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
    while(!fd_command.end_bit && (current_offset < max_offset));

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

/**   Assign animated texture to a polygon.
  *   While in original TRs we had TexInfo abstraction layer to refer texture,
  *   in OpenTomb we need to re-think animated texture concept to work on a
  *   per-polygon basis. For this, we scan all animated texture lists for
  *   same TexInfo index that is applied to polygon, and if corresponding
  *   animation list is found, we assign it to polygon.
  */
bool Res_SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count)
{
    polygon->anim_id = 0;                           // Reset to 0 by default.

    for(uint32_t i = 0; i < anim_sequences_count; i++)
    {
        for(uint16_t j = 0; j < anim_sequences[i].frames_count; j++)
        {
            if(anim_sequences[i].frame_list[j] == tex_index)
            {
                // If we have found assigned texture ID in animation texture lists,
                // we assign corresponding animation sequence to this polygon,
                // additionally specifying frame offset.
                polygon->anim_id       = i + 1;  // Animation sequence ID.
                polygon->frame_offset  = j;      // Animation frame offset.
                return true;
            }
        }
    }

    return false;   // No such TexInfo found in animation textures lists.
}


static void TR_CopyNormals(const polygon_p polygon, base_mesh_p mesh, const uint16_t *mesh_vertex_indices)
{
    for(int i = 0; i < polygon->vertex_count; i++)
    {
        vec3_copy(polygon->vertices[i].normal, mesh->vertices[mesh_vertex_indices[i]].normal);
    }
}


void TR_AccumulateNormals(tr4_mesh_t *tr_mesh, base_mesh_p mesh, int numCorners, const uint16_t *vertex_indices, polygon_p p)
{
    Polygon_Resize(p, numCorners);

    for(int i = 0; i < numCorners; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, &tr_mesh->vertices[vertex_indices[i]]);
    }
    Polygon_FindNormale(p);

    for(int i = 0; i < numCorners; i++)
    {
        vec3_add(mesh->vertices[vertex_indices[i]].normal, mesh->vertices[vertex_indices[i]].normal, p->plane);
    }
}

void TR_SetupColoredFace(tr4_mesh_t *tr_mesh, VT_Level *tr, const uint16_t *vertex_indices, unsigned color, polygon_p p)
{
    for(int i = 0; i < p->vertex_count; i++)
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
    }
}

void TR_SetupTexturedFace(tr4_mesh_t *tr_mesh, const uint16_t *vertex_indices, polygon_p p)
{
    for(int i = 0; i < p->vertex_count; i++)
    {
        if(tr_mesh->num_lights == tr_mesh->num_vertices)
        {
            p->vertices[i].color[0] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[1] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[2] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            p->vertices[i].color[3] = 1.0f;
        }
        else
        {
            vec4_set_one(p->vertices[i].color);
        }
    }
}

void TR_GenMesh(struct base_mesh_s *mesh, size_t mesh_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count, class bordered_texture_atlas *atlas, class VT_Level *tr)
{
    uint16_t col;
    tr4_mesh_t *tr_mesh;
    tr4_face4_t *face4;
    tr4_face3_t *face3;
    tr4_object_texture_t *tex;
    polygon_p p;
    float n;
    vertex_p vertex;
    const uint32_t tex_mask = (tr->game_version == TR_IV) ? (TR_TEXTURE_INDEX_MASK_TR4) : (TR_TEXTURE_INDEX_MASK);

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
    mesh->radius = tr_mesh->collision_size;
    TR_vertex_to_arr(mesh->centre, &tr_mesh->centre);

    mesh->vertex_count = tr_mesh->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(uint32_t i = 0; i < mesh->vertex_count; i++, vertex++)
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
    for(int16_t i = 0; i < tr_mesh->num_textured_triangles; i++, p++)
    {
        face3 = &tr_mesh->textured_triangles[i];
        tex = &tr->object_textures[face3->texture & tex_mask];

        p->double_side = (face3->texture >> 15) ? (0x01) : (0x00);              // CORRECT, BUT WRONG IN TR3-5

        Res_SetAnimTexture(p, face3->texture & tex_mask, anim_sequences, anim_sequences_count);

        if(face3->lighting & 0x01)
        {
            p->transparency = BM_MULTIPLY;
        }
        else
        {
            p->transparency = tex->transparency_flags;
        }

        TR_AccumulateNormals(tr_mesh, mesh, 3, face3->vertices, p);
        TR_SetupTexturedFace(tr_mesh, face3->vertices, p);

        atlas->getCoordinates(p, face3->texture & tex_mask, 0);
    }

    /*
     * coloured triangles
     */
    for(int16_t i = 0; i < tr_mesh->num_coloured_triangles; i++, p++)
    {
        face3 = &tr_mesh->coloured_triangles[i];
        col = face3->texture & 0xff;
        p->texture_index = 0;
        p->transparency = 0;
        p->anim_id = 0;

        TR_AccumulateNormals(tr_mesh, mesh, 3, face3->vertices, p);
        atlas->getWhiteTextureCoordinates(p);
        TR_SetupColoredFace(tr_mesh, tr, face3->vertices, col, p);
    }

    /*
     * textured rectangles
     */
    for(int16_t i = 0; i < tr_mesh->num_textured_rectangles; i++, p++)
    {
        face4 = &tr_mesh->textured_rectangles[i];
        tex = &tr->object_textures[face4->texture & tex_mask];

        p->double_side = (face4->texture >> 15) ? (0x01) : (0x00);              // CORRECT, BUT WRONG IN TR3-5

        Res_SetAnimTexture(p, face4->texture & tex_mask, anim_sequences, anim_sequences_count);

        if(face4->lighting & 0x01)
        {
            p->transparency = BM_MULTIPLY;
        }
        else
        {
            p->transparency = tex->transparency_flags;
        }

        TR_AccumulateNormals(tr_mesh, mesh, 4, face4->vertices, p);
        TR_SetupTexturedFace(tr_mesh, face4->vertices, p);

        atlas->getCoordinates(p, face4->texture & tex_mask, 0);
    }

    /*
     * coloured rectangles
     */
    for(int16_t i = 0; i < tr_mesh->num_coloured_rectangles; i++, p++)
    {
        face4 = &tr_mesh->coloured_rectangles[i];
        col = face4->texture & 0xff;
        Polygon_Resize(p, 4);
        p->texture_index = 0;
        p->transparency = 0;
        p->anim_id = 0;

        TR_AccumulateNormals(tr_mesh, mesh, 4, face4->vertices, p);
        atlas->getWhiteTextureCoordinates(p);
        TR_SetupColoredFace(tr_mesh, tr, face4->vertices, col, p);
    }

    /*
     * let us normalise normales %)
     */
    p = mesh->polygons;
    vertex_p v = mesh->vertices;
    for(uint32_t i = 0; i < mesh->vertex_count; i++, v++)
    {
        vec3_norm(v->normal, n);
    }

    /*
     * triangles
     */
    for(int16_t i = 0; i < tr_mesh->num_textured_triangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_mesh->textured_triangles[i].vertices);
    }

    for(int16_t i = 0; i < tr_mesh->num_coloured_triangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_mesh->coloured_triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(int16_t i = 0; i < tr_mesh->num_textured_rectangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_mesh->textured_rectangles[i].vertices);
    }

    for(int16_t i = 0; i < tr_mesh->num_coloured_rectangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_mesh->coloured_rectangles[i].vertices);
    }

    if(mesh->vertex_count > 0)
    {
        mesh->vertex_count = 0;
        free(mesh->vertices);
        mesh->vertices = NULL;
    }

    p = mesh->polygons;
    for(uint32_t i = 0; i < mesh->polygons_count; i++, p++)
    {
        if((p->anim_id > 0) && (p->anim_id <= anim_sequences_count))
        {
            anim_seq_p seq = anim_sequences + (p->anim_id - 1);
            // set tex coordinates to the first frame for correct texture transform in renderer
            atlas->getCoordinates(p, seq->frame_list[0], false, 0, seq->uvrotate);
        }
    }
}

void TR_SetupRoomPolygonVertices(polygon_p p, base_mesh_p mesh, const tr5_room_t *tr_room, const uint16_t *vertices)
{
    for (int i = 0; i < p->vertex_count; i++)
    {
        TR_vertex_to_arr(p->vertices[i].position, &tr_room->vertices[vertices[i]].vertex);
    }
    Polygon_FindNormale(p);

    for (int i = 0; i < p->vertex_count; i++)
    {
        vec3_add(mesh->vertices[vertices[i]].normal, mesh->vertices[vertices[i]].normal, p->plane);
        vec3_copy(p->vertices[i].normal, p->plane);
        TR_color_to_arr(p->vertices[i].color, &tr_room->vertices[vertices[i]].colour);
    }
}

void TR_GenRoomMesh(struct room_s *room, size_t room_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count, class bordered_texture_atlas *atlas, class VT_Level *tr)
{
    tr5_room_t *tr_room;
    polygon_p p;
    base_mesh_p mesh;
    float n;
    vertex_p vertex;
    uint32_t tex_mask = (tr->game_version == TR_IV) ? (TR_TEXTURE_INDEX_MASK_TR4) : (TR_TEXTURE_INDEX_MASK);

    tr_room = &tr->rooms[room_index];

    if(tr_room->num_triangles + tr_room->num_rectangles == 0)
    {
        room->content->mesh = NULL;
        return;
    }

    mesh = room->content->mesh = (base_mesh_p)calloc(1, sizeof(base_mesh_t));
    mesh->id = room_index;

    mesh->vertex_count = tr_room->num_vertices;
    vertex = mesh->vertices = (vertex_p)calloc(mesh->vertex_count, sizeof(vertex_t));
    for(uint32_t i = 0; i < mesh->vertex_count; i++, vertex++)
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
    for(uint32_t i = 0; i < tr_room->num_triangles; i++, p++)
    {
        uint32_t masked_texture = tr_room->triangles[i].texture & tex_mask;
        Polygon_Resize(p, 3);
        TR_SetupRoomPolygonVertices(p, mesh, tr_room, tr_room->triangles[i].vertices);
        p->double_side = (tr_room->triangles[i].texture & 0x8000) ? (0x01) : (0x00);
        p->transparency = tr->object_textures[masked_texture].transparency_flags;
        Res_SetAnimTexture(p, masked_texture, anim_sequences, anim_sequences_count);
        atlas->getCoordinates(p, masked_texture, 0);
    }

    /*
     * rectangles
     */
    for(uint32_t i = 0; i < tr_room->num_rectangles; i++, p++)
    {
        uint32_t masked_texture = tr_room->rectangles[i].texture & tex_mask;
        Polygon_Resize(p, 4);
        TR_SetupRoomPolygonVertices(p, mesh, tr_room, tr_room->rectangles[i].vertices);
        p->double_side = (tr_room->rectangles[i].texture & 0x8000) ? (0x01) : (0x00);
        p->transparency = tr->object_textures[masked_texture].transparency_flags;
        Res_SetAnimTexture(p, masked_texture, anim_sequences, anim_sequences_count);
        atlas->getCoordinates(p, masked_texture, 0);
    }

    /*
     * let us normalise normales %)
     */
    vertex_p v = mesh->vertices;
    for(uint32_t i = 0; i < mesh->vertex_count; i++, v++)
    {
        vec3_norm(v->normal, n);
    }

    /*
     * triangles
     */
    p = mesh->polygons;
    for(uint32_t i = 0; i < tr_room->num_triangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_room->triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(uint32_t i = 0; i < tr_room->num_rectangles; i++, p++)
    {
        TR_CopyNormals(p, mesh, tr_room->rectangles[i].vertices);
    }

    if(mesh->vertex_count > 0)
    {
        mesh->vertex_count = 0;
        free(mesh->vertices);
        mesh->vertices = NULL;
    }

    p = mesh->polygons;
    for(uint32_t i = 0; i < mesh->polygons_count; i++, p++)
    {
        if((p->anim_id > 0) && (p->anim_id <= anim_sequences_count))
        {
            anim_seq_p seq = anim_sequences + (p->anim_id - 1);
            // set tex coordinates to the first frame for correct texture transform in renderer
            atlas->getCoordinates(p, seq->frame_list[0], false, 0, seq->uvrotate);
        }
    }
}


void TR_SkeletalModelInterpolateFrames(skeletal_model_p model, tr_animation_t *tr_animations)
{
    uint16_t new_frames_count;
    animation_frame_p anim = model->animations;
    bone_frame_p bf, new_bone_frames;
    float lerp, t;

    for(uint16_t i = 0; i < model->animation_count; i++, anim++)
    {
        tr_animation_t *tr_anim = tr_animations + i;
        if(anim->frames_count > 1 && tr_anim->frame_rate > 1)                   // we can't interpolate one frame or rate < 2!
        {
            new_frames_count = (uint16_t)tr_anim->frame_rate * (anim->frames_count - 1) + 1;
            bf = new_bone_frames = (bone_frame_p)malloc(new_frames_count * sizeof(bone_frame_t));

            /*
             * the first frame does not changes
             */
            bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            bf->bone_tag_count = model->mesh_count;
            vec3_set_zero(bf->pos);
            vec3_copy(bf->centre, anim->frames[0].centre);
            vec3_copy(bf->pos, anim->frames[0].pos);
            vec3_copy(bf->bb_max, anim->frames[0].bb_max);
            vec3_copy(bf->bb_min, anim->frames[0].bb_min);
            for(uint16_t k = 0; k < model->mesh_count; k++)
            {
                vec3_copy(bf->bone_tags[k].offset, anim->frames[0].bone_tags[k].offset);
                vec4_copy(bf->bone_tags[k].qrotate, anim->frames[0].bone_tags[k].qrotate);
            }
            bf++;

            for(uint16_t j = 1; j < anim->frames_count; j++)
            {
                for(uint16_t lerp_index = 1; lerp_index <= tr_anim->frame_rate; lerp_index++)
                {
                    vec3_set_zero(bf->pos);
                    lerp = ((float)lerp_index) / (float)tr_anim->frame_rate;
                    t = 1.0f - lerp;

                    bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
                    bf->bone_tag_count = model->mesh_count;

                    bf->centre[0] = t * anim->frames[j-1].centre[0] + lerp * anim->frames[j].centre[0];
                    bf->centre[1] = t * anim->frames[j-1].centre[1] + lerp * anim->frames[j].centre[1];
                    bf->centre[2] = t * anim->frames[j-1].centre[2] + lerp * anim->frames[j].centre[2];

                    bf->pos[0] = t * anim->frames[j-1].pos[0] + lerp * anim->frames[j].pos[0];
                    bf->pos[1] = t * anim->frames[j-1].pos[1] + lerp * anim->frames[j].pos[1];
                    bf->pos[2] = t * anim->frames[j-1].pos[2] + lerp * anim->frames[j].pos[2];

                    bf->bb_max[0] = t * anim->frames[j-1].bb_max[0] + lerp * anim->frames[j].bb_max[0];
                    bf->bb_max[1] = t * anim->frames[j-1].bb_max[1] + lerp * anim->frames[j].bb_max[1];
                    bf->bb_max[2] = t * anim->frames[j-1].bb_max[2] + lerp * anim->frames[j].bb_max[2];

                    bf->bb_min[0] = t * anim->frames[j-1].bb_min[0] + lerp * anim->frames[j].bb_min[0];
                    bf->bb_min[1] = t * anim->frames[j-1].bb_min[1] + lerp * anim->frames[j].bb_min[1];
                    bf->bb_min[2] = t * anim->frames[j-1].bb_min[2] + lerp * anim->frames[j].bb_min[2];

                    for(uint16_t k = 0; k < model->mesh_count; k++)
                    {
                        bf->bone_tags[k].offset[0] = t * anim->frames[j-1].bone_tags[k].offset[0] + lerp * anim->frames[j].bone_tags[k].offset[0];
                        bf->bone_tags[k].offset[1] = t * anim->frames[j-1].bone_tags[k].offset[1] + lerp * anim->frames[j].bone_tags[k].offset[1];
                        bf->bone_tags[k].offset[2] = t * anim->frames[j-1].bone_tags[k].offset[2] + lerp * anim->frames[j].bone_tags[k].offset[2];

                        vec4_slerp(bf->bone_tags[k].qrotate, anim->frames[j-1].bone_tags[k].qrotate, anim->frames[j].bone_tags[k].qrotate, lerp);
                    }
                    bf++;
                }
            }

            /*
             * swap old and new animation bone frames
             * free old bone frames;
             */
            for(uint16_t j = 0; j < anim->frames_count; j++)
            {
                if(anim->frames[j].bone_tag_count)
                {
                    anim->frames[j].bone_tag_count = 0;
                    free(anim->frames[j].bone_tags);
                    anim->frames[j].bone_tags = NULL;
                }
            }
            free(anim->frames);
            anim->frames = new_bone_frames;
            anim->frames_count = new_frames_count;
        }
        if(anim->max_frame > anim->frames_count || anim->max_frame == 0)
        {
            anim->max_frame = anim->frames_count;                               // i.e.: unused animations
        }
    }
}


void TR_GenSkeletalModel(struct skeletal_model_s *model, size_t model_id, struct base_mesh_s *base_mesh_array, class VT_Level *tr)
{
    tr_moveable_t *tr_moveable = &tr->moveables[model_id];
    tr5_vertex_t *rotations;
    tr5_vertex_t min_max_pos[3];
    float rot[3];
    bone_tag_p bone_tag;
    bone_frame_p bone_frame;
    mesh_tree_tag_p tree_tag;
    animation_frame_p anim;

    model->collision_map = (uint16_t*)malloc(model->mesh_count * sizeof(uint16_t));
    model->mesh_tree = (mesh_tree_tag_p)calloc(model->mesh_count, sizeof(mesh_tree_tag_t));
    tree_tag = model->mesh_tree;

    uint32_t *mesh_index = tr->mesh_indices + tr_moveable->starting_mesh;

    for(uint16_t k = 0; k < model->mesh_count; k++, tree_tag++)
    {
        tr_mesh_thee_tag_t mtt = tr->get_mesh_tree_tag_for_model(tr_moveable, k);
        model->collision_map[k] = k;
        tree_tag->mesh_base = base_mesh_array + mesh_index[k];
        tree_tag->replace_anim = 0x00;
        tree_tag->replace_mesh = 0x00;
        tree_tag->body_part    = 0x00;
        tree_tag->parent = 0;
        tree_tag->offset[0] = mtt.dx;
        tree_tag->offset[1] = mtt.dz;
        tree_tag->offset[2] =-mtt.dy;
        tree_tag->flag = mtt.flag_data & 0xFF;
    }

    SkeletalModel_GenParentsIndexes(model);

    /*
     * =================    now, animation loading    ========================
     */

    if(tr_moveable->animation_index >= tr->animations_count)
    {
        /*
         * model has no start offset and any animation
         */
        model->animation_count = 1;
        model->animations = (animation_frame_p)malloc(sizeof(animation_frame_t));
        model->animations->frames_count = 1;
        model->animations->max_frame = 1;
        model->animations->frames = (bone_frame_p)calloc(model->animations->frames_count , sizeof(bone_frame_t));
        bone_frame = model->animations->frames;

        model->animations->id = 0;
        model->animations->next_anim = model->animations;
        model->animations->next_frame = 0;
        model->animations->state_change = NULL;
        model->animations->state_change_count = 0;
        model->animations->commands = NULL;
        model->animations->effects = NULL;
        bone_frame->bone_tag_count = model->mesh_count;
        bone_frame->bone_tags = (bone_tag_p)malloc(bone_frame->bone_tag_count * sizeof(bone_tag_t));
        vec3_set_zero(bone_frame->pos);

        rot[0] = 0.0f;
        rot[1] = 0.0f;
        rot[2] = 0.0f;
        for(uint16_t k = 0; k < bone_frame->bone_tag_count; k++)
        {
            tree_tag = model->mesh_tree + k;
            bone_tag = bone_frame->bone_tags + k;
            vec4_SetZXYRotations(bone_tag->qrotate, rot);
            vec3_copy(bone_tag->offset, tree_tag->offset);
        }
        return;
    }
    //Sys_DebugLog(LOG_FILENAME, "model = %d, anims = %d", tr_moveable->object_id, GetNumAnimationsForMoveable(tr, model_num));
    model->animation_count = TR_GetNumAnimationsForMoveable(tr, model_id);
    if(model->animation_count <= 0)
    {
        /*
         * the animation count must be >= 1
         */
        model->animation_count = 1;
    }

    model->animations = (animation_frame_p)calloc(model->animation_count, sizeof(animation_frame_t));
    anim = model->animations;
    for(uint16_t i = 0; i < model->animation_count; i++, anim++)
    {
        tr_animation_t *tr_animation = &tr->animations[tr_moveable->animation_index + i];

        anim->id = i;
        anim->speed_x = tr_animation->speed;
        anim->accel_x = tr_animation->accel;
        anim->speed_y = tr_animation->accel_lateral;
        anim->accel_y = tr_animation->speed_lateral;
        anim->commands = NULL;
        anim->effects = NULL;
        anim->state_id = tr_animation->state_id;

        anim->max_frame = tr_animation->frame_end - tr_animation->frame_start + 1;
        anim->frames_count = TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i);

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.

        if((tr_animation->num_anim_commands > 0) && (tr_animation->num_anim_commands <= 255))
        {
            // Calculate current animation anim command block offset.
            int16_t *pointer = tr->anim_commands + tr_animation->anim_command;
            animation_command_t command;
            animation_effect_t effect;
            for(uint32_t count = 0; count < tr_animation->num_anim_commands; count++, pointer++)
            {
                command.id = *pointer;
                command.frame = -1;
                vec3_set_zero(command.data);
                effect.frame = -1;
                effect.data = 0;
                switch(command.id)
                {
                    case TR_ANIMCOMMAND_SETPOSITION:
                        command.data[0] = (float)(*++pointer);     // x = x;
                        command.data[2] =-(float)(*++pointer);     // z =-y
                        command.data[1] = (float)(*++pointer);     // y = z
                        Anim_AddCommand(anim, &command);
                        break;

                    case TR_ANIMCOMMAND_JUMPDISTANCE:
                        command.data[0] = (float)(*++pointer);     // v_z
                        command.data[1] = (float)(*++pointer);     // v_y
                        command.data[2] = 0.0f;
                        Anim_AddCommand(anim, &command);
                        break;
                        
                    case TR_ANIMCOMMAND_PLAYEFFECT:
                    case TR_ANIMCOMMAND_PLAYSOUND:
                        effect.frame = *(++pointer) - tr_animation->frame_start;
                        effect.data = *(++pointer);
                    case TR_ANIMCOMMAND_KILL:
                    case TR_ANIMCOMMAND_EMPTYHANDS:
                    default:
                        effect.id = command.id;
                        effect.extra = 0x0000;
                        Anim_AddEffect(anim, &effect);
                        break;
                };
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
        rotations = (tr5_vertex_t*)Sys_GetTempMem(model->mesh_count * sizeof(tr5_vertex_t));
        for(uint16_t frame_index = 0; frame_index < anim->frames_count; frame_index++, bone_frame++)
        {
            bone_frame->bone_tag_count = model->mesh_count;
            bone_frame->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            tr->get_anim_frame_data(min_max_pos, rotations, bone_frame->bone_tag_count, tr_animation, frame_index);

            bone_frame->bb_min[0] = min_max_pos[0].x;
            bone_frame->bb_min[1] = min_max_pos[0].z;
            bone_frame->bb_min[2] =-min_max_pos[1].y;

            bone_frame->bb_max[0] = min_max_pos[1].x;
            bone_frame->bb_max[1] = min_max_pos[1].z;
            bone_frame->bb_max[2] =-min_max_pos[0].y;

            bone_frame->pos[0] = min_max_pos[2].x;
            bone_frame->pos[1] = min_max_pos[2].z;
            bone_frame->pos[2] =-min_max_pos[2].y;

            bone_frame->centre[0] = 0.5f * (bone_frame->bb_min[0] + bone_frame->bb_max[0]);
            bone_frame->centre[1] = 0.5f * (bone_frame->bb_min[1] + bone_frame->bb_max[1]);
            bone_frame->centre[2] = 0.5f * (bone_frame->bb_min[2] + bone_frame->bb_max[2]);

            for(uint16_t k = 0; k < bone_frame->bone_tag_count; k++)
            {
                tree_tag = model->mesh_tree + k;
                bone_tag = bone_frame->bone_tags + k;
                rot[0] = rotations[k].x;
                rot[1] = rotations[k].z;
                rot[2] =-rotations[k].y;
                vec4_SetZXYRotations(bone_tag->qrotate, rot);
                vec3_copy(bone_tag->offset, tree_tag->offset);
            }
        }
    }
    Sys_ReturnTempMem(model->mesh_count * sizeof(tr5_vertex_t));
    /*
     * Animations interpolation to 1/30 sec like in original. Needed for correct state change works.
     */
    TR_SkeletalModelInterpolateFrames(model, tr->animations + tr_moveable->animation_index);
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
    for(uint16_t i = 0; i < model->animation_count; i++, anim++)
    {
        anim->state_change_count = 0;
        anim->state_change = NULL;
        anim->next_anim = anim;
        anim->next_frame = 0;
        tr_animation_t *tr_animation = &tr->animations[tr_moveable->animation_index + i];
        int16_t j = tr_animation->next_animation - tr_moveable->animation_index;
        j &= 0x7fff;
        if((j >= 0) && (j < model->animation_count))
        {
            anim->next_anim = model->animations + j;
            anim->next_frame = tr_animation->next_frame - tr->animations[tr_animation->next_animation].frame_start;
            anim->next_frame %= anim->next_anim->max_frame;
            if(anim->next_frame < 0)
            {
                anim->next_frame = 0;
            }
#if LOG_ANIM_DISPATCHES
            Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, anim->next_anim->id, anim->next_frame);
#endif
        }

        anim->state_change_count = 0;
        anim->state_change = NULL;

        if((tr_animation->num_state_changes > 0) && (model->animation_count > 1))
        {
            state_change_p sch_p;
#if LOG_ANIM_DISPATCHES
            Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, (anim->next_anim) ? (anim->next_anim->id) : (-1), anim->next_frame);
#endif
            anim->state_change_count = tr_animation->num_state_changes;
            sch_p = anim->state_change = (state_change_p)malloc(tr_animation->num_state_changes * sizeof(state_change_t));

            for(uint16_t j = 0;j < tr_animation->num_state_changes; j++, sch_p++)
            {
                tr_state_change_t *tr_sch;
                tr_sch = &tr->state_changes[j+tr_animation->state_change_offset];
                sch_p->id = tr_sch->state_id;
                sch_p->anim_dispatch = NULL;
                sch_p->anim_dispatch_count = 0;
                for(uint16_t l = 0; l < tr_sch->num_anim_dispatches; l++)
                {
                    tr_anim_dispatch_t *tr_adisp = &tr->anim_dispatches[tr_sch->anim_dispatch+l];
                    uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                    if(next_anim_ind < model->animation_count)
                    {
                        sch_p->anim_dispatch_count++;
                        sch_p->anim_dispatch = (anim_dispatch_p)realloc(sch_p->anim_dispatch, sch_p->anim_dispatch_count * sizeof(anim_dispatch_t));

                        anim_dispatch_p adsp = sch_p->anim_dispatch + sch_p->anim_dispatch_count - 1;
                        uint16_t next_max_frame = model->animations[next_anim - tr_moveable->animation_index].max_frame;
                        uint16_t next_frame = tr_adisp->next_frame - tr->animations[next_anim].frame_start;

                        uint16_t low  = tr_adisp->low  - tr_animation->frame_start;
                        uint16_t high = tr_adisp->high - tr_animation->frame_start;

                        adsp->frame_low  = low  % anim->max_frame;
                        adsp->frame_high = (high - 1) % anim->max_frame;
                        adsp->next_anim = next_anim - tr_moveable->animation_index;
                        adsp->next_frame = next_frame % next_max_frame;

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


int32_t TR_GetNumAnimationsForMoveable(class VT_Level *tr, size_t moveable_ind)
{
    tr_moveable_t *curr_moveable = &tr->moveables[moveable_ind];
    if(curr_moveable->animation_index != 0xFFFF)
    {
        uint32_t next_anim_index = tr->animations_count;
        for(uint32_t movable_it = moveable_ind + 1; movable_it < tr->moveables_count; ++movable_it)
        {
            if(tr->moveables[movable_it].animation_index != 0xFFFF)
            {
                next_anim_index = tr->moveables[movable_it].animation_index;
                break;
            }
        }
        return next_anim_index - curr_moveable->animation_index;
    }

    return 0;
}


int TR_GetNumFramesForAnimation(class VT_Level *tr, size_t animation_ind)
{
    tr_animation_t *curr_anim = &tr->animations[animation_ind];
    if(curr_anim->frame_size > 0)
    {
        uint32_t next_frame_offset = 2 * tr->frame_data_size;
        if(animation_ind < tr->animations_count - 1)
        {
            next_frame_offset = (curr_anim + 1)->frame_offset;
        }
        return (next_frame_offset - curr_anim->frame_offset) / (curr_anim->frame_size * 2);
    }

    return 1;
}
