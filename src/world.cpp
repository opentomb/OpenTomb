#include "world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "audio/audio.h"
#include "camera.h"
#include "gui/console.h"
#include "engine.h"
#include "entity.h"
#include "mesh.h"
#include "obb.h"
#include "polygon.h"
#include "portal.h"
#include "render.h"
#include "resource.h"
#include "script/script.h"
#include "util/vmath.h"

void Room::empty()
{
    containers.clear();

    near_room_list.clear();

    portals.clear();

    frustum.clear();

    mesh.reset();

    if(!static_mesh.empty())
    {
        for(uint32_t i = 0; i < static_mesh.size(); i++)
        {
            if(btRigidBody* body = static_mesh[i]->bt_body)
            {
                body->setUserPointer(nullptr);
                if(body->getMotionState())
                {
                    delete body->getMotionState();
                    body->setMotionState(nullptr);
                }
                body->setCollisionShape(nullptr);

                bt_engine_dynamicsWorld->removeRigidBody(body);
                delete body;
                static_mesh[i]->bt_body = nullptr;
            }

            if(static_mesh[i]->self)
            {
                static_mesh[i]->self->room = nullptr;
                static_mesh[i]->self.reset();
            }
        }
        static_mesh.clear();
    }

    if(bt_body)
    {
        bt_body->setUserPointer(nullptr);
        if(bt_body->getMotionState())
        {
            delete bt_body->getMotionState();
            bt_body->setMotionState(nullptr);
        }
        if(bt_body->getCollisionShape())
        {
            delete bt_body->getCollisionShape();
            bt_body->setCollisionShape(nullptr);
        }

        bt_engine_dynamicsWorld->removeRigidBody(bt_body.get());
        bt_body.reset();
    }

    sectors.clear();
    sectors_x = 0;
    sectors_y = 0;

    sprites.clear();

    lights.clear();

    self.reset();
}

void Room::addEntity(Entity* entity)
{
    for(const std::shared_ptr<EngineContainer>& curr : containers)
    {
        if(curr == entity->m_self)
        {
            return;
        }
    }

    entity->m_self->room = this;
    containers.insert(containers.begin(), entity->m_self);
}

bool Room::removeEntity(Entity* entity)
{
    if(!entity || containers.empty())
        return false;

    auto it = std::find(containers.begin(), containers.end(), entity->m_self);
    if(it != containers.end())
    {
        containers.erase(it);
        entity->m_self->room = nullptr;
        return true;
    }

    if(containers.front() == entity->m_self)
    {
        containers.erase(containers.begin());
        entity->m_self->room = nullptr;
        return true;
    }

    return false;
}

void Room::addToNearRoomsList(std::shared_ptr<Room> r)
{
    if(r && !isInNearRoomsList(*r) && id != r->id && !isOverlapped(r.get()))
    {
        near_room_list.push_back(r);
    }
}

bool Room::isInNearRoomsList(const Room& r1) const
{
    if(id == r1.id)
    {
        return true;
    }

    if(r1.near_room_list.size() >= near_room_list.size())
    {
        for(const std::shared_ptr<Room>& r : near_room_list)
        {
            if(r->id == r1.id)
            {
                return true;
            }
        }
    }
    else
    {
        for(const std::shared_ptr<Room>& r : r1.near_room_list)
        {
            if(r->id == id)
            {
                return true;
            }
        }
    }
    return false;
}

bool Room::hasSector(int x, int y)
{
    return x < sectors_x && y < sectors_y;
}

RoomSector* RoomSector::checkPortalPointerRaw()
{
    if(portal_to_room >= 0)
    {
        std::shared_ptr<Room> r = engine_world.rooms[portal_to_room];
        int ind_x = (pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            return &r->sectors[(ind_x * r->sectors_y + ind_y)];
        }
    }

    return this;
}

RoomSector* RoomSector::checkPortalPointer()
{
    if(portal_to_room >= 0)
    {
        std::shared_ptr<Room> r = engine_world.rooms[portal_to_room];
        if((owner_room->base_room != nullptr) && (r->alternate_room != nullptr))
        {
            r = r->alternate_room;
        }
        else if((owner_room->alternate_room != nullptr) && (r->base_room != nullptr))
        {
            r = r->base_room;
        }
        int ind_x = (pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            return &r->sectors[(ind_x * r->sectors_y + ind_y)];
        }
    }

    return this;
}

RoomSector* RoomSector::checkBaseRoom()
{
    if(owner_room->base_room != nullptr)
    {
        std::shared_ptr<Room> r = owner_room->base_room;
        int ind_x = (pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            return &r->sectors[(ind_x * r->sectors_y + ind_y)];
        }
    }

    return this;
}

RoomSector* RoomSector::checkAlternateRoom()
{
    if(owner_room->alternate_room != nullptr)
    {
        std::shared_ptr<Room> r = owner_room->alternate_room;
        int ind_x = (pos[0] - r->transform.getOrigin()[0]) / TR_METERING_SECTORSIZE;
        int ind_y = (pos[1] - r->transform.getOrigin()[1]) / TR_METERING_SECTORSIZE;
        if((ind_x >= 0) && (ind_x < r->sectors_x) && (ind_y >= 0) && (ind_y < r->sectors_y))
        {
            return &r->sectors[(ind_x * r->sectors_y + ind_y)];
        }
    }

    return this;
}

bool RoomSector::is2SidePortals(RoomSector* s2)
{
    RoomSector* s1 = checkPortalPointer();
    s2 = s2->checkPortalPointer();

    if(owner_room == s2->owner_room)
    {
        return false;
    }

    RoomSector* s1p = s2->owner_room->getSectorRaw(pos);
    RoomSector* s2p = s1->owner_room->getSectorRaw(s2->pos);

    // 2 next conditions are the stick for TR5 door-roll-wall
    if(s1p->portal_to_room < 0)
    {
        s1p = s1p->checkAlternateRoom();
        if(s1p->portal_to_room < 0)
        {
            return false;
        }
    }
    if(s2p->portal_to_room < 0)
    {
        s2p = s2p->checkAlternateRoom();
        if(s2p->portal_to_room < 0)
        {
            return false;
        }
    }

    if(((s1p->checkPortalPointer() == s1->checkBaseRoom()) && (s2p->checkPortalPointer() == s2->checkBaseRoom())) ||
       ((s1p->checkPortalPointer() == s1->checkAlternateRoom()) && (s2p->checkPortalPointer() == s2->checkAlternateRoom())))
    {
        return true;
    }

    return false;
}

bool RoomSector::similarCeiling(RoomSector* s2, bool ignore_doors)
{
    if(!s2) return false;
    if(this == s2) return true;

    if((ceiling != s2->ceiling) ||
       (ceiling_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
       (s2->ceiling_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
       (!ignore_doors && (sector_above || s2->sector_above)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(ceiling_corners->m_floats[2] != s2->ceiling_corners->m_floats[2]) return false;
    }

    return true;
}

bool RoomSector::similarFloor(RoomSector* s2, bool ignore_doors)
{
    if(!s2) return false;
    if(this == s2) return true;

    if((floor != s2->floor) ||
       (floor_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
       (s2->floor_penetration_config == TR_PENETRATION_CONFIG_WALL) ||
       (!ignore_doors && (sector_below || s2->sector_below)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(floor_corners->m_floats[2] != s2->floor_corners->m_floats[2]) return false;
    }

    return true;
}

btVector3 Sector_HighestFloorCorner(RoomSector* rs)
{
    assert(rs != nullptr);
    btVector3 r1 = (rs->floor_corners[0][2] > rs->floor_corners[1][2]) ? (rs->floor_corners[0]) : (rs->floor_corners[1]);
    btVector3 r2 = (rs->floor_corners[2][2] > rs->floor_corners[3][2]) ? (rs->floor_corners[2]) : (rs->floor_corners[3]);

    return (r1[2] > r2[2]) ? (r1) : (r2);
}

btVector3 Sector_LowestCeilingCorner(RoomSector* rs)
{
    assert(rs != nullptr);
    btVector3 r1 = (rs->ceiling_corners[0][2] > rs->ceiling_corners[1][2]) ? (rs->ceiling_corners[1]) : (rs->ceiling_corners[0]);
    btVector3 r2 = (rs->ceiling_corners[2][2] > rs->ceiling_corners[3][2]) ? (rs->ceiling_corners[3]) : (rs->ceiling_corners[2]);

    return (r1[2] > r2[2]) ? (r2) : (r1);
}

btVector3 RoomSector::getFloorPoint()
{
    return Sector_HighestFloorCorner(getLowestSector());
}

btVector3 RoomSector::getCeilingPoint()
{
    return Sector_LowestCeilingCorner(getHighestSector());
}

bool Room::isOverlapped(Room* r1)
{
	assert( r1 != nullptr );
    if((this == r1) || (this == r1->alternate_room.get()) || (alternate_room.get() == r1))
    {
        return false;
    }

    if(bb_min[0] >= r1->bb_max[0] || bb_max[0] <= r1->bb_min[0] ||
       bb_min[1] >= r1->bb_max[1] || bb_max[1] <= r1->bb_min[1] ||
       bb_min[2] >= r1->bb_max[2] || bb_max[2] <= r1->bb_min[2])
    {
        return false;
    }

    return !isJoined(r1);
}

void World::prepare()
{
    id = 0;
    name = nullptr;
    type = 0x00;
    meshes.clear();
    sprites.clear();
    rooms.clear();
    flip_data.clear();
    textures.clear();
    entity_tree.clear();
    items_tree.clear();
    character.reset();

    audio_sources.clear();
    audio_buffers.clear();
    audio_effects.clear();
    anim_sequences.clear();
    stream_tracks.clear();
    stream_track_map.clear();

    room_boxes.clear();
    cameras_sinks.clear();
    skeletal_models.clear();
    sky_box = nullptr;
    anim_commands.clear();
}

extern EngineContainer* last_cont;

void World::empty()
{
    last_cont = nullptr;
    engine_lua.clearTasks();

    audio::deInit(); // De-initialize and destroy all audio objects.

    if(gui::main_inventory_manager != nullptr)
    {
        gui::main_inventory_manager->setInventory(nullptr);
        gui::main_inventory_manager->setItemsType(gui::MenuItemType::Supply);  // see base items
    }

    if(character)
    {
        character->m_self->room = nullptr;
        character->m_currentSector = nullptr;
    }

    entity_tree.clear();  // Clearing up entities must happen before destroying rooms.

    // Destroy Bullet's MISC objects (debug spheres etc.)
    ///@FIXME: Hide it somewhere, it is nasty being here.

    if(bt_engine_dynamicsWorld != nullptr)
    {
        for(int i = bt_engine_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
            if(btRigidBody* body = btRigidBody::upcast(obj))
            {
                EngineContainer* cont = static_cast<EngineContainer*>(body->getUserPointer());
                body->setUserPointer(nullptr);

                if(cont && (cont->object_type == OBJECT_BULLET_MISC))
                {
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(nullptr);
                    }

                    body->setCollisionShape(nullptr);

                    bt_engine_dynamicsWorld->removeRigidBody(body);
                    cont->room = nullptr;
                    delete cont;
                    delete body;
                }
            }
        }
    }

    for(auto room : rooms)
    { room->empty(); }
    rooms.clear();

    flip_data.clear();
    room_boxes.clear();
    cameras_sinks.clear();
    sprites.clear();
    items_tree.clear();
    character.reset();
    skeletal_models.clear();
    meshes.clear();

    glDeleteTextures(textures.size(), textures.data());
    textures.clear();

    tex_atlas.reset();
    anim_sequences.clear();
}

bool World::deleteEntity(uint32_t id)
{
    if(character->id() == id) return false;

    auto it = entity_tree.find(id);
    if(it == entity_tree.end())
    {
        return false;
    }
    else
    {
        entity_tree.erase(it);
        return true;
    }
}

uint32_t World::spawnEntity(uint32_t model_id, uint32_t room_id, const btVector3* pos, const btVector3* ang, int32_t id)
{
    if(SkeletalModel* model = getModelByID(model_id))
    {
        if(std::shared_ptr<Entity> ent = getEntityByID(id))
        {
            if(pos != nullptr)
            {
                ent->m_transform.getOrigin() = *pos;
            }
            if(ang != nullptr)
            {
                ent->m_angles = *ang;
                ent->updateTransform();
            }
            if(room_id < rooms.size())
            {
                ent->m_self->room = rooms[room_id].get();
                ent->m_currentSector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
            }
            else
            {
                ent->m_self->room = nullptr;
            }

            return ent->id();
        }

        std::shared_ptr<Entity> ent;
        if(id < 0)
        {
            ent = std::make_shared<Entity>(next_entity_id);
            entity_tree[next_entity_id] = ent;
            ++next_entity_id;
        }
        else
        {
            ent = std::make_shared<Entity>(id);
            if(static_cast<uint32_t>(id + 1) > next_entity_id)
                next_entity_id = id + 1;
        }

        if(pos != nullptr)
        {
            ent->m_transform.getOrigin() = *pos;
        }
        if(ang != nullptr)
        {
            ent->m_angles = *ang;
            ent->updateTransform();
        }
        if(room_id < rooms.size())
        {
            ent->m_self->room = rooms[room_id].get();
            ent->m_currentSector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
        }
        else
        {
            ent->m_self->room = nullptr;
        }

        ent->m_typeFlags = ENTITY_TYPE_SPAWNED;
        ent->m_active = ent->m_enabled = true;
        ent->m_triggerLayout = 0x00;
        ent->m_OCB = 0x00;
        ent->m_timer = 0.0;

        ent->m_moveType = MoveType::StaticPos;
        ent->m_inertiaLinear = 0.0;
        ent->m_inertiaAngular[0] = 0.0;
        ent->m_inertiaAngular[1] = 0.0;

        ent->m_bf.fromModel(model);

        ent->setAnimation(0, 0);                                     // Set zero animation and zero frame

        Res_SetEntityProperties(ent);
        ent->rebuildBV();
        ent->genRigidBody();

        if(ent->m_self->room != nullptr)
        {
            ent->m_self->room->addEntity(ent.get());
        }
        addEntity(ent);
        Res_SetEntityFunction(ent);

        return ent->id();
    }

    return 0xFFFFFFFF;
}

std::shared_ptr<Entity> World::getEntityByID(uint32_t id)
{
    if(character->id() == id) return character;
    auto it = entity_tree.find(id);
    if(it == entity_tree.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Character> World::getCharacterByID(uint32_t id)
{
    return std::dynamic_pointer_cast<Character>(getEntityByID(id));
}

std::shared_ptr<BaseItem> World::getBaseItemByID(uint32_t id)
{
    auto it = items_tree.find(id);
    if(it == items_tree.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Room> World::findRoomByPosition(const btVector3& pos)
{
    for(auto r : rooms)
    {
        if(r->active &&
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }
    return nullptr;
}

Room* Room_FindPosCogerrence(const btVector3 &new_pos, Room* room)
{
    if(room == nullptr)
    {
        return engine_world.findRoomByPosition(new_pos).get();
    }

    if(room->active &&
       (new_pos[0] >= room->bb_min[0]) && (new_pos[0] < room->bb_max[0]) &&
       (new_pos[1] >= room->bb_min[1]) && (new_pos[1] < room->bb_max[1]))
    {
        if((new_pos[2] >= room->bb_min[2]) && (new_pos[2] < room->bb_max[2]))
        {
            return room;
        }
        else if(new_pos[2] >= room->bb_max[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_above != nullptr)
            {
                return orig_sector->sector_above->owner_room->checkFlip();
            }
        }
        else if(new_pos[2] < room->bb_min[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_below != nullptr)
            {
                return orig_sector->sector_below->owner_room->checkFlip();
            }
        }
    }

    RoomSector* new_sector = room->getSectorRaw(new_pos);
    if((new_sector != nullptr) && (new_sector->portal_to_room >= 0))
    {
        return engine_world.rooms[new_sector->portal_to_room]->checkFlip();
    }

    for(const std::shared_ptr<Room>& r : room->near_room_list)
    {
        if(r->active &&
           (new_pos[0] >= r->bb_min[0]) && (new_pos[0] < r->bb_max[0]) &&
           (new_pos[1] >= r->bb_min[1]) && (new_pos[1] < r->bb_max[1]) &&
           (new_pos[2] >= r->bb_min[2]) && (new_pos[2] < r->bb_max[2]))
        {
            return r.get();
        }
    }

    return engine_world.findRoomByPosition(new_pos).get();
}

std::shared_ptr<Room> World::getByID(unsigned int ID)
{
    for(auto r : rooms)
    {
        if(ID == r->id)
        {
            return r;
        }
    }
    return nullptr;
}

RoomSector* Room::getSectorRaw(const btVector3& pos)
{
    if(!active)
    {
        return nullptr;
    }

    int x = static_cast<int>(pos[0] - transform.getOrigin()[0]) / 1024;
    int y = static_cast<int>(pos[1] - transform.getOrigin()[1]) / 1024;
    if(x < 0 || x >= sectors_x || y < 0 || y >= sectors_y)
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    return &sectors[x * sectors_y + y];
}

RoomSector* Room_GetSectorCheckFlip(std::shared_ptr<Room> room, btScalar pos[3])
{
    int x, y;
    RoomSector* ret;

    if(room != nullptr)
    {
        if(!room->active)
        {
            if((room->base_room != nullptr) && (room->base_room->active))
            {
                room = room->base_room;
            }
            else if((room->alternate_room != nullptr) && (room->alternate_room->active))
            {
                room = room->alternate_room;
            }
        }
    }
    else
    {
        return nullptr;
    }

    if(!room->active)
    {
        return nullptr;
    }

    x = static_cast<int>(pos[0] - room->transform.getOrigin()[0]) / 1024;
    y = static_cast<int>(pos[1] - room->transform.getOrigin()[1]) / 1024;
    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    ret = &room->sectors[x * room->sectors_y + y];
    return ret;
}

RoomSector* RoomSector::checkFlip()
{
    if(owner_room->active)
        return this;

    if(owner_room->base_room && owner_room->base_room->active)
    {
        std::shared_ptr<Room> r = owner_room->base_room;
        return &r->sectors[index_x * r->sectors_y + index_y];
    }
    else if(owner_room->alternate_room && owner_room->alternate_room->active)
    {
        std::shared_ptr<Room> r = owner_room->alternate_room;
        return &r->sectors[index_x * r->sectors_y + index_y];
    }
    else
    {
        return this;
    }
}

RoomSector* Room::getSectorXYZ(const btVector3& pos)
{
    Room* room = checkFlip();

    if(!room->active)
    {
        return nullptr;
    }

    int x = static_cast<int>(pos[0] - room->transform.getOrigin()[0]) / 1024;
    int y = static_cast<int>(pos[1] - room->transform.getOrigin()[1]) / 1024;
    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    RoomSector* ret = &room->sectors[x * room->sectors_y + y];

    //resolve Z overlapped neighboard rooms. room below has more priority.

    if(ret->sector_below && (ret->sector_below->ceiling >= pos[2]))
    {
        return ret->sector_below->checkFlip();
    }

    if(ret->sector_above && (ret->sector_above->floor <= pos[2]))
    {
        return ret->sector_above->checkFlip();
    }

    return ret;
}

void Room::enable()
{
    if(active)
    {
        return;
    }

    if(bt_body)
    {
        bt_engine_dynamicsWorld->addRigidBody(bt_body.get());
    }

    for(auto sm : static_mesh)
    {
        if(sm->bt_body != nullptr)
        {
            bt_engine_dynamicsWorld->addRigidBody(sm->bt_body);
        }
    }

    /*
    for(const std::shared_ptr<EngineContainer>& cont : containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                static_cast<Entity*>(cont->object)->enable();
                break;
        }
    }
    */

    active = true;
}

void Room::disable()
{
    if(!active)
    {
        return;
    }

    if(bt_body)
    {
        bt_engine_dynamicsWorld->removeRigidBody(bt_body.get());
    }

    for(auto sm : static_mesh)
    {
        if(sm->bt_body != nullptr)
        {
            bt_engine_dynamicsWorld->removeRigidBody(sm->bt_body);
        }
    }

    /*
    for(const std::shared_ptr<EngineContainer>& cont : containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                static_cast<Entity*>(cont->object)->disable();
                break;
        }
    }
    */

    active = false;
}

void Room::swapToBase()
{
    if((base_room != nullptr) && active)                        //If room is active alternate room
    {
        renderer.cleanList();
        disable();                             //Disable current room
        base_room->disable();                  //Paranoid
        swapPortals(base_room);        //Update portals to match this room
        swapItems(base_room);     //Update items to match this room
        base_room->enable();                   //Enable original room
    }
}

void Room::swapToAlternate()
{
    if((alternate_room != nullptr) && active)              //If room is active base room
    {
        renderer.cleanList();
        disable();                             //Disable current room
        alternate_room->disable();             //Paranoid
        swapPortals(alternate_room);   //Update portals to match this room
        swapItems(alternate_room);          //Update items to match this room
        alternate_room->enable();                              //Enable base room
    }
}

Room* Room::checkFlip()
{
    if(!active)
    {
        if((base_room != nullptr) && (base_room->active))
        {
            return base_room.get();
        }
        else if((alternate_room != nullptr) && (alternate_room->active))
        {
            return alternate_room.get();
        }
    }

    return this;
}

void Room::swapPortals(std::shared_ptr<Room> dest_room)
{
    //Update portals in room rooms
    for(auto r : engine_world.rooms)//For every room in the world itself
    {
        for(Portal& p : r->portals) //For every portal in this room
        {
            if(p.dest_room && p.dest_room->id == id)//If a portal is linked to the input room
            {
                p.dest_room = dest_room;//The portal destination room is the destination room!
                //Con_Printf("The current room %d! has room %d joined to it!", id, i);
            }
        }
        r->buildNearRoomsList();//Rebuild room near list!
    }
}

void Room::swapItems(std::shared_ptr<Room> dest_room)
{
    for(std::shared_ptr<EngineContainer> t : containers)
    {
        t->room = dest_room.get();
    }

    for(std::shared_ptr<EngineContainer> t : dest_room->containers)
    {
        t->room = this;
    }

    std::swap(containers, dest_room->containers);
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    if(entity_tree.find(entity->id()) != entity_tree.end())
        return;
    entity_tree[entity->id()] = entity;
    if(entity->id() + 1 > next_entity_id)
        next_entity_id = entity->id() + 1;
}

bool World::createItem(uint32_t item_id, uint32_t model_id, uint32_t world_model_id, gui::MenuItemType type, uint16_t count, const std::string& name)
{
    SkeletalModel* model = getModelByID(model_id);
    if(!model)
    {
        return false;
    }

    std::unique_ptr<SSBoneFrame> bf(new SSBoneFrame());
    bf->fromModel(model);

    auto item = std::make_shared<BaseItem>();
    item->id = item_id;
    item->world_model_id = world_model_id;
    item->type = type;
    item->count = count;
    item->name[0] = 0;
    strncpy(item->name, name.c_str(), 64);
    item->bf = std::move(bf);

    items_tree[item->id] = item;

    return true;
}

int World::deleteItem(uint32_t item_id)
{
    items_tree.erase(items_tree.find(item_id));
    return 1;
}

SkeletalModel* World::getModelByID(uint32_t id)
{
    if(skeletal_models.front().id == id)
    {
        return &skeletal_models.front();
    }
    if(skeletal_models.back().id == id)
    {
        return &skeletal_models.back();
    }

    size_t min = 0;
    size_t max = skeletal_models.size() - 1;
    do
    {
        auto i = (min + max) / 2;
        if(skeletal_models[i].id == id)
        {
            return &skeletal_models[i];
        }

        if(skeletal_models[i].id < id)
            min = i;
        else
            max = i;
    } while(min < max - 1);

    return nullptr;
}

// Find sprite by ID.
// Not a binary search - sprites may be not sorted by ID.

Sprite* World::getSpriteByID(unsigned int ID)
{
    for(Sprite& sp : sprites)
    {
        if(sp.id == ID)
        {
            return &sp;
        }
    }

    return nullptr;
}


// Check for join portals existing

bool Room::isJoined(Room* r2)
{
    for(const Portal& p : portals)
    {
        if(p.dest_room && p.dest_room->id == r2->id)
        {
            return true;
        }
    }

    for(const Portal& p : r2->portals)
    {
        if(p.dest_room && p.dest_room->id == id)
        {
            return true;
        }
    }

    return false;
}

void Room::buildNearRoomsList()
{
    near_room_list.clear();

    for(const Portal& p : portals)
    {
        addToNearRoomsList(p.dest_room);
    }

    auto nrl = near_room_list;
    for(const std::shared_ptr<Room>& r : nrl)
    {
        if(!r)
            continue;

        for(const Portal& p : r->portals)
        {
            addToNearRoomsList(p.dest_room);
        }
    }
}

void Room::buildOverlappedRoomsList()
{
    overlapped_room_list.clear();

    for(auto r : engine_world.rooms)
    {
        if(isOverlapped(r.get()))
        {
            overlapped_room_list.push_back(r);
        }
    }
}

BaseItem::~BaseItem()
{
    bf->bone_tags.clear();
}

void World::updateAnimTextures()                                                // This function is used for updating global animated texture frame
{
    for(AnimSeq& seq : anim_sequences)
    {
        if(seq.frame_lock)
        {
            continue;
        }

        seq.frame_time += engine_frame_time;
        if(seq.frame_time >= seq.frame_rate)
        {
            int j = (seq.frame_time / seq.frame_rate);
            seq.frame_time -= static_cast<btScalar>(j) * seq.frame_rate;

            switch(seq.anim_type)
            {
                case TR_ANIMTEXTURE_REVERSE:
                    if(seq.reverse_direction)
                    {
                        if(seq.current_frame == 0)
                        {
                            seq.current_frame++;
                            seq.reverse_direction = false;
                        }
                        else if(seq.current_frame > 0)
                        {
                            seq.current_frame--;
                        }
                    }
                    else
                    {
                        if(seq.current_frame == seq.frames.size() - 1)
                        {
                            seq.current_frame--;
                            seq.reverse_direction = true;
                        }
                        else if(seq.current_frame < seq.frames.size() - 1)
                        {
                            seq.current_frame++;
                        }
                        seq.current_frame %= seq.frames.size();                ///@PARANOID
                    }
                    break;

                case TR_ANIMTEXTURE_FORWARD:                                    // inversed in polygon anim. texture frames
                case TR_ANIMTEXTURE_BACKWARD:
                    seq.current_frame++;
                    seq.current_frame %= seq.frames.size();
                    break;
            };
        }
    }
}

void World::calculateWaterTint(float* tint, bool fixed_colour)
{
    if(engineVersion < loader::Engine::TR4)  // If water room and level is TR1-3
    {
        if(engineVersion < loader::Engine::TR3)
        {
            // Placeholder, color very similar to TR1 PSX ver.
            if(fixed_colour)
            {
                tint[0] = 0.585f;
                tint[1] = 0.9f;
                tint[2] = 0.9f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.585f;
                tint[1] *= 0.9f;
                tint[2] *= 0.9f;
            }
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            if(fixed_colour)
            {
                tint[0] = 0.275f;
                tint[1] = 0.45f;
                tint[2] = 0.5f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.275f;
                tint[1] *= 0.45f;
                tint[2] *= 0.5f;
            }
        }
    }
    else
    {
        if(fixed_colour)
        {
            tint[0] = 1.0f;
            tint[1] = 1.0f;
            tint[2] = 1.0f;
            tint[3] = 1.0f;
        }
    }
}

RoomSector* RoomSector::getLowestSector()
{
    RoomSector* lowest_sector = this;

    while(RoomSector* below = lowest_sector->sector_below)
    {
        RoomSector* flipped = below->checkFlip();
        if(!flipped)
            break;
        lowest_sector = flipped;
    }

    return lowest_sector->checkFlip();
}

RoomSector* RoomSector::getHighestSector()
{
    RoomSector* highest_sector = this;

    while(RoomSector* above = highest_sector->sector_above)
    {
        RoomSector* flipped = above->checkFlip();
        if(!flipped)
            break;
        highest_sector = flipped;
    }

    return highest_sector;
}

void Room::genMesh(World* world, size_t room_index, const std::unique_ptr<loader::Level>& tr)
{
    const uint32_t tex_mask = (world->engineVersion == loader::Engine::TR4) ? (loader::TextureIndexMaskTr4) : (loader::TextureIndexMask);

    auto tr_room = &tr->m_rooms[room_index];

    if(tr_room->triangles.empty() && tr_room->rectangles.empty())
    {
        mesh = nullptr;
        return;
    }

    mesh = std::make_shared<BaseMesh>();
    mesh->m_id = room_index;
    mesh->m_texturePageCount = static_cast<uint32_t>(world->tex_atlas->getNumAtlasPages()) + 1;
    mesh->m_usesVertexColors = true; // This is implicitly true on room meshes

    mesh->m_vertices.resize(tr_room->vertices.size());
    auto vertex = mesh->m_vertices.data();
    for(size_t i = 0; i < mesh->m_vertices.size(); i++, vertex++)
    {
        TR_vertex_to_arr(vertex->position, tr_room->vertices[i].vertex);
        vertex->normal.setZero();                                          // paranoid
    }

    mesh->findBB();

    mesh->m_polygons.resize(tr_room->triangles.size() + tr_room->rectangles.size());
    auto p = mesh->m_polygons.begin();

    /*
    * triangles
    */
    for(uint32_t i = 0; i < tr_room->triangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 3, tr_room->triangles[i].vertices, tr_room->triangles[i].texture & tex_mask, &*p);
        p->double_side = tr_room->triangles[i].texture & 0x8000;
    }

    /*
    * rectangles
    */
    for(uint32_t i = 0; i < tr_room->rectangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 4, tr_room->rectangles[i].vertices, tr_room->rectangles[i].texture & tex_mask, &*p);
        p->double_side = tr_room->rectangles[i].texture & 0x8000;
    }

    /*
    * let us normalise normales %)
    */
    for(Vertex& v : mesh->m_vertices)
    {
        v.normal.safeNormalize();
    }

    /*
    * triangles
    */
    p = mesh->m_polygons.begin();
    for(size_t i = 0; i < tr_room->triangles.size(); i++, ++p)
    {
        tr_copyNormals(&*p, mesh, tr_room->triangles[i].vertices);
    }

    /*
    * rectangles
    */
    for(uint32_t i = 0; i < tr_room->rectangles.size(); i++, ++p)
    {
        tr_copyNormals(&*p, mesh, tr_room->rectangles[i].vertices);
    }

    mesh->m_vertices.clear();
    mesh->genFaces();
    mesh->polySortInMesh();
}
