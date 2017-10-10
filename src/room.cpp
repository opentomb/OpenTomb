
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "core/gl_util.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/system.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/frustum.h"
#include "physics/physics.h"
#include "engine.h"
#include "entity.h"
#include "mesh.h"
#include "trigger.h"
#include "room.h"
#include "world.h"


#define ROOM_LIST_SIZE_ALIGN    (8)


void Room_Clear(struct room_s *room)
{
    if(!room)
    {
        return;
    }

    room->containers = NULL;
    room->content = NULL;
    room->frustum = NULL;

    if(room->original_content)
    {
        room_content_p content = room->original_content;
        portal_p p = content->portals;
        if(content->portals_count)
        {
            for(uint16_t i = 0; i < content->portals_count; i++, p++)
            {
                Portal_Clear(p);
            }
            free(content->portals);
            content->portals = NULL;
            content->portals_count = 0;
        }

        if(content->mesh)
        {
            BaseMesh_Clear(content->mesh);
            free(content->mesh);
            content->mesh = NULL;
        }

        if(content->static_mesh_count)
        {
            for(uint32_t i = 0; i < content->static_mesh_count; i++)
            {
                Physics_DeleteObject(content->static_mesh[i].physics_body);
                content->static_mesh[i].physics_body = NULL;

                OBB_Delete(content->static_mesh[i].obb);
                content->static_mesh[i].obb = NULL;
                if(content->static_mesh[i].self)
                {
                    content->static_mesh[i].self->room = NULL;
                    Container_Delete(content->static_mesh[i].self);
                    content->static_mesh[i].self = NULL;
                }
            }
            free(content->static_mesh);
            content->static_mesh = NULL;
            content->static_mesh_count = 0;
        }

        Physics_DeleteObject(content->physics_body);
        content->physics_body = NULL;
        Physics_DeleteObject(content->physics_alt_tween);
        content->physics_alt_tween = NULL;

        if(content->sprites_count)
        {
            free(content->sprites);
            content->sprites = NULL;
            content->sprites_count = 0;
        }

        if(content->sprites_vertices)
        {
            free(content->sprites_vertices);
            content->sprites_vertices = NULL;
        }

        if(content->lights_count)
        {
            free(content->lights);
            content->lights = NULL;
            content->lights_count = 0;
        }

        if(room->sectors_count)
        {
            room_sector_p s = content->sectors;
            for(uint32_t i = 0; i < room->sectors_count; i++, s++)
            {
                if(s->trigger)
                {
                    for(trigger_command_p current_command = s->trigger->commands; current_command; )
                    {
                        trigger_command_p next_command = current_command->next;
                        current_command->next = NULL;
                        free(current_command);
                        current_command = next_command;
                    }
                    free(s->trigger);
                    s->trigger = NULL;
                }
            }
            free(content->sectors);
            content->sectors = NULL;
            room->sectors_count = 0;
            room->sectors_x = 0;
            room->sectors_y = 0;
        }

        if(content->overlapped_room_list)
        {
            content->overlapped_room_list_size = 0;
            free(content->overlapped_room_list);
            content->overlapped_room_list = NULL;
        }

        if(content->near_room_list)
        {
            content->near_room_list_size = 0;
            free(content->near_room_list);
            content->near_room_list = NULL;
        }

        free(content);
    }
    room->original_content = NULL;

    if(room->obb)
    {
        OBB_Delete(room->obb);
        room->obb = NULL;
    }

    if(room->self)
    {
        room->self->room = NULL;
        Container_Delete(room->self);
        room->self = NULL;
    }
}


void Room_Enable(struct room_s *room)
{
    if(room->content->physics_body != NULL)
    {
        Physics_EnableObject(room->content->physics_body);
    }

    if(room->content->physics_alt_tween)
    {
        Physics_DisableObject(room->content->physics_alt_tween);
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(room->content->static_mesh[i].physics_body != NULL)
        {
            Physics_EnableObject(room->content->static_mesh[i].physics_body);
        }
    }

    for(engine_container_p cont = room->containers; cont; cont = cont->next)
    {
        if(cont->collision_group == COLLISION_NONE)
        {
            continue;
        }
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                if(((entity_p)cont->object)->state_flags & ENTITY_STATE_ENABLED)
                {
                    Physics_EnableCollision(((entity_p)cont->object)->physics);
                }
                break;
        }
    }
}


void Room_Disable(struct room_s *room)
{
    if(room->content->physics_body)
    {
        Physics_DisableObject(room->content->physics_body);
    }

    if(room->content->physics_alt_tween)
    {
        Physics_DisableObject(room->content->physics_alt_tween);
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(room->content->static_mesh[i].physics_body)
        {
            Physics_DisableObject(room->content->static_mesh[i].physics_body);
        }
    }

    for(engine_container_p cont = room->containers; cont; cont = cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                if(((entity_p)cont->object)->state_flags & ENTITY_STATE_ENABLED)
                {
                    Physics_DisableCollision(((entity_p)cont->object)->physics);
                }
                break;
        }
    }
}


int  Room_AddObject(struct room_s *room, struct engine_container_s *cont)
{
    engine_container_p curr = room->containers;

    for(; curr; curr = curr->next)
    {
        if(curr == cont)
        {
            return 0;
        }
    }

    cont->room = room;
    cont->next = room->containers;
    room->containers = cont;
    return 1;
}


int  Room_RemoveObject(struct room_s *room, struct engine_container_s *cont)
{
    engine_container_p previous_cont, current_cont;

    if(!room || !room->containers)
    {
        return 0;
    }

    if(room->containers == cont)
    {
        room->containers = cont->next;
        cont->room = NULL;
        return 1;
    }

    previous_cont = room->containers;
    current_cont = previous_cont->next;
    while(current_cont)
    {
        if(current_cont == cont)
        {
            previous_cont->next = current_cont->next;
            cont->room = NULL;
            return 1;
        }

        previous_cont = current_cont;
        current_cont = current_cont->next;
    }

    return 0;
}


void Room_SetActiveContent(struct room_s *room, struct room_s *room_with_content_from)
{
    engine_container_p cont = room->containers;
    room->containers = NULL;
    room->content = room_with_content_from->original_content;
    Physics_SetOwnerObject(room->content->physics_body, room->self);
    Physics_SetOwnerObject(room->content->physics_alt_tween, room->self);

    for(uint32_t i = 0; i < room->content->static_mesh_count; ++i)
    {
        room->content->static_mesh[i].self->room = room;
    }

    for(uint32_t i = 0; i < room->sectors_count; ++i)
    {
        room->content->sectors[i].owner_room = room;
    }

    if(room == room->real_room)
    {
        Room_Enable(room);
    }
    else
    {
        Room_Disable(room);
    }

    room->containers = cont;
}


void Room_DoFlip(struct room_s *room1, struct room_s *room2)
{
    if(room1 && room2 && (room1 != room2))
    {
        room1->frustum = NULL;
        room2->frustum = NULL;

        // swap content
        {
            room_content_p t = room1->content;
            room1->content = room2->content;
            room2->content = t;

            // fix physics
            Physics_SetOwnerObject(room1->content->physics_body, room1->self);
            Physics_SetOwnerObject(room1->content->physics_alt_tween, room1->self);
            Physics_SetOwnerObject(room2->content->physics_body, room2->self);
            Physics_SetOwnerObject(room2->content->physics_alt_tween, room2->self);

            // fix static meshes
            for(uint32_t i = 0; i < room1->content->static_mesh_count; ++i)
            {
                room1->content->static_mesh[i].self->room = room1;
            }
            for(uint32_t i = 0; i < room2->content->static_mesh_count; ++i)
            {
                room2->content->static_mesh[i].self->room = room2;
            }

            // update enability if it is necessary
            {
                room_p base_room = (room1 == room1->real_room) ? room1 : NULL;
                base_room = (room2 == room2->real_room) ? room2 : room1;
                if(base_room)
                {
                    engine_container_p cont = base_room->containers;
                    base_room->containers = NULL;
                    room_p alt_room = (room1 == base_room) ? room2 : room1;
                    Room_Disable(alt_room);
                    Room_Enable(base_room);                 // enable new collisions
                    base_room->containers = cont;
                }
            }

            // fix sectors ownership
            for(uint32_t i = 0; i < room1->sectors_count; ++i)
            {
                room1->content->sectors[i].owner_room = room1;
            }

            for(uint32_t i = 0; i < room2->sectors_count; ++i)
            {
                room2->content->sectors[i].owner_room = room2;
            }
        }
    }
}


struct room_sector_s *Room_GetSectorRaw(struct room_s *room, float pos[3])
{
    if(room)
    {
        int x = (int)(pos[0] - room->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int y = (int)(pos[1] - room->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
        {
            return NULL;
        }
        /*
         * column index system
         * X - column number, Y - string number
         */
        return room->content->sectors + x * room->sectors_y + y;
    }

    return NULL;
}


struct room_sector_s *Room_GetSectorXYZ(struct room_s *room, float pos[3])
{
    room_sector_p ret = NULL;
    int x = (int)(pos[0] - room->transform[12 + 0]) / TR_METERING_SECTORSIZE;
    int y = (int)(pos[1] - room->transform[12 + 1]) / TR_METERING_SECTORSIZE;

    room = room->real_room;

    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return NULL;
    }
    /*
     * column index system
     * X - column number, Y - string number
     */
    ret = room->content->sectors + x * room->sectors_y + y;

    /*
     * resolve Z overlapped neighboard rooms. room below has more priority.
     */
    if(ret->room_below && (pos[2] < ret->floor))
    {
        ret = Room_GetSectorRaw(ret->room_below->real_room, ret->pos);
    }

    if(ret->room_above && (pos[2] > ret->ceiling))
    {
        ret = Room_GetSectorRaw(ret->room_above->real_room, ret->pos);
    }

    return ret;
}


void Room_AddToNearRoomsList(struct room_s *room, struct room_s *r)
{
    if(room && r && (r->real_room->id != room->real_room->id) &&
       (room->bb_min[0] <= r->bb_max[0] && room->bb_max[0] >= r->bb_min[0]) &&
       (room->bb_min[1] <= r->bb_max[1] && room->bb_max[1] >= r->bb_min[1]))
    {
        for(uint32_t i = 0; i < room->content->near_room_list_size; ++i)
        {
            if(room->content->near_room_list[i]->id == r->id)
            {
                return;
            }
        }

        if(!Room_IsOverlapped(room, r))
        {
            if(!room->content->near_room_list)
            {
                room->content->near_room_list = (room_p*)malloc(ROOM_LIST_SIZE_ALIGN * sizeof(room_p));
            }
            else if((room->content->near_room_list_size + 1) % ROOM_LIST_SIZE_ALIGN == 0)
            {
                room_p *old_list = room->content->near_room_list;
                uint16_t rooms_count = room->content->near_room_list_size + 1 + ROOM_LIST_SIZE_ALIGN;
                room->content->near_room_list = (room_p*)malloc(rooms_count * sizeof(room_p));
                memcpy(room->content->near_room_list, old_list, room->content->near_room_list_size * sizeof(room_p));
                free(old_list);
            }
            room->content->near_room_list[room->content->near_room_list_size++] = r->real_room;
        }
    }
}


void Room_AddToOverlappedRoomsList(struct room_s *room, struct room_s *r)
{
    for(uint32_t i = 0; i < room->content->overlapped_room_list_size; ++i)
    {
        if(room->content->overlapped_room_list[i]->id == r->id)
        {
            return;
        }
    }

    if(!room->content->overlapped_room_list)
    {
        room->content->overlapped_room_list = (room_p*)malloc(ROOM_LIST_SIZE_ALIGN * sizeof(room_p));
    }
    else if((room->content->overlapped_room_list_size + 1) % ROOM_LIST_SIZE_ALIGN == 0)
    {
        room_p *old_list = room->content->overlapped_room_list;
        uint16_t rooms_count = room->content->overlapped_room_list_size + 1 + ROOM_LIST_SIZE_ALIGN;
        room->content->overlapped_room_list = (room_p*)malloc(rooms_count * sizeof(room_p));
        memcpy(room->content->overlapped_room_list, old_list, room->content->overlapped_room_list_size * sizeof(room_p));
        free(old_list);
    }
    room->content->overlapped_room_list[room->content->overlapped_room_list_size++] = r->real_room;
}


int Room_IsJoined(struct room_s *r1, struct room_s *r2)
{
    room_sector_p rs = r1->content->sectors;
    for(uint32_t i = 0; i < r1->sectors_count; i++, rs++)
    {
        if((rs->portal_to_room == r2->real_room) ||
           (rs->room_above == r2->real_room) ||
           (rs->room_below == r2->real_room))
        {
            return 1;
        }
    }

    rs = r2->content->sectors;
    for(uint32_t i = 0; i < r2->sectors_count; i++, rs++)
    {
        if((rs->portal_to_room == r1->real_room) ||
           (rs->room_above == r1->real_room) ||
           (rs->room_below == r1->real_room))
        {
            return 1;
        }
    }

    return 0;
}


int Room_IsOverlapped(struct room_s *r0, struct room_s *r1)
{
    if((r0 == r1) || (r0->real_room == r1->real_room))
    {
        return 0;
    }

    if(r0->bb_min[0] >= r1->bb_max[0] || r0->bb_max[0] <= r1->bb_min[0] ||
       r0->bb_min[1] >= r1->bb_max[1] || r0->bb_max[1] <= r1->bb_min[1] ||
       r0->bb_min[2] >= r1->bb_max[2] || r0->bb_max[2] <= r1->bb_min[2])
    {
        return 0;
    }

    room_sector_p rs = r0->content->sectors;
    for(uint32_t i = 0; i < r0->sectors_count; i++, rs++)
    {
        if((rs->room_above == r1->real_room) ||
           (rs->room_below == r1->real_room))
        {
            return 0;
        }
    }

    rs = r1->content->sectors;
    for(uint32_t i = 0; i < r1->sectors_count; i++, rs++)
    {
        if((rs->room_above == r0->real_room) ||
           (rs->room_below == r0->real_room))
        {
            return 0;
        }
    }

    return 1;
}


int Room_IsInNearRoomsList(struct room_s *r0, struct room_s *r1)
{
    if(r0 && r1)
    {
        if(r0->id == r1->id)
        {
            return 1;
        }

        for(uint16_t i = 0; i < r0->content->near_room_list_size; i++)
        {
            if(r0->content->near_room_list[i]->real_room->id == r1->real_room->id)
            {
                return 1;
            }
        }
    }

    return 0;
}


int Room_IsInOverlappedRoomsList(struct room_s *r0, struct room_s *r1)
{
    if(r0 && r1 && (r0->id != r1->id))
    {
        for(uint16_t i = 0; i < r0->content->overlapped_room_list_size; i++)
        {
            if(r0->content->overlapped_room_list[i]->real_room->id == r1->real_room->id)
            {
                return 1;
            }
        }
    }

    return 0;
}


void Room_MoveActiveItems(struct room_s *room_to, struct room_s *room_from)
{
    engine_container_p t = room_from->containers;

    room_from->containers = NULL;
    for(; t; t = t->next)
    {
        t->room = room_to;
        t->next = room_to->containers;
        room_to->containers = t;
    }
}


void Room_GenSpritesBuffer(struct room_s *room)
{
    room->content->sprites_vertices = NULL;

    if(room->content->sprites_count > 0)
    {
        room->content->sprites_vertices = (vertex_p)malloc(room->content->sprites_count * 4 * sizeof(vertex_t));
        for(uint32_t i = 0; i < room->content->sprites_count; i++)
        {
            room_sprite_p s = room->content->sprites + i;
            if(s->sprite)
            {
                vertex_p v = room->content->sprites_vertices + i * 4;
                vec4_set_one(v[0].color);
                vec4_set_one(v[1].color);
                vec4_set_one(v[2].color);
                vec4_set_one(v[3].color);
                v[0].tex_coord[0] = s->sprite->tex_coord[0];
                v[0].tex_coord[1] = s->sprite->tex_coord[1];
                v[1].tex_coord[0] = s->sprite->tex_coord[2];
                v[1].tex_coord[1] = s->sprite->tex_coord[3];
                v[2].tex_coord[0] = s->sprite->tex_coord[4];
                v[2].tex_coord[1] = s->sprite->tex_coord[5];
                v[3].tex_coord[0] = s->sprite->tex_coord[6];
                v[3].tex_coord[1] = s->sprite->tex_coord[7];
            }
        }
    }
}


/*
 *   Sectors functionality
 */
struct room_sector_s *Sector_GetNextSector(struct room_sector_s *rs, float dir[3])
{
    int ind_x = rs->index_x;
    int ind_y = rs->index_y;
    room_p r = rs->owner_room;

    if(fabs(dir[0]) > fabs(dir[1]))
    {
        ind_x += (dir[0] > 0.0f) ? (1) : (-1);
        ind_x = ((ind_x >= 0) && (ind_x < r->sectors_x)) ? (ind_x) : (rs->index_x);
    }
    else
    {
        ind_y += (dir[1] > 0.0f) ? (1) : (-1);
        ind_y = ((ind_y >= 0) && (ind_y < r->sectors_y)) ? (ind_y) : (rs->index_y);
    }

    return r->content->sectors + (ind_x * r->sectors_y + ind_y);
}


struct room_sector_s *Sector_GetPortalSectorTargetRaw(struct room_sector_s *rs)
{
    if(rs && rs->portal_to_room)
    {
        room_p r = rs->portal_to_room;
        int ind_x = (rs->pos[0] - r->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->content->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


struct room_sector_s *Sector_GetLowest(struct room_sector_s *sector)
{
    for(; sector && sector->room_below; sector = Room_GetSectorRaw(sector->room_below->real_room, sector->pos));

    return sector;
}


struct room_sector_s *Sector_GetHighest(struct room_sector_s *sector)
{
    for(; sector && sector->room_above; sector = Room_GetSectorRaw(sector->room_above->real_room, sector->pos));

    return sector;
}


void Sector_HighestFloorCorner(room_sector_p rs, float v[3])
{
    float *r1 = (rs->floor_corners[0][2] > rs->floor_corners[1][2]) ? (rs->floor_corners[0]) : (rs->floor_corners[1]);
    float *r2 = (rs->floor_corners[2][2] > rs->floor_corners[3][2]) ? (rs->floor_corners[2]) : (rs->floor_corners[3]);

    if(r1[2] > r2[2])
    {
        vec3_copy(v, r1);
    }
    else
    {
        vec3_copy(v, r2);
    }
}


void Sector_LowestCeilingCorner(room_sector_p rs, float v[3])
{
    float *r1 = (rs->ceiling_corners[0][2] > rs->ceiling_corners[1][2]) ? (rs->ceiling_corners[0]) : (rs->ceiling_corners[1]);
    float *r2 = (rs->ceiling_corners[2][2] > rs->ceiling_corners[3][2]) ? (rs->ceiling_corners[2]) : (rs->ceiling_corners[3]);

    if(r1[2] < r2[2])
    {
        vec3_copy(v, r1);
    }
    else
    {
        vec3_copy(v, r2);
    }
}


int Sectors_SimilarFloor(room_sector_p s1, room_sector_p s2, int ignore_doors)
{
    if(!s1 || !s2) return 0;
    if( s1 ==  s2) return 1;

    if( (s1->floor != s2->floor) ||
        (s1->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
        (s2->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
        (!ignore_doors && (s1->room_below || s2->room_below)) )
    {
          return 0;
    }

    for(int i = 0; i < 4; i++)
    {
        if(s1->floor_corners[i][2] != s2->floor_corners[i][2])
        {
            return 0;
        }
    }

    return 1;
}


int Sectors_SimilarCeiling(room_sector_p s1, room_sector_p s2, int ignore_doors)
{
    if(!s1 || !s2) return 0;
    if( s1 ==  s2) return 1;

    if( (s1->ceiling != s2->ceiling) ||
        (s1->ceiling_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
        (s2->ceiling_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
        (!ignore_doors && (s1->room_above || s2->room_above)) )
    {
          return 0;
    }

    for(int i = 0; i < 4; i++)
    {
        if(s1->ceiling_corners[i][2] != s2->ceiling_corners[i][2])
        {
            return 0;
        }
    }

    return 1;
}


/////////////////////////////////////////
static bool Room_IsBoxForPath(room_box_p curr_box, room_box_p next_box, box_validition_options_p op)
{
    if(next_box && !next_box->is_blocked)
    {
        int32_t step = next_box->bb_min[2] - curr_box->bb_min[2];
        if((op->zone_type == ZONE_TYPE_FLY) || ((step >= 0) ? (step - op->step_up <= 1.0f) : (-1.0f <= step + op->step_down)))
        {
            switch(op->zone_type)
            {
                case ZONE_TYPE_ALL:
                    return true;

                case ZONE_TYPE_FLY:
                    return (op->zone_alt) ? (op->zone & next_box->zone.FlyZone_Alternate) : (op->zone & next_box->zone.FlyZone_Normal);

                case ZONE_TYPE_1:
                    return (op->zone_alt) ? (op->zone & next_box->zone.GroundZone1_Alternate) : (op->zone & next_box->zone.GroundZone1_Normal);

                case ZONE_TYPE_2:
                    return (op->zone_alt) ? (op->zone & next_box->zone.GroundZone2_Alternate) : (op->zone & next_box->zone.GroundZone2_Normal);

                case ZONE_TYPE_3:
                    return (op->zone_alt) ? (op->zone & next_box->zone.GroundZone3_Alternate) : (op->zone & next_box->zone.GroundZone3_Normal);

                case ZONE_TYPE_4:
                    return (op->zone_alt) ? (op->zone & next_box->zone.GroundZone4_Alternate) : (op->zone & next_box->zone.GroundZone4_Normal);
            }
        }
    }
    return false;
}


int  Room_IsInBox(room_box_p box, float pos[3])
{
    return (box->bb_min[0] <= pos[0]) && (pos[0] <= box->bb_max[0]) &&
           (box->bb_min[1] <= pos[1]) && (pos[1] <= box->bb_max[1]);
}


int  Room_FindPath(room_box_p *path_buf, uint32_t max_boxes, room_sector_p from, room_sector_p to, box_validition_options_p op)
{
    int ret = 0;
    if(from->box && to->box)
    {
        if(from->box->id != to->box->id)
        {
            float pt_from[3], pt_to[3];
            const int buf_size = sizeof(room_box_p) * max_boxes;
            room_box_p *current_front = (room_box_p*)Sys_GetTempMem(3 * buf_size);
            room_box_p *next_front = current_front + max_boxes;
            room_box_p *parents = next_front + max_boxes;
            int32_t *weights = (int32_t*)Sys_GetTempMem(max_boxes * sizeof(int32_t));
            size_t current_front_size = 1;
            size_t next_front_size = 0;

            current_front[0] = from->box;
            weights[current_front[0]->id] = 0;
            memset(parents, 0x00, buf_size);

            while(current_front_size > 0)
            {
                for(size_t i = 0; i < current_front_size; ++i)
                {
                    room_box_p current_box = current_front[i];
                    box_overlap_p ov = current_box->overlaps;
                    if(parents[current_box->id])
                    {
                        Room_GetOverlapCenter(parents[current_box->id], current_box, pt_from);
                    }
                    else
                    {
                        vec3_copy(pt_from, from->pos);
                    }

                    while(ov)
                    {
                        room_box_p next_box = World_GetRoomBoxByID(ov->box);
                        Room_GetOverlapCenter(current_box, next_box, pt_to);
                        int32_t weight = (fabs(pt_to[0] - pt_from[0]) + fabs(pt_to[1] - pt_from[1]) + 1.0f) / TR_METERING_STEP;
                        if((next_box->id != from->box->id) && Room_IsBoxForPath(current_box, next_box, op) &&
                           (!parents[to->box->id] || (weights[current_box->id] + weight < weights[to->box->id])))
                        {
                            if(!parents[next_box->id])
                            {
                                next_front[next_front_size++] = next_box;
                                parents[next_box->id] = current_box;
                                weights[next_box->id] = weights[current_box->id] + weight;
                            }
                            else if(weights[next_box->id] > weights[current_box->id] + weight)
                            {
                                bool not_in_front = true;
                                parents[next_box->id] = current_box;
                                weights[next_box->id] = weights[current_box->id] + weight;
                                for(size_t j = 0; j < next_front_size; ++j)
                                {
                                    if(next_front[j]->id == next_box->id)
                                    {
                                        not_in_front = false;
                                        break;
                                    }
                                }

                                if(not_in_front)
                                {
                                    next_front[next_front_size++] = next_box;
                                }
                            }
                        }

                        if(ov->end)
                        {
                            break;
                        }
                        ov++;
                    }
                }

                ///SWAP FRONTS HERE
                {
                    room_box_p *tn = current_front;
                    current_front = next_front;
                    current_front_size = next_front_size;
                    next_front = tn;
                    next_front_size = 0;
                }
            }

            if(parents[to->box->id])
            {
                room_box_p p = to->box;
                while(p)
                {
                    path_buf[ret++] = p;
                    p = parents[p->id];
                }
            }

            Sys_ReturnTempMem(3 * buf_size + max_boxes * sizeof(int32_t));
        }
        else
        {
            path_buf[0] = from->box;
            ret = 1;
        }
    }

    return ret;
}


void Room_GetOverlapCenter(room_box_p b1, room_box_p b2, float pos[3])
{
    pos[0] = (b1->bb_min[0] > b2->bb_min[0]) ? (b1->bb_min[0]) : (b2->bb_min[0]);
    pos[0] += (b1->bb_max[0] > b2->bb_max[0]) ? (b2->bb_max[0]) : (b1->bb_max[0]);
    pos[0] *= 0.5f;
    pos[1] = (b1->bb_min[1] > b2->bb_min[1]) ? (b1->bb_min[1]) : (b2->bb_min[1]);
    pos[1] += (b1->bb_max[1] > b2->bb_max[1]) ? (b2->bb_max[1]) : (b1->bb_max[1]);
    pos[1] *= 0.5f;
    pos[2] = 0.5f * (b1->bb_min[2] + b2->bb_min[2] + TR_METERING_SECTORSIZE);
}