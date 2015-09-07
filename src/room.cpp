
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
#include "engine_physics.h"
#include "entity.h"
#include "mesh.h"
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

        if(room->content->light_count)
        {
            free(room->content->lights);
            room->content->lights = NULL;
            room->content->light_count = 0;
        }

        free(room->content);
        room->content = NULL;
    }


    p = room->portals;
    room->near_room_list_size = 0;

    if(room->portals_count)
    {
        for(uint16_t i = 0;i < room->portals_count; i++, p++)
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
        free(room->sectors);
        room->sectors = NULL;
        room->sectors_count = 0;
        room->sectors_x = 0;
        room->sectors_y = 0;
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
                Entity_Enable((entity_p)cont->object);
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
                Entity_Disable((entity_p)cont->object);
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
    for(;current_cont!=NULL;)
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


struct room_sector_s *Room_GetSectorRaw(struct room_s *room, float pos[3])
{
    int x, y;
    room_sector_p ret = NULL;

    if((room == NULL) || !room->active)
    {
        return NULL;
    }

    x = (int)(pos[0] - room->transform[12]) / 1024;
    y = (int)(pos[1] - room->transform[13]) / 1024;
    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return NULL;
    }
    /*
     * column index system
     * X - column number, Y - string number
     */
    ret = room->sectors + x * room->sectors_y + y;
    return ret;
}


struct room_sector_s *Room_GetSectorCheckFlip(struct room_s *room, float pos[3])
{
    int x, y;
    room_sector_p ret = NULL;

    if(room != NULL)
    {
        if(room->active == 0)
        {
            if((room->base_room != NULL) && (room->base_room->active))
            {
                room = room->base_room;
            }
            else if((room->alternate_room != NULL) && (room->alternate_room->active))
            {
                room = room->alternate_room;
            }
        }
    }
    else
    {
        return NULL;
    }

    if(!room->active)
    {
        return NULL;
    }

    x = (int)(pos[0] - room->transform[12]) / 1024;
    y = (int)(pos[1] - room->transform[13]) / 1024;
    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return NULL;
    }
    /*
     * column index system
     * X - column number, Y - string number
     */
    ret = room->sectors + x * room->sectors_y + y;
    return ret;
}


room_sector_p Room_GetSectorXYZ(room_p room, float pos[3])
{
    int x, y;
    room_sector_p ret = NULL;

    room = Room_CheckFlip(room);

    if(!room->active)
    {
        return NULL;
    }

    x = (int)(pos[0] - room->transform[12]) / 1024;
    y = (int)(pos[1] - room->transform[13]) / 1024;
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
    if(ret->sector_below && (ret->sector_below->ceiling >= pos[2]))
    {
        return Sector_CheckFlip(ret->sector_below);
    }

    if(ret->sector_above && (ret->sector_above->floor <= pos[2]))
    {
        return Sector_CheckFlip(ret->sector_above);
    }

    return ret;
}


void Room_AddToNearRoomsList(struct room_s *room, struct room_s *r)
{
    if(room && r && !Room_IsInNearRoomsList(room, r) && room->id != r->id && !Room_IsOverlapped(room, r) && room->near_room_list_size < 64)
    {
        room->near_room_list[room->near_room_list_size] = r;
        room->near_room_list_size++;
    }
}


int Room_IsJoined(struct room_s *r1, struct room_s *r2)
{
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
    if((r0 == r1) || (r0 == r1->alternate_room) || (r0->alternate_room == r1))
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
            for(uint16_t i=0;i<r0->near_room_list_size;i++)
            {
                if(r0->near_room_list[i]->id == r1->id)
                {
                    return 1;
                }
            }
        }
        else
        {
            for(uint16_t i=0;i<r1->near_room_list_size;i++)
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


void Room_SwapItems(struct room_s *room, struct room_s *dest_room)
{
    engine_container_p t;

    for(t=room->content->containers;t!=NULL;t=t->next)
    {
        t->room = dest_room;
    }

    for(t=dest_room->content->containers;t!=NULL;t=t->next)
    {
        t->room = room;
    }

    SWAPT(room->content->containers, dest_room->content->containers, t);
}


struct room_s *Room_CheckFlip(struct room_s *r)
{
    if(r && (r->active == 0))
    {
        if((r->base_room != NULL) && (r->base_room->active))
        {
            r = r->base_room;
        }
        else if((r->alternate_room != NULL) && (r->alternate_room->active))
        {
            r = r->alternate_room;
        }
    }

    return r;
}

/*
 *   Sectors functionality
 */
room_sector_p Sector_CheckBaseRoom(room_sector_p rs)
{
    if((rs != NULL) && (rs->owner_room->base_room != NULL))
    {
        room_p r = rs->owner_room->base_room;
        int ind_x = (rs->pos[0] - r->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


room_sector_p Sector_CheckAlternateRoom(room_sector_p rs)
{
    if((rs != NULL) && (rs->owner_room->alternate_room != NULL))
    {
        room_p r = rs->owner_room->alternate_room;
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
    if((rs != NULL) && (rs->portal_to_room != NULL))
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


struct room_sector_s *Sector_GetPortalSectorTarget(struct room_sector_s *rs)
{
    if((rs != NULL) && (rs->portal_to_room != NULL))
    {
        room_p r = rs->portal_to_room;
        if((rs->owner_room->base_room != NULL) && (r->alternate_room != NULL))
        {
            r = r->alternate_room;
        }
        else if((rs->owner_room->alternate_room != NULL) && (r->base_room != NULL))
        {
            r = r->base_room;
        }
        int ind_x = (rs->pos[0] - r->transform[12 + 0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform[12 + 1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


int Sectors_Is2SidePortals(struct room_sector_s *s1, struct room_sector_s *s2)
{
    s1 = Sector_GetPortalSectorTarget(s1);
    s2 = Sector_GetPortalSectorTarget(s2);

    if(s1->owner_room == s2->owner_room)
    {
        return 0;
    }

    room_sector_p s1p = Room_GetSectorRaw(s2->owner_room, s1->pos);
    room_sector_p s2p = Room_GetSectorRaw(s1->owner_room, s2->pos);

    // 2 next conditions are the stick for TR_V door-roll-wall
    if(!s1p->portal_to_room)
    {
        s1p = Sector_CheckAlternateRoom(s1p);
        if(!s1p->portal_to_room)
        {
            return 0;
        }
    }
    if(!s2p->portal_to_room)
    {
        s2p = Sector_CheckAlternateRoom(s2p);
        if(!s2p->portal_to_room)
        {
            return 0;
        }
    }

    if((Sector_GetPortalSectorTarget(s1p) == Sector_CheckBaseRoom(s1)) && (Sector_GetPortalSectorTarget(s2p) == Sector_GetPortalSectorTarget(s2)) ||
       (Sector_GetPortalSectorTarget(s1p) == Sector_CheckAlternateRoom(s1)) && (Sector_GetPortalSectorTarget(s2p) == Sector_GetPortalSectorTarget(s2)))
    {
        return 1;
    }

    return 0;
}


struct room_sector_s *Sector_CheckFlip(struct room_sector_s *rs)
{
    if((rs != NULL) && (rs->owner_room->active == 0))
    {
        if((rs->owner_room->base_room != NULL) && (rs->owner_room->base_room->active))
        {
            room_p r = rs->owner_room->base_room;
            rs = r->sectors + rs->index_x * r->sectors_y + rs->index_y;
        }
        else if((rs->owner_room->alternate_room != NULL) && (rs->owner_room->alternate_room->active))
        {
            room_p r = rs->owner_room->alternate_room;
            rs = r->sectors + rs->index_x * r->sectors_y + rs->index_y;
        }
    }

    return rs;
}


struct room_sector_s *Sector_GetLowest(struct room_sector_s *sector)
{
    for(sector=Sector_CheckFlip(sector);sector->sector_below!=NULL;sector=Sector_CheckFlip(sector->sector_below));

    return Sector_CheckFlip(sector);
}


struct room_sector_s *Sector_GetHighest(struct room_sector_s *sector)
{
    for(sector=Sector_CheckFlip(sector);sector->sector_above!=NULL;sector=Sector_CheckFlip(sector->sector_above));

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
        (!ignore_doors && (s1->sector_below || s2->sector_below))     )
    {
          return 0;
    }

    for(int i = 0; i < 4; i++)
    {
        if(s1->floor_corners[0][2] != s2->floor_corners[0][2])
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
        (!ignore_doors && (s1->sector_above || s2->sector_above)) )
    {
          return 0;
    }

    for(int i = 0; i < 4; i++)
    {
        if(s1->ceiling_corners[0][2] != s2->ceiling_corners[0][2])
        {
            return 0;
        }
    }

    return 1;
}
