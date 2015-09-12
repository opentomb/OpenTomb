#include "room.h"

#include "engine/engine.h"
#include "entity.h"
#include "loader/level.h"
#include "resource.h"
#include "staticmesh.h"
#include "world.h"

#include <btBulletDynamicsCommon.h>

namespace world
{

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

                engine::bt_engine_dynamicsWorld->removeRigidBody(body);
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

        engine::bt_engine_dynamicsWorld->removeRigidBody(bt_body.get());
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
    for(const std::shared_ptr<engine::EngineContainer>& curr : containers)
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

bool Room::isOverlapped(Room* r1)
{
    assert(r1 != nullptr);
    if((this == r1) || (this == r1->alternate_room.get()) || (alternate_room.get() == r1))
    {
        return false;
    }

    if(!boundingBox.overlaps(r1->boundingBox))
        return false;

    return !isJoined(r1);
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
        engine::bt_engine_dynamicsWorld->addRigidBody(bt_body.get());
    }

    for(auto sm : static_mesh)
    {
        if(sm->bt_body != nullptr)
        {
            engine::bt_engine_dynamicsWorld->addRigidBody(sm->bt_body);
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
        engine::bt_engine_dynamicsWorld->removeRigidBody(bt_body.get());
    }

    for(auto sm : static_mesh)
    {
        if(sm->bt_body != nullptr)
        {
            engine::bt_engine_dynamicsWorld->removeRigidBody(sm->bt_body);
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
        render::renderer.cleanList();
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
        render::renderer.cleanList();
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
    for(auto r : engine::engine_world.rooms)//For every room in the world itself
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
    for(std::shared_ptr<engine::EngineContainer> t : containers)
    {
        t->room = dest_room.get();
    }

    for(std::shared_ptr<engine::EngineContainer> t : dest_room->containers)
    {
        t->room = this;
    }

    std::swap(containers, dest_room->containers);
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

    for(auto r : engine::engine_world.rooms)
    {
        if(isOverlapped(r.get()))
        {
            overlapped_room_list.push_back(r);
        }
    }
}

void Room::genMesh(World* world, uint32_t room_index, const std::unique_ptr<loader::Level>& tr)
{
    const uint32_t tex_mask = (world->engineVersion == loader::Engine::TR4) ? (loader::TextureIndexMaskTr4) : (loader::TextureIndexMask);

    auto tr_room = &tr->m_rooms[room_index];

    if(tr_room->triangles.empty() && tr_room->rectangles.empty())
    {
        mesh = nullptr;
        return;
    }

    mesh = std::make_shared<core::BaseMesh>();
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

    mesh->updateBoundingBox();

    mesh->m_polygons.resize(tr_room->triangles.size() + tr_room->rectangles.size());
    auto p = mesh->m_polygons.begin();

    /*
    * triangles
    */
    for(uint32_t i = 0; i < tr_room->triangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 3, tr_room->triangles[i].vertices, tr_room->triangles[i].texture & tex_mask, &*p);
        p->double_side = (tr_room->triangles[i].texture & 0x8000) != 0;
    }

    /*
    * rectangles
    */
    for(uint32_t i = 0; i < tr_room->rectangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, mesh, 4, tr_room->rectangles[i].vertices, tr_room->rectangles[i].texture & tex_mask, &*p);
        p->double_side = (tr_room->rectangles[i].texture & 0x8000) != 0;
    }

    /*
    * let us normalise normales %)
    */
    for(world::core::Vertex& v : mesh->m_vertices)
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

RoomSector* RoomSector::checkPortalPointerRaw()
{
    if(portal_to_room >= 0)
    {
        std::shared_ptr<Room> r = engine::engine_world.rooms[portal_to_room];
        int ind_x = static_cast<int>((position[0] - r->transform.getOrigin()[0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->transform.getOrigin()[1]) / MeteringSectorSize);
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
        std::shared_ptr<Room> r = engine::engine_world.rooms[portal_to_room];
        if((owner_room->base_room != nullptr) && (r->alternate_room != nullptr))
        {
            r = r->alternate_room;
        }
        else if((owner_room->alternate_room != nullptr) && (r->base_room != nullptr))
        {
            r = r->base_room;
        }
        int ind_x = static_cast<int>((position[0] - r->transform.getOrigin()[0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->transform.getOrigin()[1]) / MeteringSectorSize);
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
        int ind_x = static_cast<int>((position[0] - r->transform.getOrigin()[0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->transform.getOrigin()[1]) / MeteringSectorSize);
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
        int ind_x = static_cast<int>((position[0] - r->transform.getOrigin()[0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->transform.getOrigin()[1]) / MeteringSectorSize);
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

    RoomSector* s1p = s2->owner_room->getSectorRaw(position);
    RoomSector* s2p = s1->owner_room->getSectorRaw(s2->position);

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
       (ceiling_penetration_config == PenetrationConfig::Wall) ||
       (s2->ceiling_penetration_config == PenetrationConfig::Wall) ||
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
       (floor_penetration_config == PenetrationConfig::Wall) ||
       (s2->floor_penetration_config == PenetrationConfig::Wall) ||
       (!ignore_doors && (sector_below || s2->sector_below)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(floor_corners->m_floats[2] != s2->floor_corners->m_floats[2]) return false;
    }

    return true;
}

namespace
{
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
} // anonymous namespace

btVector3 RoomSector::getFloorPoint()
{
    return Sector_HighestFloorCorner(getLowestSector());
}

btVector3 RoomSector::getCeilingPoint()
{
    return Sector_LowestCeilingCorner(getHighestSector());
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

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(const std::vector<RoomSector>& heightmap, const std::vector<SectorTween>& tweens, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    std::shared_ptr<Room> r = heightmap.front().owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;

    for(uint32_t i = 0; i < r->sectors.size(); i++)
    {
        if((heightmap[i].floor_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].floor_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].floor_diagonal_type == DiagonalType::None) ||
               (heightmap[i].floor_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
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
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
        }

        if((heightmap[i].ceiling_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].ceiling_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].ceiling_diagonal_type == DiagonalType::None) ||
               (heightmap[i].ceiling_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
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
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
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

    for(const SectorTween& tween : tweens)
    {
        switch(tween.ceiling_tween_type)
        {
            case TweenType::TwoTriangles:
            {
                btScalar t = std::abs((tween.ceiling_corners[2][2] - tween.ceiling_corners[3][2]) /
                                      (tween.ceiling_corners[0][2] - tween.ceiling_corners[1][2]));
                t = 1.0f / (1.0f + t);
                btVector3 o;
                o.setInterpolate3(tween.ceiling_corners[0], tween.ceiling_corners[2], t);
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     o, true);
                trimesh->addTriangle(tween.ceiling_corners[3],
                                     tween.ceiling_corners[2],
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt += 2;
                break;
        };

        switch(tween.floor_tween_type)
        {
            case TweenType::TwoTriangles:
            {
                btScalar t = std::abs((tween.floor_corners[2][2] - tween.floor_corners[3][2]) /
                                      (tween.floor_corners[0][2] - tween.floor_corners[1][2]));
                t = 1.0 / (1.0 + t);
                btVector3 o;
                o.setInterpolate3(tween.floor_corners[0], tween.floor_corners[2], t);
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     o, true);
                trimesh->addTriangle(tween.floor_corners[3],
                                     tween.floor_corners[2],
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt += 2;
                break;
        };
    }

    if(cnt == 0)
    {
        delete trimesh;
        return nullptr;
    }

    auto ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);
    return ret;
}

} // namespace world
