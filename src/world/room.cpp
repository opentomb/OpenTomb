#include "room.h"

#include "engine/engine.h"
#include "entity.h"
#include "loader/level.h"
#include "resource.h"
#include "staticmesh.h"
#include "world.h"

#include <btBulletDynamicsCommon.h>

#include <boost/log/trivial.hpp>

namespace world
{

Room::~Room()
{
    for(std::shared_ptr<StaticMesh>& mesh : m_staticMeshes)
    {
        if(btRigidBody* body = mesh->bt_body)
        {
            body->setUserPointer(nullptr);
            if(auto state = body->getMotionState())
            {
                delete state;
                body->setMotionState(nullptr);
            }
            body->setCollisionShape(nullptr);

            engine::bt_engine_dynamicsWorld->removeRigidBody(body);
            delete body;
            mesh->bt_body = nullptr;
        }

        mesh->setRoom( nullptr );
    }

    if(m_btBody)
    {
        m_btBody->setUserPointer(nullptr);
        if(auto state = m_btBody->getMotionState())
        {
            delete state;
            m_btBody->setMotionState(nullptr);
        }
        if(auto shape = m_btBody->getCollisionShape())
        {
            delete shape;
            m_btBody->setCollisionShape(nullptr);
        }

        engine::bt_engine_dynamicsWorld->removeRigidBody(m_btBody.get());
    }
}

void Room::addEntity(Entity* entity)
{
    for(const Object* object : m_objects)
    {
        if(object == entity)
        {
            return;
        }
    }

    entity->setRoom( this );
    m_objects.emplace_back(entity);
}

bool Room::removeEntity(Entity* entity)
{
    if(!entity || m_objects.empty())
        return false;

    auto it = std::find(m_objects.begin(), m_objects.end(), entity);
    if(it != m_objects.end())
    {
        m_objects.erase(it);
        entity->setRoom( nullptr );
        return true;
    }

    return false;
}

void Room::addToNearRoomsList(Room* r)
{
    if(r && !isInNearRoomsList(*r) && getId() != r->getId() && !isOverlapped(r))
    {
        m_nearRooms.push_back(r);
    }
}

bool Room::isInNearRoomsList(const Room& r1) const
{
    if(getId() == r1.getId())
    {
        return true;
    }

    if(r1.m_nearRooms.size() >= m_nearRooms.size())
    {
        for(const Room* r : m_nearRooms)
        {
            if(r->getId() == r1.getId())
            {
                return true;
            }
        }
    }
    else
    {
        for(const Room* r : r1.m_nearRooms)
        {
            if(r->getId() == getId())
            {
                return true;
            }
        }
    }
    return false;
}

bool Room::hasSector(size_t x, size_t y)
{
    return x < m_sectors.shape()[0] && y < m_sectors.shape()[1];
}

bool Room::isOverlapped(Room* r1)
{
    BOOST_ASSERT(r1 != nullptr);
    if((this == r1) || (this == r1->m_alternateRoom.get()) || (m_alternateRoom.get() == r1))
    {
        return false;
    }

    if(!m_boundingBox.overlaps(r1->m_boundingBox))
        return false;

    return !isJoined(r1);
}

RoomSector* Room::getSectorRaw(const glm::vec3& pos)
{
    if(!m_active)
    {
        return nullptr;
    }

    int x = static_cast<int>(pos[0] - m_modelMatrix[3][0]) / 1024;
    int y = static_cast<int>(pos[1] - m_modelMatrix[3][1]) / 1024;
    if(x < 0 || static_cast<size_t>(x) >= m_sectors.shape()[0] || y < 0 || static_cast<size_t>(y) >= m_sectors.shape()[1])
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    return &m_sectors[x][y];
}

RoomSector* Room::getSectorXYZ(const glm::vec3& pos)
{
    Room* room = checkFlip();

    if(!room->m_active)
    {
        return nullptr;
    }

    int x = static_cast<int>(pos[0] - room->m_modelMatrix[3][0]) / 1024;
    int y = static_cast<int>(pos[1] - room->m_modelMatrix[3][1]) / 1024;
    if(x < 0 || static_cast<size_t>(x) >= room->m_sectors.shape()[0] || y < 0 || static_cast<size_t>(y) >= room->m_sectors.shape()[1])
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    RoomSector* ret = &room->m_sectors[x][y];

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
    if(m_active)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Enabling room " << getId();

    if(m_btBody)
    {
        engine::bt_engine_dynamicsWorld->addRigidBody(m_btBody.get());
    }

    for(const std::shared_ptr<StaticMesh>& sm : m_staticMeshes)
    {
        if(sm->bt_body != nullptr)
        {
            engine::bt_engine_dynamicsWorld->addRigidBody(sm->bt_body);
        }
    }

    m_active = true;
}

void Room::disable()
{
    if(!m_active)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Disabling room " << getId();

    if(m_btBody)
    {
        engine::bt_engine_dynamicsWorld->removeRigidBody(m_btBody.get());
    }

    for(auto sm : m_staticMeshes)
    {
        if(sm->bt_body != nullptr)
        {
            engine::bt_engine_dynamicsWorld->removeRigidBody(sm->bt_body);
        }
    }

    m_active = false;
}

void Room::swapToBase()
{
    if((m_baseRoom != nullptr) && m_active)                        //If room is active alternate room
    {
        render::renderer.cleanList();
        disable();                             //Disable current room
        m_baseRoom->disable();                  //Paranoid
        swapPortals(m_baseRoom);        //Update portals to match this room
        swapObjects(m_baseRoom);     //Update items to match this room
        m_baseRoom->enable();                   //Enable original room
    }
}

void Room::swapToAlternate()
{
    if((m_alternateRoom != nullptr) && m_active)              //If room is active base room
    {
        render::renderer.cleanList();
        disable();                             //Disable current room
        m_alternateRoom->disable();             //Paranoid
        swapPortals(m_alternateRoom);   //Update portals to match this room
        swapObjects(m_alternateRoom);          //Update items to match this room
        m_alternateRoom->enable();                              //Enable base room
    }
}

Room* Room::checkFlip()
{
    if(!m_active)
    {
        if((m_baseRoom != nullptr) && (m_baseRoom->m_active))
        {
            return m_baseRoom.get();
        }
        else if((m_alternateRoom != nullptr) && (m_alternateRoom->m_active))
        {
            return m_alternateRoom.get();
        }
    }

    return this;
}

void Room::swapPortals(std::shared_ptr<Room> dest_room)
{
    //Update portals in room rooms
    for(auto r : engine::engine_world.rooms)//For every room in the world itself
    {
        for(Portal& p : r->m_portals) //For every portal in this room
        {
            if(p.destination && p.destination->getId() == getId())//If a portal is linked to the input room
            {
                p.destination = dest_room.get();//The portal destination room is the destination room!
                                        //Con_Printf("The current room %d! has room %d joined to it!", id, i);
            }
        }
        r->buildNearRoomsList();//Rebuild room near list!
    }
}

void Room::swapObjects(std::shared_ptr<Room> dest_room)
{
    for(Object* object : m_objects)
    {
        object->setRoom( dest_room.get() );
    }

    for(Object* object : dest_room->m_objects)
    {
        object->setRoom( this );
    }

    std::swap(m_objects, dest_room->m_objects);
}

// Check for join portals existing

bool Room::isJoined(Room* r2)
{
    for(const Portal& p : m_portals)
    {
        if(p.destination && p.destination->getId() == r2->getId())
        {
            return true;
        }
    }

    for(const Portal& p : r2->m_portals)
    {
        if(p.destination && p.destination->getId() == getId())
        {
            return true;
        }
    }

    return false;
}

void Room::buildNearRoomsList()
{
    m_nearRooms.clear();

    for(const Portal& p : m_portals)
    {
        addToNearRoomsList(p.destination);
    }

    auto nrl = m_nearRooms;
    for(const Room* r : nrl)
    {
        if(!r)
            continue;

        for(const Portal& p : r->m_portals)
        {
            addToNearRoomsList(p.destination);
        }
    }
}

void Room::buildOverlappedRoomsList()
{
    m_overlappedRooms.clear();

    for(auto r : engine::engine_world.rooms)
    {
        if(isOverlapped(r.get()))
        {
            m_overlappedRooms.push_back(r);
        }
    }
}

void Room::genMesh(World* world, const std::unique_ptr<loader::Level>& tr)
{
    const uint32_t tex_mask = (world->engineVersion == loader::Engine::TR4) ? (loader::TextureIndexMaskTr4) : (loader::TextureIndexMask);

    auto tr_room = &tr->m_rooms[getId()];

    if(tr_room->triangles.empty() && tr_room->rectangles.empty())
    {
        m_mesh = nullptr;
        return;
    }

    m_mesh = std::make_shared<core::BaseMesh>();
    m_mesh->m_id = getId();
    m_mesh->m_texturePageCount = static_cast<uint32_t>(world->tex_atlas->getNumAtlasPages()) + 1;
    m_mesh->m_usesVertexColors = true; // This is implicitly true on room meshes

    m_mesh->m_vertices.resize(tr_room->vertices.size());
    auto vertex = m_mesh->m_vertices.data();
    for(size_t i = 0; i < m_mesh->m_vertices.size(); i++, vertex++)
    {
        vertex->position = util::convert(tr_room->vertices[i].vertex);
        vertex->normal = { 0,0,0 };                                          // paranoid
    }

    m_mesh->updateBoundingBox();

    m_mesh->m_polygons.resize(tr_room->triangles.size() + tr_room->rectangles.size());
    auto p = m_mesh->m_polygons.begin();

    /*
    * triangles
    */
    for(uint32_t i = 0; i < tr_room->triangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, m_mesh, 3, tr_room->triangles[i].vertices, tr_room->triangles[i].texture & tex_mask, &*p);
        p->double_side = (tr_room->triangles[i].texture & 0x8000) != 0;
    }

    /*
    * rectangles
    */
    for(uint32_t i = 0; i < tr_room->rectangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(world, tr, tr_room, m_mesh, 4, tr_room->rectangles[i].vertices, tr_room->rectangles[i].texture & tex_mask, &*p);
        p->double_side = (tr_room->rectangles[i].texture & 0x8000) != 0;
    }

    /*
    * let us normalise normales %)
    */
    for(world::core::Vertex& v : m_mesh->m_vertices)
    {
        v.normal = glm::normalize(v.normal);
    }

    /*
    * triangles
    */
    p = m_mesh->m_polygons.begin();
    for(size_t i = 0; i < tr_room->triangles.size(); i++, ++p)
    {
        tr_copyNormals(&*p, m_mesh, tr_room->triangles[i].vertices);
    }

    /*
    * rectangles
    */
    for(uint32_t i = 0; i < tr_room->rectangles.size(); i++, ++p)
    {
        tr_copyNormals(&*p, m_mesh, tr_room->rectangles[i].vertices);
    }

    m_mesh->m_vertices.clear();
    m_mesh->genFaces();
    m_mesh->polySortInMesh();
}

RoomSector* RoomSector::checkPortalPointerRaw()
{
    if(portal_to_room >= 0)
    {
        std::shared_ptr<Room> r = engine::engine_world.rooms[portal_to_room];
        int ind_x = static_cast<int>((position[0] - r->m_modelMatrix[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->m_modelMatrix[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->m_sectors.shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->m_sectors.shape()[1])
        {
            return &r->m_sectors[ind_x][ind_y];
        }
    }

    return this;
}

RoomSector* RoomSector::checkPortalPointer()
{
    if(portal_to_room >= 0)
    {
        std::shared_ptr<Room> r = engine::engine_world.rooms[portal_to_room];
        if((owner_room->m_baseRoom != nullptr) && (r->m_alternateRoom != nullptr))
        {
            r = r->m_alternateRoom;
        }
        else if((owner_room->m_alternateRoom != nullptr) && (r->m_baseRoom != nullptr))
        {
            r = r->m_baseRoom;
        }
        int ind_x = static_cast<int>((position[0] - r->m_modelMatrix[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->m_modelMatrix[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->m_sectors.shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->m_sectors.shape()[1])
        {
            return &r->m_sectors[ind_x][ind_y];
        }
    }

    return this;
}

RoomSector* RoomSector::checkBaseRoom()
{
    if(owner_room->m_baseRoom != nullptr)
    {
        std::shared_ptr<Room> r = owner_room->m_baseRoom;
        int ind_x = static_cast<int>((position[0] - r->m_modelMatrix[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->m_modelMatrix[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->m_sectors.shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->m_sectors.shape()[1])
        {
            return &r->m_sectors[ind_x][ind_y];
        }
    }

    return this;
}

RoomSector* RoomSector::checkAlternateRoom()
{
    if(owner_room->m_alternateRoom != nullptr)
    {
        std::shared_ptr<Room> r = owner_room->m_alternateRoom;
        int ind_x = static_cast<int>((position[0] - r->m_modelMatrix[3][1]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->m_modelMatrix[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->m_sectors.shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->m_sectors.shape()[1])
        {
            return &r->m_sectors[ind_x][ind_y];
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
    if(!s2)
        return false;
    if(this == s2)
        return true;

    if((ceiling != s2->ceiling) ||
       (ceiling_penetration_config == PenetrationConfig::Wall) ||
       (s2->ceiling_penetration_config == PenetrationConfig::Wall) ||
       (!ignore_doors && (sector_above || s2->sector_above)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(ceiling_corners[i].z != s2->ceiling_corners[i].z)
            return false;
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
        if(floor_corners[i].z != s2->floor_corners[i].z)
            return false;
    }

    return true;
}

glm::vec3 RoomSector::getFloorPoint()
{
    return getLowestSector()->getHighestFloorCorner();
}

glm::vec3 RoomSector::getCeilingPoint()
{
    return getHighestSector()->getLowestCeilingCorner();
}

RoomSector* RoomSector::checkFlip()
{
    if(owner_room->m_active)
        return this;

    if(owner_room->m_baseRoom && owner_room->m_baseRoom->m_active)
    {
        std::shared_ptr<Room> r = owner_room->m_baseRoom;
        return &r->m_sectors[index_x][index_y];
    }
    else if(owner_room->m_alternateRoom && owner_room->m_alternateRoom->m_active)
    {
        std::shared_ptr<Room> r = owner_room->m_alternateRoom;
        return &r->m_sectors[index_x][index_y];
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
btCollisionShape *BT_CSfromHeightmap(const boost::multi_array<RoomSector, 2>& heightmap, const std::vector<SectorTween>& tweens, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    std::shared_ptr<Room> r = heightmap.origin()->owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;

    for(const auto& column : heightmap)
    {
        for(const RoomSector& sector : column)
        {
            if((sector.floor_penetration_config != PenetrationConfig::Ghost) &&
               (sector.floor_penetration_config != PenetrationConfig::Wall))
            {
                if((sector.floor_diagonal_type == DiagonalType::None) ||
                   (sector.floor_diagonal_type == DiagonalType::NW))
                {
                    if(sector.floor_penetration_config != PenetrationConfig::DoorVerticalA)
                    {
                        trimesh->addTriangle(util::convert(sector.floor_corners[3]),
                                             util::convert(sector.floor_corners[2]),
                                             util::convert(sector.floor_corners[0]),
                                             true);
                        cnt++;
                    }

                    if(sector.floor_penetration_config != PenetrationConfig::DoorVerticalB)
                    {
                        trimesh->addTriangle(util::convert(sector.floor_corners[2]),
                                             util::convert(sector.floor_corners[1]),
                                             util::convert(sector.floor_corners[0]),
                                             true);
                        cnt++;
                    }
                }
                else
                {
                    if(sector.floor_penetration_config != PenetrationConfig::DoorVerticalA)
                    {
                        trimesh->addTriangle(util::convert(sector.floor_corners[3]),
                                             util::convert(sector.floor_corners[2]),
                                             util::convert(sector.floor_corners[1]),
                                             true);
                        cnt++;
                    }

                    if(sector.floor_penetration_config != PenetrationConfig::DoorVerticalB)
                    {
                        trimesh->addTriangle(util::convert(sector.floor_corners[3]),
                                             util::convert(sector.floor_corners[1]),
                                             util::convert(sector.floor_corners[0]),
                                             true);
                        cnt++;
                    }
                }
            }

            if((sector.ceiling_penetration_config != PenetrationConfig::Ghost) &&
               (sector.ceiling_penetration_config != PenetrationConfig::Wall))
            {
                if((sector.ceiling_diagonal_type == DiagonalType::None) ||
                   (sector.ceiling_diagonal_type == DiagonalType::NW))
                {
                    if(sector.ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                    {
                        trimesh->addTriangle(util::convert(sector.ceiling_corners[0]),
                                             util::convert(sector.ceiling_corners[2]),
                                             util::convert(sector.ceiling_corners[3]),
                                             true);
                        cnt++;
                    }

                    if(sector.ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                    {
                        trimesh->addTriangle(util::convert(sector.ceiling_corners[0]),
                                             util::convert(sector.ceiling_corners[1]),
                                             util::convert(sector.ceiling_corners[2]),
                                             true);
                        cnt++;
                    }
                }
                else
                {
                    if(sector.ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                    {
                        trimesh->addTriangle(util::convert(sector.ceiling_corners[0]),
                                             util::convert(sector.ceiling_corners[1]),
                                             util::convert(sector.ceiling_corners[3]),
                                             true);
                        cnt++;
                    }

                    if(sector.ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                    {
                        trimesh->addTriangle(util::convert(sector.ceiling_corners[1]),
                                             util::convert(sector.ceiling_corners[2]),
                                             util::convert(sector.ceiling_corners[3]),
                                             true);
                        cnt++;
                    }
                }
            }
        }
    }

    for(const SectorTween& tween : tweens)
    {
        switch(tween.ceiling_tween_type)
        {
            case TweenType::None:
                break;

            case TweenType::TwoTriangles:
            {
                glm::float_t t = glm::abs((tween.ceiling_corners[2][2] - tween.ceiling_corners[3][2]) /
                                      (tween.ceiling_corners[0][2] - tween.ceiling_corners[1][2]));
                t = 1.0f / (1.0f + t);
                btVector3 o;
                o.setInterpolate3(util::convert(tween.ceiling_corners[0]), util::convert(tween.ceiling_corners[2]), t);
                trimesh->addTriangle(util::convert(tween.ceiling_corners[0]),
                                     util::convert(tween.ceiling_corners[1]),
                                     o, true);
                trimesh->addTriangle(util::convert(tween.ceiling_corners[3]),
                                     util::convert(tween.ceiling_corners[2]),
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(util::convert(tween.ceiling_corners[0]),
                                     util::convert(tween.ceiling_corners[1]),
                                     util::convert(tween.ceiling_corners[3]),
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(util::convert(tween.ceiling_corners[2]),
                                     util::convert(tween.ceiling_corners[1]),
                                     util::convert(tween.ceiling_corners[3]),
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(util::convert(tween.ceiling_corners[0]),
                                     util::convert(tween.ceiling_corners[1]),
                                     util::convert(tween.ceiling_corners[3]),
                                     true);
                trimesh->addTriangle(util::convert(tween.ceiling_corners[2]),
                                     util::convert(tween.ceiling_corners[1]),
                                     util::convert(tween.ceiling_corners[3]),
                                     true);
                cnt += 2;
                break;
        };

        switch(tween.floor_tween_type)
        {
            case TweenType::None:
                break;

            case TweenType::TwoTriangles:
            {
                glm::float_t t = glm::abs((tween.floor_corners[2][2] - tween.floor_corners[3][2]) /
                                      (tween.floor_corners[0][2] - tween.floor_corners[1][2]));
                t = 1.0f / (1.0f + t);
                btVector3 o;
                o.setInterpolate3(util::convert(tween.floor_corners[0]), util::convert(tween.floor_corners[2]), t);
                trimesh->addTriangle(util::convert(tween.floor_corners[0]),
                                     util::convert(tween.floor_corners[1]),
                                     o, true);
                trimesh->addTriangle(util::convert(tween.floor_corners[3]),
                                     util::convert(tween.floor_corners[2]),
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(util::convert(tween.floor_corners[0]),
                                     util::convert(tween.floor_corners[1]),
                                     util::convert(tween.floor_corners[3]),
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(util::convert(tween.floor_corners[2]),
                                     util::convert(tween.floor_corners[1]),
                                     util::convert(tween.floor_corners[3]),
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(util::convert(tween.floor_corners[0]),
                                     util::convert(tween.floor_corners[1]),
                                     util::convert(tween.floor_corners[3]),
                                     true);
                trimesh->addTriangle(util::convert(tween.floor_corners[2]),
                                     util::convert(tween.floor_corners[1]),
                                     util::convert(tween.floor_corners[3]),
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
