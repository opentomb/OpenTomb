
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "core/gl_util.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/frustum.h"
#include "physics/physics.h"
#include "engine.h"
#include "entity.h"
#include "mesh.h"
#include "trigger.h"
#include "room.h"


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
                    free(content->static_mesh[i].self);
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
        free(room->self);
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
       !Room_IsInNearRoomsList(room, r) && !Room_IsInOverlappedRoomsList(room, r))
    {
        room->content->near_room_list[room->content->near_room_list_size] = r->real_room;
        room->content->near_room_list_size++;
    }
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

    const int margin = TR_METERING_SECTORSIZE * 2;

    if(r0->bb_min[0] >= r1->bb_max[0] - margin || r0->bb_max[0] - margin <= r1->bb_min[0] ||
       r0->bb_min[1] >= r1->bb_max[1] - margin || r0->bb_max[1] - margin <= r1->bb_min[1] ||
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

        if(r1->content->near_room_list_size >= r0->content->near_room_list_size)
        {
            for(uint16_t i = 0; i < r0->content->near_room_list_size; i++)
            {
                if(r0->content->near_room_list[i]->real_room->id == r1->real_room->id)
                {
                    return 1;
                }
            }
        }
        else
        {
            for(uint16_t i = 0; i < r1->content->near_room_list_size; i++)
            {
                if(r1->content->near_room_list[i]->real_room->id == r0->real_room->id)
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}


int Room_IsInOverlappedRoomsList(struct room_s *r0, struct room_s *r1)
{
    if(r0 && r1)
    {
        if(r0->id == r1->id)
        {
            return 0;
        }

        if(r1->content->overlapped_room_list_size >= r0->content->overlapped_room_list_size)
        {
            for(uint16_t i = 0; i < r0->content->overlapped_room_list_size; i++)
            {
                if(r0->content->overlapped_room_list[i]->real_room->id == r1->real_room->id)
                {
                    return 1;
                }
            }
        }
        else
        {
            for(uint16_t i = 0; i < r1->content->overlapped_room_list_size; i++)
            {
                if(r1->content->overlapped_room_list[i]->real_room->id == r0->real_room->id)
                {
                    return 1;
                }
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
static room_sector_p Sector_CheckRealRoom(room_sector_p rs)
{
    if(rs && (rs->owner_room != rs->owner_room->real_room))
    {
        room_p r = rs->owner_room->real_room;
        int ind_x = (rs->pos[0] - r->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->content->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
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


struct room_sector_s *Sector_GetPortalSectorTargetReal(struct room_sector_s *rs)
{
    if(rs && rs->portal_to_room)
    {
        room_p r = rs->portal_to_room->real_room;
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
