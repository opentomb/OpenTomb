
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

#include "audio.h"
#include "vmath.h"
#include "polygon.h"
#include "camera.h"
#include "portal.h"
#include "frustum.h"
#include "world.h"
#include "mesh.h"
#include "entity.h"
#include "render.h"
#include "engine.h"
#include "script.h"
#include "bounding_volume.h"
#include "redblack.h"


void Room_Empty(room_p room)
{
    int i;
    portal_p p;
    btRigidBody* body;

    if(room == NULL)
    {
        return;
    }

    p = room->portals;
    room->near_room_list_size = 0;

    if(room->portal_count)
    {
        for(i=0;i<room->portal_count;i++,p++)
        {
            Portal_Clear(p);
        }
        free(room->portals);
        room->portals = NULL;
        room->portal_count = 0;
    }
    
    Frustum_Delete(room->frustum);
    room->frustum = NULL;

    if(room->mesh)
    {
        BaseMesh_Clear(room->mesh);
        free(room->mesh);
        room->mesh = NULL;
    }

    if(room->static_mesh_count)
    {
        for(i=0;i<room->static_mesh_count;i++)
        {
            body = room->static_mesh[i].bt_body;
            if(body)
            {
                body->setUserPointer(NULL);
                if(body && body->getMotionState())
                {
                    delete body->getMotionState();
                }
                if(body && body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                }

                bt_engine_dynamicsWorld->removeRigidBody(body);
                delete body;
                room->static_mesh[i].bt_body = NULL;
            }

            BV_Clear(room->static_mesh[i].bv);
            free(room->static_mesh[i].bv);
            room->static_mesh[i].bv = NULL;
            if(room->static_mesh[i].self)
            {
                free(room->static_mesh[i].self);
                room->static_mesh[i].self = NULL;
            }
        }
        free(room->static_mesh);
        room->static_mesh = NULL;
        room->static_mesh_count = 0;
    }

    if(room->bt_body)
    {
        body = room->bt_body;
        if(body)
        {
            body->setUserPointer(NULL);
            if(body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            if(body && body->getCollisionShape())
            {
                delete body->getCollisionShape();
            }

            bt_engine_dynamicsWorld->removeRigidBody(body);
            delete body;
            room->bt_body = NULL;
        }
    }

    if(room->sectors_count)
    {
        free(room->sectors);
        room->sectors = NULL;
        room->sectors_count = 0;
        room->sectors_x = 0;
        room->sectors_y = 0;
    }

    if(room->sprites_count)
    {
        free(room->sprites);
        room->sprites = NULL;
        room->sprites_count = 0;
    }

    if(room->self)
    {
        free(room->self);
        room->self = NULL;
    }
}


void Room_AddEntity(room_p room, struct entity_s *entity)
{
    engine_container_p curr;

    curr = entity->self;
    curr->object = entity;
    curr->object_type = OBJECT_ENTITY;

    curr->next = room->containers;
    room->containers = curr;
}


int Room_RemoveEntity(room_p room, struct entity_s *entity)
{
    engine_container_p cont, prev;

    prev = NULL;
    for(cont=room->containers;cont;)
    {
        if(entity == (entity_p)cont->object)
        {
            break;
        }
        prev = cont;
        cont = cont->next;
    }
    if(cont == NULL)
    {
        return 0;                                                               // Entity not found
    }

    if(prev == NULL)                                                            // start list
    {
        room->containers = room->containers->next;
        //free(cont);
    }
    else
    {
        prev->next = cont->next;
        //free(cont);
    }

    cont->next = NULL;
    return 1;
}


void Room_AddToNearRoomsList(room_p room, room_p r)
{
    if(room && r && !Room_IsInNearRoomsList(room, r) && room->ID != r->ID && !Room_IsOverlapped(room, r) && room->near_room_list_size < 64)
    {
        room->near_room_list[room->near_room_list_size++] = r;
    }
}


int Room_IsInNearRoomsList(room_p r0, room_p r1)
{
    int i;

    if(r0 && r1)
    {
        if(r0->ID == r1->ID)
        {
            return 1;
        }

        if(r1->near_room_list_size >= r0->near_room_list_size)
        {
            for(i=0;i<r0->near_room_list_size;i++)
            {
                if(r0->near_room_list[i]->ID == r1->ID)
                {
                    return 1;
                }
            }
        }
        else
        {
            for(i=0;i<r1->near_room_list_size;i++)
            {
                if(r1->near_room_list[i]->ID == r0->ID)
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}


int Room_IsOverlapped(room_p r0, room_p r1)
{
    if(r0->bb_min[0] >= r1->bb_max[0] || r0->bb_max[0] <= r1->bb_min[0] || 
       r0->bb_min[1] >= r1->bb_max[1] || r0->bb_max[1] <= r1->bb_min[1] ||
       r0->bb_min[2] >= r1->bb_max[2] || r0->bb_max[2] <= r1->bb_min[2])
    {
        return 0;
    }
    
    return !Room_IsJoined(r0, r1);
}


void World_Prepare(world_p world)
{
    world->ID = 0;
    world->name = NULL;
    world->type = 0x00;
    world->meshes = NULL;
    world->meshs_count = 0;
    world->sprites = NULL;
    world->sprites_count = 0;
    world->room_count = 0;
    world->rooms = 0;
    world->textures = NULL;
    world->type = 0;
    world->entity_tree = NULL;
    world->Character = NULL;
    
    world->audio_sources = NULL;
    world->audio_sources_count = 0;
    world->audio_buffers = NULL;
    world->audio_buffers_count = 0;
    world->audio_effects = NULL;
    world->audio_effects_count = 0;
    world->anim_sequences = NULL;
    world->anim_sequences_count = 0;
    world->stream_tracks = NULL;
    world->stream_tracks_count = 0;
    world->tex_count = 0;
    world->textures = 0;
    world->floor_data = NULL;
    world->floor_data_size = 0;
    world->room_boxes = NULL;
    world->room_box_count = 0;
    world->skeletal_models = NULL;
    world->skeletal_model_count = 0;
    world->sky_box = NULL;
    world->anim_commands = NULL;
    world->anim_commands_count = 0;
}


void World_Empty(world_p world)
{
    int32_t i;
    engine_container_p cont;
    extern entity_p last_rmb;
    
    last_rmb = NULL;
    
    for(i=0;i<world->room_count;i++)
    {
        Room_Empty(world->rooms+i);
    }
    free(world->rooms);
    world->rooms = NULL;

    if(world->room_box_count)
    {
        free(world->room_boxes);
        world->room_boxes = NULL;
        world->room_box_count = 0;
    }

    /*sprite empty*/
    if(world->sprites_count)
    {
        free(world->sprites);
        world->sprites = NULL;
        world->sprites_count = 0;
    }

    /*entity empty*/
    RB_Free(world->entity_tree);
    world->entity_tree = NULL;
    
    if(world->Character)
    {
        Entity_Clear(world->Character);
        free(world->Character);
        world->Character = NULL;
    }

    if(world->skeletal_model_count)
    {
        for(i=0;i<world->skeletal_model_count;i++)
        {
            SkeletalModel_Clear(world->skeletal_models+i);
        }
        free(world->skeletal_models);
        world->skeletal_models = NULL;
        world->skeletal_model_count = 0;
    }

    /*mesh empty*/

    if(world->meshs_count)
    {
        for(i=0;i<world->meshs_count;i++)
        {
            BaseMesh_Clear(world->meshes+i);
        }
        free(world->meshes);
        world->meshes = NULL;
        world->meshs_count = 0;
    }

    /*
     * FD clear
     */
    if(world->floor_data_size)
    {
        free(world->floor_data);
        world->floor_data = NULL;
        world->floor_data_size = 0;
    }

    if(bt_engine_dynamicsWorld)
    {
        for (i=bt_engine_dynamicsWorld->getNumCollisionObjects()-1;i>=0;i--)
        {
            btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if(body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            if(body && body->getCollisionShape())
            {
                delete body->getCollisionShape();
            }
            cont = (engine_container_p)body->getUserPointer();
            if(cont && cont->object_type == OBJECT_BULLET_MISC)
            {
                body->setUserPointer(NULL);
                cont->room = NULL;
                free(cont);
            }

            bt_engine_dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
    }
    if(world->tex_count)
    {
        glDeleteTextures(world->tex_count ,world->textures);
        world->tex_count = 0;
        free(world->textures);
        world->textures = NULL;
    }
    
    if(world->tex_atlas)
    {
        BorderedTextureAtlas_Destroy(world->tex_atlas);
        world->tex_atlas = NULL;
    }
                
    if(world->anim_sequences_count)
    {
        for(i = 0; i < world->anim_sequences_count; i++)
        {
            if(world->anim_sequences[i].frame_count)
            {
                free(world->anim_sequences[i].frame_list);
                world->anim_sequences[i].frame_list = NULL;
            }
        }
        world->anim_sequences_count = 0;
        free(world->anim_sequences);
        world->anim_sequences = NULL;
    }
    
    // De-initialize and destroy all audio objects.
    Audio_DeInit();
}


int compEntityEQ(void *x, void *y)
{
    return (*((uint32_t*)x) == *((uint32_t*)y));
}


int compEntityLT(void *x, void *y)
{
    return (*((uint32_t*)x) < *((uint32_t*)y));
}


void RBEntityFree(void *x)
{
    Entity_Clear((entity_p)x);
    free(x);
}


struct entity_s *World_GetEntityByID(world_p world, uint32_t id)
{
    entity_p ent = NULL;
    RedBlackNode_p node;
    
    if(world->Character && (world->Character->ID == id))
    {
        return world->Character;
    }
       
    node = RB_SearchNode(&id, world->entity_tree);
    if(node)
    {
        ent = (entity_p)node->data;
    }
    
    return ent;
}


inline int Room_IsPointIn(room_p room, btScalar dot[3])
{
    return (dot[0] >= room->bb_min[0]) && (dot[0] < room->bb_max[0]) &&
           (dot[1] >= room->bb_min[1]) && (dot[1] < room->bb_max[1]) &&
           (dot[2] >= room->bb_min[2]) && (dot[2] < room->bb_max[2]);
}


room_p Room_FindPos(world_p w, btScalar pos[3])
{
    unsigned int i;
    room_p r = w->rooms;
    for(i=0;i<w->room_count;i++,r++)
    {
        if(r->active && 
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }
    return NULL;
}


room_p Room_FindPos2d(world_p w, btScalar pos[3])
{
    unsigned int i;
    room_p r = w->rooms;
    for(i=0;i<w->room_count;i++,r++)
    {
        if((pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]))
        {
            return r;
        }
    }
    return NULL;
}


room_p Room_FindPosCogerrence(world_p w, btScalar pos[3], room_p room)
{
    unsigned int i;
    room_p r;
    
    if(room == NULL)
    {
        return Room_FindPos(w, pos);
    }

    if(room->active && 
       (pos[0] >= room->bb_min[0]) && (pos[0] < room->bb_max[0]) &&
       (pos[1] >= room->bb_min[1]) && (pos[1] < room->bb_max[1]) &&
       (pos[2] >= room->bb_min[2]) && (pos[2] < room->bb_max[2]))
    {
        return room;
    }

    for(i=0;i<room->near_room_list_size;i++)
    {
        r = room->near_room_list[i];
        if(r->active &&
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }

    return Room_FindPos(w, pos);
}


room_p Room_FindPosCogerrence2d(world_p w, btScalar pos[3], room_p room)
{
    unsigned int i;
    room_p r;
    if(room == NULL)
    {
        return Room_FindPos2d(w, pos);
    }

    if((pos[0] >= room->bb_min[0]) && (pos[0] < room->bb_max[0]) &&
       (pos[1] >= room->bb_min[1]) && (pos[1] < room->bb_max[1]) &&
       (pos[2] >= room->bb_min[2]) && (pos[2] < room->bb_max[2]))
    {
        return room;
    }

    for(i=0;i<room->portal_count;i++)
    {
        r = room->portals[i].dest_room;
        if((pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }
    return Room_FindPos2d(w, pos);
}


room_p Room_GetByID(world_p w, unsigned int ID)
{
    unsigned int i;
    room_p r = w->rooms;
    for(i=0;i<w->room_count;i++,r++)
    {
        if(ID == r->ID)
        {
            return r;
        }
    }
    return NULL;
}


room_sector_p Room_GetSector(room_p room, btScalar pos[3])
{
    int x, y;
    room_sector_p ret = NULL;

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


room_sector_p Room_GetSectorXYZ(room_p room, btScalar pos[3])
{
    int x, y;
    room_sector_p ret = NULL;

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
        return ret->sector_below;
    }
    
    if(ret->sector_above && (ret->sector_above->floor <= pos[2]))
    {
        return ret->sector_above;
    }
    
    return ret;
}


void Room_Enable(room_p room)
{
    int i;
    engine_container_p cont;
    
    if(room->active)
    {
        return;
    }
    
    if(room->bt_body)
    {
        bt_engine_dynamicsWorld->addRigidBody(room->bt_body);
    }
    for(i=0;i<room->static_mesh_count;i++)
    {
        if(room->static_mesh[i].bt_body)
        {
            bt_engine_dynamicsWorld->addRigidBody(room->static_mesh[i].bt_body);
        }
    }
    
    for(cont = room->containers;cont;cont = cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                Entity_Disable((entity_p)cont->object);
                break;
        }
    }
    
    room->active = 1;
}


void Room_Disable(room_p room)
{
    int i;
    
    if(!room->active)
    {
        return;
    }
    
    if(room->bt_body)
    {
        bt_engine_dynamicsWorld->removeRigidBody(room->bt_body);
    }
    for(i=0;i<room->static_mesh_count;i++)
    {
        if(room->static_mesh[i].bt_body)
        {
            bt_engine_dynamicsWorld->removeRigidBody(room->static_mesh[i].bt_body);
        }
    }
    
    room->active = 0;
}


void Room_SwapAlternate(room_p room)
{
    /*if(room->alternate_room)
    {
        if(room->active)
        {
            Room_Disable(room);
            Room_Enable(room->alternate_room);
        }
        else
        {
            Room_Enable(room);
            Room_Disable(room->alternate_room);
        }
    }*/
}


int World_AddEntity(world_p world, struct entity_s *entity)
{
    RB_InsertIgnore(&entity->ID, entity, world->entity_tree);
    return 1;
}


int World_DeleteEntity(world_p world, struct entity_s *entity)
{
    RB_Delete(world->entity_tree, RB_SearchNode(&entity->ID, world->entity_tree));
    return 1;
}


struct skeletal_model_s* World_FindModelByID(world_p w, uint32_t id)
{
    long int i, min, max;

    min = 0;
    max = w->skeletal_model_count - 1;
    if(w->skeletal_models[min].ID == id)
    {
        return w->skeletal_models + min;
    }
    if(w->skeletal_models[max].ID == id)
    {
        return w->skeletal_models + max;
    }
    do
    {
        i = (min + max) / 2;
        if(w->skeletal_models[i].ID == id)
        {
            return w->skeletal_models + i;
        }

        if(w->skeletal_models[i].ID < id)
        {
            min = i;
        }
        else
        {
            max = i;
        }
    }
    while(min < max - 1);

    return NULL;
}


/*
 * find sprite by ID.
 * not a binary search - sprites may be not sorted by ID
 */
struct sprite_s* World_FindSpriteByID(unsigned int ID, world_p world)
{
    int i;
    sprite_p sp;

    sp = world->sprites;
    for(i=0;i<world->sprites_count;i++,sp++)
    {
        if(sp->ID == ID)
        {
            return sp;
        }
    }

    return NULL;
}


/*
 * Check for join portals existing
 */
int Room_IsJoined(room_p r1, room_p r2)
{
    unsigned short int i;
    portal_p p;

    if(r1->portal_count <= r2->portal_count)
    {
        p = r1->portals;
        for(i=0;i<r1->portal_count;i++,p++)
        {
            if(p->dest_room->ID == r2->ID)
            {
                return 1;
            }
        }
    }
    else
    {
        p = r2->portals;
        for(i=0;i<r2->portal_count;i++,p++)
        {
            if(p->dest_room->ID == r1->ID)
            {
                return 1;
            }
        }
    }

    return 0;
}

