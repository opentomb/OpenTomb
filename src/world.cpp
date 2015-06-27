
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

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
#include "obb.h"
#include "console.h"
#include "resource.h"
#include "bsp_tree.h"

void Room_Empty(std::shared_ptr<Room> room)
{
    btRigidBody* body;

    if(room == NULL)
    {
        return;
    }

    room->containers.clear();

    room->near_room_list_size = 0;

    room->portals.clear();

    room->frustum.clear();

    room->mesh.reset();

    if(!room->static_mesh.empty())
    {
        for(uint32_t i=0;i<room->static_mesh.size();i++)
        {
            body = room->static_mesh[i]->bt_body;
            if(body)
            {
                body->setUserPointer(NULL);
                if(body->getMotionState())
                {
                    delete body->getMotionState();
                    body->setMotionState(NULL);
                }
                if(body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                    body->setCollisionShape(NULL);
                }

                bt_engine_dynamicsWorld->removeRigidBody(body);
                delete body;
                room->static_mesh[i]->bt_body = NULL;
            }

            delete room->static_mesh[i]->obb;
            room->static_mesh[i]->obb = nullptr;
            if(room->static_mesh[i]->self)
            {
                room->static_mesh[i]->self->room = NULL;
                free(room->static_mesh[i]->self);
                room->static_mesh[i]->self = NULL;
            }
        }
        room->static_mesh.clear();
    }

    if(room->bt_body)
    {
        body = room->bt_body;
        if(body)
        {
            body->setUserPointer(NULL);
            if(body->getMotionState())
            {
                delete body->getMotionState();
                body->setMotionState(NULL);
            }
            if(body->getCollisionShape())
            {
                delete body->getCollisionShape();
                body->setCollisionShape(NULL);
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

    if(room->light_count)
    {
        free(room->lights);
        room->lights = NULL;
        room->light_count = 0;
    }

    if(room->self)
    {
        room->self->room = NULL;
        free(room->self);
        room->self = NULL;
    }
}


void Room_AddEntity(std::shared_ptr<Room> room, std::shared_ptr<Entity> entity)
{
    for(const std::shared_ptr<EngineContainer>& curr : room->containers)
    {
        if(curr == entity->m_self)
        {
            return;
        }
    }

    entity->m_self->room = room;
    room->containers.insert(room->containers.begin(), entity->m_self);
}


bool Room_RemoveEntity(std::shared_ptr<Room> room, std::shared_ptr<Entity> entity)
{
    if(!entity || room->containers.empty())
        return false;

    auto it = std::find( room->containers.begin(), room->containers.end(), entity->m_self );
    if(it != room->containers.end()) {
        room->containers.erase(it);
        entity->m_self->room.reset();
        return true;
    }

    if(room->containers.front() == entity->m_self)
    {
        room->containers.erase(room->containers.begin());
        entity->m_self->room.reset();
        return 1;
    }

    return false;
}


void Room_AddToNearRoomsList(std::shared_ptr<Room> room, std::shared_ptr<Room> r)
{
    if(room && r && !Room_IsInNearRoomsList(room, r) && room->id != r->id && !Room_IsOverlapped(room, r) && room->near_room_list_size < 64)
    {
        room->near_room_list[room->near_room_list_size] = r;
        room->near_room_list_size++;
    }
}


int Room_IsInNearRoomsList(std::shared_ptr<Room> r0, std::shared_ptr<Room> r1)
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


int Room_HasSector(std::shared_ptr<Room> room, int x, int y)
{
    if(x < room->sectors_x && y < room->sectors_y )
    {
        return 1;
    }

    return 0;
}


room_sector_p TR_Sector_CheckPortalPointerRaw(room_sector_p rs)
{
    if((rs != NULL) && (rs->portal_to_room >= 0))
    {
        std::shared_ptr<Room> r = engine_world.rooms[rs->portal_to_room];
        int ind_x = (rs->pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


room_sector_p TR_Sector_CheckPortalPointer(room_sector_p rs)
{
    if((rs != NULL) && (rs->portal_to_room >= 0))
    {
        std::shared_ptr<Room> r = engine_world.rooms[ rs->portal_to_room ];
        if((rs->owner_room->base_room != NULL) && (r->alternate_room != NULL))
        {
            r = r->alternate_room;
        }
        else if((rs->owner_room->alternate_room != NULL) && (r->base_room != NULL))
        {
            r = r->base_room;
        }
        int ind_x = (rs->pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


room_sector_p TR_Sector_CheckBaseRoom(room_sector_p rs)
{
    if((rs != NULL) && (rs->owner_room->base_room != NULL))
    {
        std::shared_ptr<Room> r = rs->owner_room->base_room;
        int ind_x = (rs->pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


room_sector_p TR_Sector_CheckAlternateRoom(room_sector_p rs)
{
    if((rs != NULL) && (rs->owner_room->alternate_room != NULL))
    {
        std::shared_ptr<Room> r = rs->owner_room->alternate_room;
        int ind_x = (rs->pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (rs->pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            rs = r->sectors + (ind_x * r->sectors_y + ind_y);
        }
    }

    return rs;
}


int Sectors_Is2SidePortals(room_sector_p s1, room_sector_p s2)
{
    s1 = TR_Sector_CheckPortalPointer(s1);
    s2 = TR_Sector_CheckPortalPointer(s2);

    if(s1->owner_room == s2->owner_room)
    {
        return 0;
    }

    room_sector_p s1p = Room_GetSectorRaw(s2->owner_room, s1->pos);
    room_sector_p s2p = Room_GetSectorRaw(s1->owner_room, s2->pos);

    // 2 next conditions are the stick for TR_V door-roll-wall
    if(s1p->portal_to_room < 0)
    {
        s1p = TR_Sector_CheckAlternateRoom(s1p);
        if(s1p->portal_to_room < 0)
        {
            return 0;
        }
    }
    if(s2p->portal_to_room < 0)
    {
        s2p = TR_Sector_CheckAlternateRoom(s2p);
        if(s2p->portal_to_room < 0)
        {
            return 0;
        }
    }

    if((TR_Sector_CheckPortalPointer(s1p) == TR_Sector_CheckBaseRoom(s1)) && (TR_Sector_CheckPortalPointer(s2p) == TR_Sector_CheckBaseRoom(s2)) ||
       (TR_Sector_CheckPortalPointer(s1p) == TR_Sector_CheckAlternateRoom(s1)) && (TR_Sector_CheckPortalPointer(s2p) == TR_Sector_CheckAlternateRoom(s2)))
    {
        return 1;
    }

    return 0;
}


int Room_IsOverlapped(std::shared_ptr<Room> r0, std::shared_ptr<Room> r1)
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


void World_Prepare(world_p world)
{
    world->id = 0;
    world->name = NULL;
    world->type = 0x00;
    world->meshes.clear();
    world->sprites = NULL;
    world->sprites_count = 0;
    world->rooms.clear();
    world->flip_map = NULL;
    world->flip_state = NULL;
    world->flip_count = 0;
    world->textures = NULL;
    world->type = 0;
    world->entity_tree.clear();
    world->items_tree.clear();
    world->Character = NULL;

    world->audio_sources.clear();
    world->audio_buffers.clear();
    world->audio_effects.clear();
    world->anim_sequences = NULL;
    world->anim_sequences_count = 0;
    world->stream_tracks.clear();
    world->stream_track_map.clear();

    world->tex_count = 0;
    world->textures = 0;
    world->room_boxes = NULL;
    world->room_box_count = 0;
    world->cameras_sinks.clear();
    world->skeletal_models = NULL;
    world->skeletal_model_count = 0;
    world->sky_box = NULL;
    world->anim_commands = NULL;
    world->anim_commands_count = 0;
}


void World_Empty(world_p world)
{
    extern EngineContainer* last_cont;

    last_cont = NULL;
    Engine_LuaClearTasks();
    // De-initialize and destroy all audio objects.
    Audio_DeInit();

    if(main_inventory_manager != NULL)
    {
        main_inventory_manager->setInventory(NULL);
        main_inventory_manager->setItemsType(1);                                // see base items
    }

    if(world->Character != NULL)
    {
        world->Character->m_self->room.reset();
        world->Character->m_currentSector = nullptr;
    }

    /* entity empty must be done before rooms destroy */
    world->entity_tree.clear();

    /* Now we can delete bullet misc */
    if(bt_engine_dynamicsWorld != NULL)
    {
        for(int i=bt_engine_dynamicsWorld->getNumCollisionObjects()-1;i>=0;i--)
        {
            btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if(body != NULL)
            {
                EngineContainer* cont = (EngineContainer*)body->getUserPointer();
                body->setUserPointer(NULL);

                if(cont && (cont->object_type == OBJECT_BULLET_MISC))
                {
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(NULL);
                    }

                    if(body->getCollisionShape())
                    {
                        delete body->getCollisionShape();
                        body->setCollisionShape(NULL);
                    }

                    bt_engine_dynamicsWorld->removeRigidBody(body);
                    cont->room = NULL;
                    free(cont);
                    delete body;
                }
            }
        }
    }

    for(auto room : world->rooms)
    {
        Room_Empty(room);
    }
    world->rooms.clear();

    free(world->flip_map);
    free(world->flip_state);
    world->flip_map = NULL;
    world->flip_state = NULL;
    world->flip_count = 0;

    if(world->room_box_count)
    {
        free(world->room_boxes);
        world->room_boxes = NULL;
        world->room_box_count = 0;
    }

    world->cameras_sinks.clear();

    /*sprite empty*/
    if(world->sprites_count)
    {
        free(world->sprites);
        world->sprites = NULL;
        world->sprites_count = 0;
    }

    /*items empty*/
    world->items_tree.clear();

    world->Character.reset();

    if(world->skeletal_model_count)
    {
        for(uint32_t i=0;i<world->skeletal_model_count;i++)
        {
            world->skeletal_models[i].clear();
        }
        free(world->skeletal_models);
        world->skeletal_models = NULL;
        world->skeletal_model_count = 0;
    }

    /*mesh empty*/

    world->meshes.clear();

    if(world->tex_count)
    {
        glDeleteTextures(world->tex_count ,world->textures);
        world->tex_count = 0;
        free(world->textures);
        world->textures = NULL;
    }

    if(world->tex_atlas)
    {
        delete world->tex_atlas;
        world->tex_atlas = NULL;
    }

    if(world->anim_sequences_count)
    {
        for(uint32_t i=0;i < world->anim_sequences_count;i++)
        {
            world->anim_sequences[i].frames.clear();
        }
        world->anim_sequences_count = 0;
        free(world->anim_sequences);
        world->anim_sequences = NULL;
    }
}


int compEntityEQ(void *x, void *y)
{
    return (*((uint32_t*)x) == *((uint32_t*)y));
}


int compEntityLT(void *x, void *y)
{
    return (*((uint32_t*)x) < *((uint32_t*)y));
}


uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, const btVector3* pos, const btVector3* ang, int32_t id)
{
    if(!engine_world.entity_tree.empty())
    {
        SkeletalModel* model = World_GetModelByID(&engine_world, model_id);
        if(model != NULL)
        {
            std::shared_ptr<Entity> ent = World_GetEntityByID(&engine_world, id);

            if(ent != NULL)
            {
                if(pos != NULL)
                {
                    ent->m_transform.getOrigin() = *pos;
                }
                if(ang != NULL)
                {
                    ent->m_angles = *ang;
                    ent->updateRotation();
                }
                if(room_id < engine_world.rooms.size())
                {
                    ent->m_self->room = engine_world.rooms[ room_id ];
                    ent->m_currentSector = Room_GetSectorRaw(ent->m_self->room, ent->m_transform.getOrigin());
                }
                else
                {
                    ent->m_self->room = NULL;
                }

                return ent->m_id;
            }

            ent = std::make_shared<Entity>();

            if(id < 0)
            {
                ent->m_id = engine_world.entity_tree.size();
                engine_world.entity_tree[id] = ent;
            }
            else
            {
                ent->m_id = id;
            }

            if(pos != NULL)
            {
                ent->m_transform.getOrigin() = *pos;
            }
            if(ang != NULL)
            {
                ent->m_angles = *ang;
                ent->updateRotation();
            }
            if(room_id < engine_world.rooms.size())
            {
                ent->m_self->room = engine_world.rooms[ room_id ];
                ent->m_currentSector = Room_GetSectorRaw(ent->m_self->room, ent->m_transform.getOrigin());
            }
            else
            {
                ent->m_self->room = NULL;
            }

            ent->m_typeFlags     = ENTITY_TYPE_SPAWNED;
            ent->m_stateFlags    = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
            ent->m_triggerLayout = 0x00;
            ent->m_OCB            = 0x00;
            ent->m_timer          = 0.0;

            ent->m_self->collide_flag = 0x00;
            ent->m_moveType          = 0x0000;
            ent->m_inertiaLinear     = 0.0;
            ent->m_inertiaAngular[0] = 0.0;
            ent->m_inertiaAngular[1] = 0.0;
            ent->m_moveType          = 0;

            ent->m_bf.fromModel(model);
            ent->setAnimation(0, 0);                                     // Set zero animation and zero frame
            ent->genEntityRigidBody();

            ent->rebuildBV();
            if(ent->m_self->room != NULL)
            {
                Room_AddEntity(ent->m_self->room, ent);
            }
            World_AddEntity(&engine_world, ent);

            return ent->m_id;
        }
    }

    return 0xFFFFFFFF;
}


std::shared_ptr<Entity> World_GetEntityByID(world_p world, uint32_t id)
{
    if(!world)
        return nullptr;

    auto it = world->entity_tree.find(id);
    if(it==world->entity_tree.end())
        return nullptr;
    else
        return it->second;
}


std::shared_ptr<base_item_s> World_GetBaseItemByID(world_p world, uint32_t id)
{
    if(!world)
        return nullptr;

    auto it = world->items_tree.find(id);
    if(it==world->items_tree.end())
        return nullptr;
    else
        return it->second;
}


inline int Room_IsPointIn(std::shared_ptr<Room> room, btScalar dot[3])
{
    return (dot[0] >= room->bb_min[0]) && (dot[0] < room->bb_max[0]) &&
           (dot[1] >= room->bb_min[1]) && (dot[1] < room->bb_max[1]) &&
           (dot[2] >= room->bb_min[2]) && (dot[2] < room->bb_max[2]);
}


std::shared_ptr<Room> Room_FindPos(const btVector3& pos)
{
    for(auto r : engine_world.rooms)
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


std::shared_ptr<Room> Room_FindPosCogerrence(const btVector3 &new_pos, std::shared_ptr<Room> room)
{
    if(room == NULL)
    {
        return Room_FindPos(new_pos);
    }

    if(room->active &&
       (new_pos[0] >= room->bb_min[0]) && (new_pos[0] < room->bb_max[0]) &&
       (new_pos[1] >= room->bb_min[1]) && (new_pos[1] < room->bb_max[1]) &&
       (new_pos[2] >= room->bb_min[2]) && (new_pos[2] < room->bb_max[2]))
    {
        return room;
    }

    room_sector_p new_sector = Room_GetSectorRaw(room, new_pos);
    if((new_sector != NULL) && (new_sector->portal_to_room >= 0))
    {
        return Room_CheckFlip(engine_world.rooms[new_sector->portal_to_room]);
    }

    for(uint16_t i=0;i<room->near_room_list_size;i++)
    {
        std::shared_ptr<Room> r = room->near_room_list[i];
        if(r->active &&
           (new_pos[0] >= r->bb_min[0]) && (new_pos[0] < r->bb_max[0]) &&
           (new_pos[1] >= r->bb_min[1]) && (new_pos[1] < r->bb_max[1]) &&
           (new_pos[2] >= r->bb_min[2]) && (new_pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }

    return Room_FindPos(new_pos);
}


std::shared_ptr<Room> Room_GetByID(world_p w, unsigned int ID)
{
    for(auto r : w->rooms)
    {
        if(ID == r->id)
        {
            return r;
        }
    }
    return NULL;
}


room_sector_p Room_GetSectorRaw(std::shared_ptr<Room> room, const btVector3& pos)
{
    int x, y;
    room_sector_p ret = NULL;

    if((room == NULL) || !room->active)
    {
        return NULL;
    }

    x = (int)(pos[0] - room->transform.getOrigin()[0]) / 1024;
    y = (int)(pos[1] - room->transform.getOrigin()[1]) / 1024;
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


room_sector_p Room_GetSectorCheckFlip(std::shared_ptr<Room> room, btScalar pos[3])
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

    x = (int)(pos[0] - room->transform.getOrigin()[0]) / 1024;
    y = (int)(pos[1] - room->transform.getOrigin()[1]) / 1024;
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


room_sector_p Sector_CheckFlip(room_sector_p rs)
{
    if((rs != NULL) && (rs->owner_room->active == 0))
    {
        if((rs->owner_room->base_room != NULL) && (rs->owner_room->base_room->active))
        {
            std::shared_ptr<Room> r = rs->owner_room->base_room;
            rs = r->sectors + rs->index_x * r->sectors_y + rs->index_y;
        }
        else if((rs->owner_room->alternate_room != NULL) && (rs->owner_room->alternate_room->active))
        {
            std::shared_ptr<Room> r = rs->owner_room->alternate_room;
            rs = r->sectors + rs->index_x * r->sectors_y + rs->index_y;
        }
    }

    return rs;
}


room_sector_p Room_GetSectorXYZ(std::shared_ptr<Room> room, const btVector3& pos)
{
    int x, y;
    room_sector_p ret = NULL;

    room = Room_CheckFlip(room);

    if(!room->active)
    {
        return NULL;
    }

    x = (int)(pos[0] - room->transform.getOrigin()[0]) / 1024;
    y = (int)(pos[1] - room->transform.getOrigin()[1]) / 1024;
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


void Room_Enable(std::shared_ptr<Room> room)
{
    if(room->active)
    {
        return;
    }

    if(room->bt_body != NULL)
    {
        bt_engine_dynamicsWorld->addRigidBody(room->bt_body);
    }

    for(auto sm : room->static_mesh)
    {
        if(sm->bt_body != NULL)
        {
            bt_engine_dynamicsWorld->addRigidBody(sm->bt_body);
        }
    }

    for(const std::shared_ptr<EngineContainer>& cont : room->containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                std::static_pointer_cast<Entity>(cont->object)->enable();
                break;
        }
    }

    room->active = 1;
}


void Room_Disable(std::shared_ptr<Room> room)
{
    if(!room->active)
    {
        return;
    }

    if(room->bt_body != NULL)
    {
        bt_engine_dynamicsWorld->removeRigidBody(room->bt_body);
    }

    for(auto sm : room->static_mesh)
    {
        if(sm->bt_body != NULL)
        {
            bt_engine_dynamicsWorld->removeRigidBody(sm->bt_body);
        }
    }

    for(const std::shared_ptr<EngineContainer>& cont : room->containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                std::static_pointer_cast<Entity>(cont->object)->disable();
                break;
        }
    }

    room->active = 0;
}

void Room_SwapToBase(std::shared_ptr<Room> room)
{
    if((room->base_room != NULL) && (room->active == 1))                        //If room is active alternate room
    {
        Render_CleanList();
        Room_Disable(room);                             //Disable current room
        Room_Disable(room->base_room);                  //Paranoid
        Room_SwapPortals(room, room->base_room);        //Update portals to match this room
        Room_SwapItems(room, room->base_room);     //Update items to match this room
        Room_Enable(room->base_room);                   //Enable original room
    }
}

void Room_SwapToAlternate(std::shared_ptr<Room> room)
{
    if((room->alternate_room != NULL) && (room->active == 1))              //If room is active base room
    {
        Render_CleanList();
        Room_Disable(room);                             //Disable current room
        Room_Disable(room->alternate_room);             //Paranoid
        Room_SwapPortals(room, room->alternate_room);   //Update portals to match this room
        Room_SwapItems(room, room->alternate_room);          //Update items to match this room
        Room_Enable(room->alternate_room);                              //Enable base room
    }
}

std::shared_ptr<Room> Room_CheckFlip(std::shared_ptr<Room> r)
{
    if((r != NULL) && (r->active == 0))
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

void Room_SwapPortals(std::shared_ptr<Room> room, std::shared_ptr<Room> dest_room)
{
    //Update portals in room rooms
    for(auto r : engine_world.rooms)//For every room in the world itself
    {
        for(portal_s& p : r->portals) //For every portal in this room
        {
            if(p.dest_room->id == room->id)//If a portal is linked to the input room
            {
                p.dest_room = dest_room;//The portal destination room is the destination room!
                //Con_Printf("The current room %d! has room %d joined to it!", room->id, i);
            }
        }
        Room_BuildNearRoomsList(r);//Rebuild room near list!
    }
}

void Room_SwapItems(std::shared_ptr<Room> room, std::shared_ptr<Room> dest_room)
{
    for(std::shared_ptr<EngineContainer> t : room->containers)
    {
        t->room = dest_room;
    }

    for(std::shared_ptr<EngineContainer> t : dest_room->containers)
    {
        t->room = room;
    }

    std::swap(room->containers, dest_room->containers);
}

int World_AddEntity(world_p world, std::shared_ptr<Entity> entity)
{
    if(world->entity_tree.find(entity->m_id) != world->entity_tree.end())
        return 1;
    world->entity_tree[entity->m_id] = entity;
    return 1;
}


int World_CreateItem(world_p world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name)
{
    SkeletalModel* model = World_GetModelByID(world, model_id);
    if((model == NULL) || (world->items_tree.empty()))
    {
        return 0;
    }

    std::unique_ptr<SSBoneFrame> bf(new SSBoneFrame());
    bf->fromModel(model);

    auto item = std::make_shared<base_item_s>();
    item->id = item_id;
    item->world_model_id = world_model_id;
    item->type = type;
    item->count = count;
    item->name[0] = 0;
    if(name)
    {
        strncpy(item->name, name, 64);
    }
    item->bf = std::move(bf);

    world->items_tree[item->id] = item;

    return 1;
}


int World_DeleteItem(world_p world, uint32_t item_id)
{
    world->items_tree.erase(world->items_tree.find(item_id));
    return 1;
}


struct SkeletalModel* World_GetModelByID(world_p w, uint32_t id)
{
    long int i, min, max;

    min = 0;
    max = w->skeletal_model_count - 1;
    if(w->skeletal_models[min].id == id)
    {
        return w->skeletal_models + min;
    }
    if(w->skeletal_models[max].id == id)
    {
        return w->skeletal_models + max;
    }
    do
    {
        i = (min + max) / 2;
        if(w->skeletal_models[i].id == id)
        {
            return w->skeletal_models + i;
        }

        if(w->skeletal_models[i].id < id)
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
struct Sprite* World_GetSpriteByID(unsigned int ID, world_p world)
{
    Sprite* sp = world->sprites;
    for(uint32_t i=0;i<world->sprites_count;i++,sp++)
    {
        if(sp->id == ID)
        {
            return sp;
        }
    }

    return NULL;
}


/*
 * Check for join portals existing
 */
bool Room_IsJoined(std::shared_ptr<Room> r1, std::shared_ptr<Room> r2)
{
    for(const portal_s& p : r1->portals)
    {
        if(p.dest_room->id == r2->id)
        {
            return true;
        }
    }

    for(const portal_s& p : r2->portals)
    {
        if(p.dest_room->id == r1->id)
        {
            return true;
        }
    }

    return false;
}

void Room_BuildNearRoomsList(std::shared_ptr<Room> room)
{
    room->near_room_list_size = 0;

    for(const portal_s& p : room->portals)
    {
        Room_AddToNearRoomsList(room, p.dest_room);
    }

    uint16_t nc1 = room->near_room_list_size;

    for(uint16_t i=0;i<nc1;i++)
    {
        std::shared_ptr<Room> r = room->near_room_list[i];
        for(const portal_s& p : r->portals)
        {
            Room_AddToNearRoomsList(room, p.dest_room);
        }
    }
}

void Room_BuildOverlappedRoomsList(std::shared_ptr<Room> room)
{
    room->overlapped_room_list_size = 0;

    for(auto r : engine_world.rooms)
    {
        if(Room_IsOverlapped(room, r))
        {
            room->overlapped_room_list[room->overlapped_room_list_size] = r;
            room->overlapped_room_list_size++;
        }
    }
}

base_item_s::~base_item_s() {
    bf->bone_tags.clear();
}
