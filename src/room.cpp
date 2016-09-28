
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "core/gl_util.h"
#include "core/console.h"
#include "core/redblack.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/frustum.h"
#include "engine.h"
#include "physics.h"
#include "entity.h"
#include "mesh.h"
#include "trigger.h"
#include "room.h"


void Room_Clear(struct room_s *room)
{
    portal_p p;

    if(room == NULL)
    {
        return;
    }

    if(room->content)
    {
        room->content->containers = NULL;

        if(room->content->mesh)
        {
            BaseMesh_Clear(room->content->mesh);
            free(room->content->mesh);
            room->content->mesh = NULL;
        }

        if(room->content->static_mesh_count)
        {
            for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
            {
                Physics_DeleteObject(room->content->static_mesh[i].physics_body);
                room->content->static_mesh[i].physics_body = NULL;

                OBB_Clear(room->content->static_mesh[i].obb);
                free(room->content->static_mesh[i].obb);
                room->content->static_mesh[i].obb = NULL;
                if(room->content->static_mesh[i].self)
                {
                    room->content->static_mesh[i].self->room = NULL;
                    free(room->content->static_mesh[i].self);
                    room->content->static_mesh[i].self = NULL;
                }
            }
            free(room->content->static_mesh);
            room->content->static_mesh = NULL;
            room->content->static_mesh_count = 0;
        }

        Physics_DeleteObject(room->content->physics_body);
        room->content->physics_body = NULL;

        if(room->content->sprites_count)
        {
            free(room->content->sprites);
            room->content->sprites = NULL;
            room->content->sprites_count = 0;
        }

        if(room->content->sprites_vertices)
        {
            free(room->content->sprites_vertices);
            room->content->sprites_vertices = NULL;
        }

        if(room->content->lights_count)
        {
            free(room->content->lights);
            room->content->lights = NULL;
            room->content->lights_count = 0;
        }

        free(room->content);
        room->content = NULL;
    }


    p = room->portals;
    room->near_room_list_size = 0;

    if(room->portals_count)
    {
        for(uint16_t i = 0; i < room->portals_count; i++, p++)
        {
            Portal_Clear(p);
        }
        free(room->portals);
        room->portals = NULL;
        room->portals_count = 0;
    }

    room->frustum = NULL;

    if(room->sectors_count)
    {
        room_sector_p s = room->sectors;
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
        free(room->sectors);
        room->sectors = NULL;
        room->sectors_count = 0;
        room->sectors_x = 0;
        room->sectors_y = 0;
    }

    if(room->overlapped_room_list)
    {
        room->overlapped_room_list_size = 0;
        free(room->overlapped_room_list);
        room->overlapped_room_list = NULL;
    }

    if(room->near_room_list)
    {
        room->near_room_list_size = 0;
        free(room->near_room_list);
        room->near_room_list = NULL;
    }

    if(room->obb)
    {
        OBB_Clear(room->obb);
        free(room->obb);
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
    if(room->active)
    {
        return;
    }

    if(room->content->physics_body != NULL)
    {
        Physics_EnableObject(room->content->physics_body);
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(room->content->static_mesh[i].physics_body != NULL)
        {
            Physics_EnableObject(room->content->static_mesh[i].physics_body);
        }
    }

    for(engine_container_p cont = room->content->containers; cont; cont = cont->next)
    {
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

    room->active = 1;
}


void Room_Disable(struct room_s *room)
{
    if(!room->active)
    {
        return;
    }

    if(room->content->physics_body)
    {
        Physics_DisableObject(room->content->physics_body);
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(room->content->static_mesh[i].physics_body)
        {
            Physics_DisableObject(room->content->static_mesh[i].physics_body);
        }
    }

    for(engine_container_p cont = room->content->containers; cont; cont = cont->next)
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

    room->active = 0;
}


int  Room_AddObject(struct room_s *room, struct engine_container_s *cont)
{
    engine_container_p curr = room->content->containers;

    for(; curr; curr = curr->next)
    {
        if(curr == cont)
        {
            return 0;
        }
    }

    cont->room = room;
    cont->next = room->content->containers;
    room->content->containers = cont;
    return 1;
}


int  Room_RemoveObject(struct room_s *room, struct engine_container_s *cont)
{
    engine_container_p previous_cont, current_cont;

    if((cont == NULL) || (room->content->containers == NULL))
    {
        return 0;
    }

    if(room->content->containers == cont)
    {
        room->content->containers = cont->next;
        cont->room = NULL;
        return 1;
    }

    previous_cont = room->content->containers;
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


void Room_SwapContent(struct room_s *room1, struct room_s *room2)
{
    if(room1 && room2)
    {
        room1->frustum = NULL;
        room2->frustum = NULL;

        // swap flags
        {
            uint32_t t = room1->flags;
            room1->flags = room2->flags;
            room2->flags = t;
        }

        // swap portals
        {
            portal_p t = room1->portals;
            uint16_t count = room1->portals_count;
            room1->portals = room2->portals;
            room1->portals_count = room2->portals_count;
            room2->portals = t;
            room2->portals_count = count;
        }

        // swap sectors
        {
            room_sector_p t = room1->sectors;
            uint32_t count = room1->sectors_count;
            room1->sectors = room2->sectors;
            room1->sectors_count = room2->sectors_count;
            room2->sectors = t;
            room2->sectors_count = count;

            for(uint32_t i = 0; i < room1->sectors_count; ++i)
            {
                room1->sectors[i].owner_room = room1;
            }

            for(uint32_t i = 0; i < room2->sectors_count; ++i)
            {
                room2->sectors[i].owner_room = room2;
            }
        }

        // swap near / overlapped rooms list
        {
            room_p *t = room1->near_room_list;
            uint16_t size = room1->near_room_list_size;
            room1->near_room_list = room2->near_room_list;
            room1->near_room_list_size = room2->near_room_list_size;
            room2->near_room_list = t;
            room2->near_room_list_size = size;

            t = room1->overlapped_room_list;
            size = room1->overlapped_room_list_size;
            room1->overlapped_room_list = room2->overlapped_room_list;
            room1->overlapped_room_list_size = room2->overlapped_room_list_size;
            room2->overlapped_room_list = t;
            room2->overlapped_room_list_size = size;
        }

        // swap content
        {
            room_content_p t = room1->content;
            room1->content = room2->content;
            room2->content = t;

            // fix physics
            Physics_SetOwnerObject(room1->content->physics_body, room1->self);
            Physics_SetOwnerObject(room2->content->physics_body, room2->self);

            // fix static meshes
            for(uint32_t i = 0; i < room1->content->static_mesh_count; ++i)
            {
                room1->content->static_mesh[i].self->room = room1;
            }
            for(uint32_t i = 0; i < room2->content->static_mesh_count; ++i)
            {
                room2->content->static_mesh[i].self->room = room2;
            }

            // fix containers
            for(engine_container_p cont = room1->content->containers; cont; cont = cont->next)
            {
                cont->room = room1;
            }
            for(engine_container_p cont = room2->content->containers; cont; cont = cont->next)
            {
                cont->room = room2;
            }
        }
    }
}


struct room_sector_s *Room_GetSectorRaw(struct room_s *room, float pos[3])
{
    if(room)
    {
        int x = (int)(pos[0] - room->transform[12]) / 1024;
        int y = (int)(pos[1] - room->transform[13]) / 1024;
        if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
        {
            return NULL;
        }
        /*
         * column index system
         * X - column number, Y - string number
         */
        return room->sectors + x * room->sectors_y + y;
    }

    return NULL;
}


room_sector_p Room_GetSectorXYZ(room_p room, float pos[3])
{
    room_sector_p ret = NULL;
    int x = (int)(pos[0] - room->transform[12]) / 1024;
    int y = (int)(pos[1] - room->transform[13]) / 1024;

    room = room->real_room;

    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return NULL;
    }
    /*
     * column index system
     * X - column number, Y - string number
     */
    ret = room->sectors + x * room->sectors_y + y;

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
       !Room_IsInNearRoomsList(room, r) && !Room_IsOverlapped(room, r))
    {
        room->near_room_list[room->near_room_list_size] = r->real_room;
        room->near_room_list_size++;
    }
}


int Room_IsJoined(struct room_s *r1, struct room_s *r2)
{
    r1 = r1->real_room;
    r2 = r2->real_room;
    portal_p p = r1->portals;
    for(uint16_t i = 0; i < r1->portals_count; i++, p++)
    {
        if(p->dest_room->id == r2->id)
        {
            return 1;
        }
    }

    p = r2->portals;
    for(uint16_t i = 0; i < r2->portals_count; i++, p++)
    {
        if(p->dest_room->id == r1->id)
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

    return !Room_IsJoined(r0, r1);
}


int Room_IsInNearRoomsList(struct room_s *r0, struct room_s *r1)
{
    if(r0 && r1)
    {
        if(r0->id == r1->id)
        {
            return 1;
        }

        if(r1->near_room_list_size >= r0->near_room_list_size)
        {
            for(uint16_t i = 0; i < r0->near_room_list_size; i++)
            {
                if(r0->near_room_list[i]->id == r1->id)
                {
                    return 1;
                }
            }
        }
        else
        {
            for(uint16_t i = 0; i < r1->near_room_list_size; i++)
            {
                if(r1->near_room_list[i]->id == r0->id)
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
    engine_container_p t = room_from->content->containers;

    room_from->content->containers = NULL;
    for(; t; t = t->next)
    {
        t->room = room_to;
        t->next = room_to->content->containers;
        room_to->content->containers = t;
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
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
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
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
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
    float *r1 = (rs->floor_corners[0][2] > rs->floor_corners[1][2])?(rs->floor_corners[0]):(rs->floor_corners[1]);
    float *r2 = (rs->floor_corners[2][2] > rs->floor_corners[3][2])?(rs->floor_corners[2]):(rs->floor_corners[3]);

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
    float *r1 = (rs->ceiling_corners[0][2] > rs->ceiling_corners[1][2])?(rs->ceiling_corners[0]):(rs->ceiling_corners[1]);
    float *r2 = (rs->ceiling_corners[2][2] > rs->ceiling_corners[3][2])?(rs->ceiling_corners[2]):(rs->ceiling_corners[3]);

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
