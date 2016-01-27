#include "room.h"

#include "engine/bullet.h"
#include "engine/engine.h"
#include "entity.h"
#include "loader/level.h"
#include "render/shader_description.h"
#include "resource.h"
#include "staticmesh.h"
#include "world.h"

#include <btBulletDynamicsCommon.h>

#include <boost/log/trivial.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace world
{
Room::~Room()
{
    for(const std::shared_ptr<StaticMesh>& mesh : m_staticMeshes)
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

            getWorld()->m_engine->bullet.dynamicsWorld->removeRigidBody(body);
            delete body;
            mesh->bt_body = nullptr;
        }

        mesh->setRoom(nullptr);
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

        getWorld()->m_engine->bullet.dynamicsWorld->removeRigidBody(m_btBody.get());
    }
}

void Room::addSprite(core::Sprite* sprite, const glm::vec3& pos)
{
    RoomSprite sp;
    sp.sprite = sprite;
    sp.pos = pos;
    sp.was_rendered = false;
    m_sprites.emplace_back(sp);
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

    entity->setRoom(this);
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
        entity->setRoom(nullptr);
        return true;
    }

    return false;
}

void Room::addToNearRoomsList(Room* r)
{
    if(r && !isInNearRoomsList(*r) && getId() != r->getId() && !overlaps(r))
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

bool Room::hasSector(size_t x, size_t y) const
{
    return x < m_sectors.shape()[0] && y < m_sectors.shape()[1];
}

bool Room::overlaps(Room* r1)
{
    BOOST_ASSERT(r1 != nullptr);
    if(this == r1 || this == r1->m_alternateRoom || m_alternateRoom == r1)
    {
        return false;
    }

    if(!m_boundingBox.overlaps(r1->m_boundingBox))
        return false;

    return !isJoined(r1);
}

const RoomSector* Room::getSectorRaw(const glm::vec3& pos) const
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

const RoomSector* Room::getSectorXYZ(const glm::vec3& pos) const
{
    const Room* room = checkFlip();

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

    const RoomSector* ret = &room->m_sectors[x][y];

    //resolve Z overlapped neighboard rooms. room below has more priority.

    if(ret->sector_below && ret->sector_below->ceiling >= pos[2])
    {
        return ret->sector_below->checkFlip();
    }

    if(ret->sector_above && ret->sector_above->floor <= pos[2])
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
        getWorld()->m_engine->bullet.dynamicsWorld->addRigidBody(m_btBody.get());
    }

    for(const std::shared_ptr<StaticMesh>& sm : m_staticMeshes)
    {
        if(sm->bt_body != nullptr)
        {
            getWorld()->m_engine->bullet.dynamicsWorld->addRigidBody(sm->bt_body);
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
        getWorld()->m_engine->bullet.dynamicsWorld->removeRigidBody(m_btBody.get());
    }

    for(auto sm : m_staticMeshes)
    {
        if(sm->bt_body != nullptr)
        {
            getWorld()->m_engine->bullet.dynamicsWorld->removeRigidBody(sm->bt_body);
        }
    }

    m_active = false;
}

void Room::swapToBase()
{
    if(m_baseRoom != nullptr && m_active)                        //If room is active alternate room
    {
        getWorld()->m_engine->renderer.cleanList();
        disable();                             //Disable current room
        m_baseRoom->disable();                  //Paranoid
        swapPortals(m_baseRoom);        //Update portals to match this room
        swapObjects(m_baseRoom);     //Update items to match this room
        m_baseRoom->enable();                   //Enable original room
    }
}

void Room::swapToAlternate()
{
    if(m_alternateRoom != nullptr && m_active)              //If room is active base room
    {
        getWorld()->m_engine->renderer.cleanList();
        disable();                             //Disable current room
        m_alternateRoom->disable();             //Paranoid
        swapPortals(m_alternateRoom);   //Update portals to match this room
        swapObjects(m_alternateRoom);          //Update items to match this room
        m_alternateRoom->enable();                              //Enable base room
    }
}

Room* Room::checkFlip()
{
    if(m_active)
        return this;

    if(m_baseRoom != nullptr && m_baseRoom->m_active)
    {
        return m_baseRoom;
    }
    else if(m_alternateRoom != nullptr && m_alternateRoom->m_active)
    {
        return m_alternateRoom;
    }

    return this;
}

void Room::swapPortals(Room* dest_room)
{
    //Update portals in room rooms
    for(std::shared_ptr<Room> r : getWorld()->m_rooms)//For every room in the *getWorld() itself
    {
        for(Portal& p : r->m_portals) //For every portal in this room
        {
            if(p.destination && p.destination->getId() == getId())//If a portal is linked to the input room
            {
                p.destination = dest_room;//The portal destination room is the destination room!
                                        //Con_Printf("The current room %d! has room %d joined to it!", id, i);
            }
        }
        r->buildNearRoomsList();//Rebuild room near list!
    }
}

void Room::swapObjects(Room* dest_room)
{
    for(Object* object : m_objects)
    {
        object->setRoom(dest_room);
    }

    for(Object* object : dest_room->m_objects)
    {
        object->setRoom(this);
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

    for(auto r : getWorld()->m_rooms)
    {
        if(overlaps(r.get()))
        {
            m_overlappedRooms.push_back(r);
        }
    }
}

void Room::genMesh(const std::unique_ptr<loader::Level>& tr)
{
    const uint32_t tex_mask = getWorld()->m_engineVersion == loader::Engine::TR4 ? loader::TextureIndexMaskTr4 : loader::TextureIndexMask;

    auto& tr_room = tr->m_rooms[getId()];

    if(tr_room.triangles.empty() && tr_room.rectangles.empty())
    {
        m_mesh = nullptr;
        return;
    }

    m_mesh = std::make_shared<core::BaseMesh>();
    m_mesh->m_texturePageCount = getWorld()->m_textureAtlas->getNumAtlasPages() + 1;
    m_mesh->m_usesVertexColors = true; // This is implicitly true on room meshes

    m_mesh->m_vertices.resize(tr_room.vertices.size());
    auto vertex = m_mesh->m_vertices.data();
    for(size_t i = 0; i < m_mesh->m_vertices.size(); i++, vertex++)
    {
        vertex->position = util::convert(tr_room.vertices[i].vertex);
        vertex->normal = { 0,0,0 };                                          // paranoid
    }

    m_mesh->updateBoundingBox();

    m_mesh->m_polygons.resize(tr_room.triangles.size() + tr_room.rectangles.size());
    auto p = m_mesh->m_polygons.begin();

    /*
    * triangles
    */
    for(size_t i = 0; i < tr_room.triangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(*getWorld(), tr, tr_room, m_mesh, 3, tr_room.triangles[i].vertices, tr_room.triangles[i].texture & tex_mask, *p);
        p->isDoubleSided = (tr_room.triangles[i].texture & 0x8000) != 0;
    }

    /*
    * rectangles
    */
    for(size_t i = 0; i < tr_room.rectangles.size(); i++, ++p)
    {
        tr_setupRoomVertices(*getWorld(), tr, tr_room, m_mesh, 4, tr_room.rectangles[i].vertices, tr_room.rectangles[i].texture & tex_mask, *p);
        p->isDoubleSided = (tr_room.rectangles[i].texture & 0x8000) != 0;
    }

    /*
    * let us normalise normales %)
    */
    for(core::Vertex& v : m_mesh->m_vertices)
    {
        v.normal = glm::normalize(v.normal);
    }

    /*
    * triangles
    */
    p = m_mesh->m_polygons.begin();
    for(size_t i = 0; i < tr_room.triangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *m_mesh, tr_room.triangles[i].vertices);
    }

    /*
    * rectangles
    */
    for(size_t i = 0; i < tr_room.rectangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *m_mesh, tr_room.rectangles[i].vertices);
    }

    m_mesh->m_vertices.clear();
    m_mesh->genFaces();
    m_mesh->polySortInMesh(*getWorld());
}

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(const Room::SectorArray& heightmap, const std::vector<SectorTween>& tweens, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;

    for(const auto& column : heightmap)
    {
        for(const RoomSector& sector : column)
        {
            if(sector.floor_penetration_config != PenetrationConfig::Ghost &&
               sector.floor_penetration_config != PenetrationConfig::Wall)
            {
                if(sector.floor_diagonal_type == DiagonalType::None ||
                   sector.floor_diagonal_type == DiagonalType::NW)
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

            if(sector.ceiling_penetration_config != PenetrationConfig::Ghost &&
               sector.ceiling_penetration_config != PenetrationConfig::Wall)
            {
                if(sector.ceiling_diagonal_type == DiagonalType::None ||
                   sector.ceiling_diagonal_type == DiagonalType::NW)
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

void Room::load(const std::unique_ptr<loader::Level>& tr)
{
    m_active = true;
    m_flags = tr->m_rooms[getId()].flags;
    m_lightMode = tr->m_rooms[getId()].light_mode;
    m_reverbType = tr->m_rooms[getId()].reverb_info;
    m_waterScheme = tr->m_rooms[getId()].water_scheme;
    m_alternateGroup = tr->m_rooms[getId()].alternate_group;

    m_modelMatrix = glm::translate(glm::mat4(1.0f), { tr->m_rooms[getId()].offset.x, -tr->m_rooms[getId()].offset.z, tr->m_rooms[getId()].offset.y });
    m_ambientLighting[0] = tr->m_rooms[getId()].light_colour.r * 2;
    m_ambientLighting[1] = tr->m_rooms[getId()].light_colour.g * 2;
    m_ambientLighting[2] = tr->m_rooms[getId()].light_colour.b * 2;
    setRoom(this);
    m_nearRooms.clear();
    m_overlappedRooms.clear();

    genMesh(tr);

    m_btBody.reset();

    /*
     *  let us load static room meshes
     */
    m_staticMeshes.clear();

    const loader::Room *tr_room = &tr->m_rooms[getId()];
    for(size_t i = 0; i < tr_room->static_meshes.size(); i++)
    {
        const loader::StaticMesh* tr_static = tr->findStaticMeshById(tr_room->static_meshes[i].object_id);
        if(tr_static == nullptr)
        {
            continue;
        }
        m_staticMeshes.emplace_back(std::make_shared<StaticMesh>(tr_room->static_meshes[i].object_id, getWorld()));
        std::shared_ptr<StaticMesh> r_static = m_staticMeshes.back();
        r_static->setRoom(this);
        r_static->mesh = getWorld()->m_meshes[tr->m_meshIndices[tr_static->mesh]];
        r_static->tint[0] = tr_room->static_meshes[i].tint.r * 2;
        r_static->tint[1] = tr_room->static_meshes[i].tint.g * 2;
        r_static->tint[2] = tr_room->static_meshes[i].tint.b * 2;
        r_static->tint[3] = tr_room->static_meshes[i].tint.a * 2;

        r_static->collisionBoundingBox.min[0] = tr_static->collision_box[0].x;
        r_static->collisionBoundingBox.min[1] = -tr_static->collision_box[0].z;
        r_static->collisionBoundingBox.min[2] = tr_static->collision_box[1].y;
        r_static->collisionBoundingBox.max[0] = tr_static->collision_box[1].x;
        r_static->collisionBoundingBox.max[1] = -tr_static->collision_box[1].z;
        r_static->collisionBoundingBox.max[2] = tr_static->collision_box[0].y;

        r_static->visibleBoundingBox.min[0] = tr_static->visibility_box[0].x;
        r_static->visibleBoundingBox.min[1] = -tr_static->visibility_box[0].z;
        r_static->visibleBoundingBox.min[2] = tr_static->visibility_box[1].y;

        r_static->visibleBoundingBox.max[0] = tr_static->visibility_box[1].x;
        r_static->visibleBoundingBox.max[1] = -tr_static->visibility_box[1].z;
        r_static->visibleBoundingBox.max[2] = tr_static->visibility_box[0].y;

        r_static->obb.transform = &m_staticMeshes[i]->transform;
        r_static->obb.radius = m_staticMeshes[i]->mesh->m_radius;

        glm::vec3 position{ tr_room->static_meshes[i].position.x, -tr_room->static_meshes[i].position.z, tr_room->static_meshes[i].position.y};

        r_static->transform = glm::mat4(1.0f);
        r_static->transform = glm::rotate(r_static->transform, glm::radians(tr_room->static_meshes[i].rotation), { 0,0,1 });
        r_static->transform = glm::translate(r_static->transform, position);

        r_static->was_rendered = false;
        r_static->obb.rebuild(r_static->visibleBoundingBox);
        r_static->obb.doTransform();

        r_static->bt_body = nullptr;
        r_static->hide = false;

        // Disable static mesh collision, if flag value is 3 (TR1) or all bounding box
        // coordinates are equal (TR2-5).

        if(tr_static->flags == 3 ||
           (tr_static->collision_box[0].x == -tr_static->collision_box[0].y
            && tr_static->collision_box[0].y == tr_static->collision_box[0].z
            && tr_static->collision_box[1].x == -tr_static->collision_box[1].y
            && tr_static->collision_box[1].y == tr_static->collision_box[1].z))
        {
            r_static->setCollisionType(CollisionType::None);
        }
        else
        {
            r_static->setCollisionType(CollisionType::Static);
            r_static->setCollisionShape(CollisionShape::Box);
        }

        // Set additional static mesh properties from level script override.

        Res_SetStaticMeshProperties(r_static);

        // Set static mesh collision.
        if(r_static->getCollisionType() == CollisionType::None)
            continue;

        btCollisionShape* cshape;
        switch(r_static->getCollisionShape())
        {
            case CollisionShape::Box:
                cshape = core::BT_CSfromBBox(r_static->collisionBoundingBox, true, true);
                break;

            case CollisionShape::BoxBase:
                cshape = core::BT_CSfromBBox(r_static->mesh->m_boundingBox, true, true);
                break;

            case CollisionShape::TriMesh:
                cshape = core::BT_CSfromMesh(r_static->mesh, true, true, true);
                break;

            case CollisionShape::TriMeshConvex:
                cshape = core::BT_CSfromMesh(r_static->mesh, true, true, false);
                break;

            default:
                cshape = nullptr;
                break;
        };

        if(!cshape)
            continue;

        btTransform startTransform;
        startTransform.setFromOpenGLMatrix(glm::value_ptr(r_static->transform));
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        btVector3 localInertia(0, 0, 0);
        r_static->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
        getWorld()->m_engine->bullet.dynamicsWorld->addRigidBody(r_static->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
        r_static->bt_body->setUserPointer(r_static.get());
    }

    /*
     * sprites loading section
     */
    for(size_t i = 0; i < tr_room->sprites.size(); i++)
    {
        m_sprites.emplace_back();
        if(tr_room->sprites[i].texture < 0 || static_cast<size_t>(tr_room->sprites[i].texture) >= getWorld()->m_sprites.size())
            continue;

        m_sprites[i].sprite = &getWorld()->m_sprites[tr_room->sprites[i].texture];
        m_sprites[i].pos = util::convert(tr_room->vertices[tr_room->sprites[i].vertex].vertex);
        m_sprites[i].pos += glm::vec3(getModelMatrix()[3]);
    }

    /*
     * let us load sectors
     */
    m_sectors.resize(boost::extents[tr_room->num_xsectors][tr_room->num_zsectors]);

    /*
     * base sectors information loading and collisional mesh creation
     */

     // To avoid manipulating with unnecessary information, we declare simple
     // heightmap here, which will be operated with sector and floordata parsing,
     // then vertical inbetween polys will be constructed, and Bullet collisional
     // object will be created. Afterwards, this heightmap also can be used to
     // quickly detect slopes for pushable blocks and other entities that rely on
     // floor level.

    for(size_t i = 0; i < m_sectors.num_elements(); i++)
    {
        const auto indexX = i / m_sectors.shape()[1];
        const auto indexY = i % m_sectors.shape()[1];
        RoomSector& sector = m_sectors[indexX][indexY];
        // Filling base sectors information.

        sector.index_x = indexX;
        sector.index_y = indexY;

        sector.position[0] = getModelMatrix()[3][0] + sector.index_x * MeteringSectorSize + 0.5f * MeteringSectorSize;
        sector.position[1] = getModelMatrix()[3][1] + sector.index_y * MeteringSectorSize + 0.5f * MeteringSectorSize;
        sector.position[2] = 0.5f * (tr_room->y_bottom + tr_room->y_top);

        sector.owner_room = this;

        if(tr->m_gameVersion < loader::Game::TR3)
        {
            sector.box_index = tr_room->sector_list[i].box_index;
            sector.material = SECTOR_MATERIAL_STONE;
        }
        else
        {
            sector.box_index = (tr_room->sector_list[i].box_index & 0xFFF0) >> 4;
            sector.material = tr_room->sector_list[i].box_index & 0x000F;
        }

        if(sector.box_index == 0xFFFF)
            sector.box_index = -1;

        sector.flags = 0;  // Clear sector flags.

        sector.floor = -MeteringStep * static_cast<int>(tr_room->sector_list[i].floor);
        sector.ceiling = -MeteringStep * static_cast<int>(tr_room->sector_list[i].ceiling);
        sector.trig_index = tr_room->sector_list[i].fd_index;

        // BUILDING CEILING HEIGHTMAP.

        // Penetration config is used later to build inbetween vertical collision polys.
        // If sector's penetration config is a wall, we simply build a vertical plane to
        // isolate this sector from top to bottom. Also, this allows to trick out wall
        // sectors inside another wall sectors to be ignored completely when building
        // collisional mesh.
        // Door penetration config means that we should either ignore sector collision
        // completely (classic door) or ignore one of the triangular sector parts (TR3+).

        if(sector.ceiling == MeteringWallHeight)
        {
            sector.ceiling_penetration_config = PenetrationConfig::Wall;
        }
        else if(tr_room->sector_list[i].room_above != 0xFF)
        {
            sector.ceiling_penetration_config = PenetrationConfig::Ghost;
        }
        else
        {
            sector.ceiling_penetration_config = PenetrationConfig::Solid;
        }

        // Reset some sector parameters to avoid garbaged memory issues.

        sector.portal_to_room = boost::none;
        sector.ceiling_diagonal_type = DiagonalType::None;
        sector.floor_diagonal_type = DiagonalType::None;

        // Now, we define heightmap cells position and draft (flat) height.
        // Draft height is derived from sector's floor and ceiling values, which are
        // copied into heightmap cells Y coordinates. As result, we receive flat
        // heightmap cell, which will be operated later with floordata.

        sector.ceiling_corners[0][0] = sector.index_x * MeteringSectorSize;
        sector.ceiling_corners[0][1] = sector.index_y * MeteringSectorSize + MeteringSectorSize;
        sector.ceiling_corners[0][2] = sector.ceiling;

        sector.ceiling_corners[1][0] = sector.index_x * MeteringSectorSize + MeteringSectorSize;
        sector.ceiling_corners[1][1] = sector.index_y * MeteringSectorSize + MeteringSectorSize;
        sector.ceiling_corners[1][2] = sector.ceiling;

        sector.ceiling_corners[2][0] = sector.index_x * MeteringSectorSize + MeteringSectorSize;
        sector.ceiling_corners[2][1] = sector.index_y * MeteringSectorSize;
        sector.ceiling_corners[2][2] = sector.ceiling;

        sector.ceiling_corners[3][0] = sector.index_x * MeteringSectorSize;
        sector.ceiling_corners[3][1] = sector.index_y * MeteringSectorSize;
        sector.ceiling_corners[3][2] = sector.ceiling;

        // BUILDING FLOOR HEIGHTMAP.

        // Features same steps as for the ceiling.

        if(sector.floor == MeteringWallHeight)
        {
            sector.floor_penetration_config = PenetrationConfig::Wall;
        }
        else if(tr_room->sector_list[i].room_below != 0xFF)
        {
            sector.floor_penetration_config = PenetrationConfig::Ghost;
        }
        else
        {
            sector.floor_penetration_config = PenetrationConfig::Solid;
        }

        sector.floor_corners[0][0] = sector.index_x * MeteringSectorSize;
        sector.floor_corners[0][1] = sector.index_y * MeteringSectorSize + MeteringSectorSize;
        sector.floor_corners[0][2] = sector.floor;

        sector.floor_corners[1][0] = sector.index_x * MeteringSectorSize + MeteringSectorSize;
        sector.floor_corners[1][1] = sector.index_y * MeteringSectorSize + MeteringSectorSize;
        sector.floor_corners[1][2] = sector.floor;

        sector.floor_corners[2][0] = sector.index_x * MeteringSectorSize + MeteringSectorSize;
        sector.floor_corners[2][1] = sector.index_y * MeteringSectorSize;
        sector.floor_corners[2][2] = sector.floor;

        sector.floor_corners[3][0] = sector.index_x * MeteringSectorSize;
        sector.floor_corners[3][1] = sector.index_y * MeteringSectorSize;
        sector.floor_corners[3][2] = sector.floor;
    }

    /*
     *  load lights
     */
    m_lights.reserve(tr_room->lights.size());

    for(size_t i = 0; i < tr_room->lights.size(); i++)
    {
        core::Light lgt;
        lgt.light_type = tr_room->lights[i].getLightType();

        lgt.position[0] = tr_room->lights[i].position.x;
        lgt.position[1] = -tr_room->lights[i].position.z;
        lgt.position[2] = tr_room->lights[i].position.y;

        if(lgt.light_type == loader::LightType::Shadow)
        {
            lgt.color[0] = -(tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
            lgt.color[1] = -(tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
            lgt.color[2] = -(tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
            lgt.color[3] = 1.0f;
        }
        else
        {
            lgt.color[0] = tr_room->lights[i].color.r / 255.0f * tr_room->lights[i].intensity;
            lgt.color[1] = tr_room->lights[i].color.g / 255.0f * tr_room->lights[i].intensity;
            lgt.color[2] = tr_room->lights[i].color.b / 255.0f * tr_room->lights[i].intensity;
            lgt.color[3] = 1.0f;
        }

        lgt.inner = tr_room->lights[i].r_inner;
        lgt.outer = tr_room->lights[i].r_outer;
        lgt.length = tr_room->lights[i].length;
        lgt.cutoff = tr_room->lights[i].cutoff;

        lgt.falloff = 0.001f / lgt.outer;

        m_lights.emplace_back(std::move(lgt));
    }

    /*
     * portals loading / calculation!!!
     */
    for(const loader::Portal& p : tr_room->portals)
    {
        std::shared_ptr<Room> r_dest = getWorld()->m_rooms[p.adjoining_room];
        m_portals.emplace_back(p, this, r_dest.get(), getModelMatrix());
    }

    /*
     * room borders calculation
     */
    m_boundingBox.min[2] = tr_room->y_bottom;
    m_boundingBox.max[2] = tr_room->y_top;

    m_boundingBox.min[0] = getModelMatrix()[3][0] + MeteringSectorSize;
    m_boundingBox.min[1] = getModelMatrix()[3][1] + MeteringSectorSize;
    m_boundingBox.max[0] = getModelMatrix()[3][0] + MeteringSectorSize * m_sectors.shape()[0] - MeteringSectorSize;
    m_boundingBox.max[1] = getModelMatrix()[3][1] + MeteringSectorSize * m_sectors.shape()[1] - MeteringSectorSize;

    /*
     * alternate room pointer calculation if one exists.
     */
    m_alternateRoom = nullptr;
    m_baseRoom = nullptr;

    if(tr_room->alternate_room >= 0 && static_cast<uint32_t>(tr_room->alternate_room) < tr->m_rooms.size())
    {
        m_alternateRoom = getWorld()->m_rooms[tr_room->alternate_room].get();
    }
}

///@TODO: resolve floor >> ceiling case
std::vector<SectorTween> Room::generateTweens() const
{
    std::vector<SectorTween> result;
    for(size_t h = 0; h < m_sectors.shape()[1] - 1; h++)
    {
        for(size_t w = 0; w < m_sectors.shape()[0] - 1; w++)
        {
            result.emplace_back();
            SectorTween* room_tween = &result.back();
            // Init X-plane tween [ | ]

            const RoomSector* current_heightmap = &m_sectors[w][h];
            const RoomSector* next_heightmap = current_heightmap + 1;
            bool joined_floors = false;
            bool joined_ceilings = false;

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

            if(w > 0)
            {
                if(next_heightmap->floor_penetration_config != PenetrationConfig::Wall || current_heightmap->floor_penetration_config != PenetrationConfig::Wall)                                                           // Init X-plane tween [ | ]
                {
                    if(Res_Sector_IsWall(*getWorld(), next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[0][2];
                        room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                        room_tween->setFloorConfig();
                        room_tween->ceiling_tween_type = TweenType::None;
                        joined_floors = true;
                        joined_ceilings = true;
                    }
                    else if(Res_Sector_IsWall(*getWorld(), current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                        room_tween->floor_corners[3][2] = next_heightmap->floor_corners[2][2];
                        room_tween->setFloorConfig();
                        room_tween->ceiling_tween_type = TweenType::None;
                        joined_floors = true;
                        joined_ceilings = true;
                    }
                    else
                    {
                        /************************** SECTION WITH DROPS CALCULATIONS **********************/
                        if((!current_heightmap->portal_to_room && !next_heightmap->portal_to_room) || current_heightmap->is2SidePortals(*getWorld(), next_heightmap))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                            next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                            if(!current_heightmap->portal_to_room && !next_heightmap->portal_to_room && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                            {
                                if(current_heightmap->floor_penetration_config == PenetrationConfig::Solid || next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                                {
                                    room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                                    room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                                    room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                                    room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                                    room_tween->setFloorConfig();
                                    joined_floors = true;
                                }
                                if(current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid || next_heightmap->ceiling_penetration_config == PenetrationConfig::Solid)
                                {
                                    room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                                    room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                                    room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                                    room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                                    room_tween->setCeilingConfig();
                                    joined_ceilings = true;
                                }
                            }
                        }
                    }
                }

                current_heightmap = &m_sectors[w][h];
                next_heightmap = current_heightmap + 1;
                if(!joined_floors && (!current_heightmap->portal_to_room || !next_heightmap->portal_to_room))
                {
                    bool valid = false;
                    if(next_heightmap->portal_to_room && current_heightmap->sector_above != nullptr && current_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                        if(next_heightmap->owner_room->getId() == current_heightmap->sector_above->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->position);
                            if(rs && *rs->portal_to_room == next_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(current_heightmap->portal_to_room && next_heightmap->sector_above != nullptr && next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                        if(current_heightmap->owner_room->getId() == next_heightmap->sector_above->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->position);
                            if(rs && *rs->portal_to_room == current_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(valid && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                        room_tween->setFloorConfig();
                    }
                }

                current_heightmap = &m_sectors[w][h];
                next_heightmap = current_heightmap + 1;
                if(!joined_ceilings && (!current_heightmap->portal_to_room || !next_heightmap->portal_to_room))
                {
                    bool valid = false;
                    if(next_heightmap->portal_to_room && current_heightmap->sector_below != nullptr && current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid)
                    {
                        next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                        if(next_heightmap->owner_room->getId() == current_heightmap->sector_below->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->position);
                            if(rs && *rs->portal_to_room == next_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(current_heightmap->portal_to_room && next_heightmap->sector_below != nullptr && next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                        if(current_heightmap->owner_room->getId() == next_heightmap->sector_below->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(valid == 0)
                        {
                            const RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->position);
                            if(rs && *rs->portal_to_room == current_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(valid && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->setCeilingConfig();
                    }
                }
            }

            /*****************************************************************************************************
             ********************************   CENTRE  OF  THE  ALGORITHM   *************************************
             *****************************************************************************************************/

            result.emplace_back();
            room_tween = &result.back();
            current_heightmap = &m_sectors[w][h];
            next_heightmap = &m_sectors[w + 1][h];
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

            joined_floors = false;
            joined_ceilings = false;

            if(h > 0)
            {
                if(next_heightmap->floor_penetration_config != PenetrationConfig::Wall || current_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                {
                    // Init Y-plane tween  [ - ]
                    if(Res_Sector_IsWall(*getWorld(), next_heightmap, current_heightmap))
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                        room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[2][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                        room_tween->setFloorConfig();
                        room_tween->ceiling_tween_type = TweenType::None;
                        joined_floors = true;
                        joined_ceilings = true;
                    }
                    else if(Res_Sector_IsWall(*getWorld(), current_heightmap, next_heightmap))
                    {
                        room_tween->floor_corners[0][2] = next_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                        room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->floor_corners[3][2] = next_heightmap->floor_corners[3][2];
                        room_tween->setFloorConfig();
                        room_tween->ceiling_tween_type = TweenType::None;
                        joined_floors = true;
                        joined_ceilings = true;
                    }
                    else
                    {
                        /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
                        if((!current_heightmap->portal_to_room && !next_heightmap->portal_to_room) || current_heightmap->is2SidePortals(*getWorld(), next_heightmap))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                            next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                            if(!current_heightmap->portal_to_room && !next_heightmap->portal_to_room && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                            {
                                if(current_heightmap->floor_penetration_config == PenetrationConfig::Solid || next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                                {
                                    room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                                    room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                                    room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                                    room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                                    room_tween->setFloorConfig();
                                    joined_floors = true;
                                }
                                if(current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid || next_heightmap->ceiling_penetration_config == PenetrationConfig::Solid)
                                {
                                    room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                                    room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                                    room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                                    room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                                    room_tween->setCeilingConfig();
                                    joined_ceilings = true;
                                }
                            }
                        }
                    }
                }

                current_heightmap = &m_sectors[w][h];
                next_heightmap = &m_sectors[w + 1][h];
                if(!joined_floors && (!current_heightmap->portal_to_room || !next_heightmap->portal_to_room))
                {
                    bool valid = false;
                    if(next_heightmap->portal_to_room && current_heightmap->sector_above != nullptr && current_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                        if(next_heightmap->owner_room->getId() == current_heightmap->sector_above->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->position);
                            if(rs && *rs->portal_to_room == next_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(current_heightmap->portal_to_room && next_heightmap->sector_above != nullptr && next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                        if(current_heightmap->owner_room->getId() == next_heightmap->sector_above->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->position);
                            if(rs && *rs->portal_to_room == current_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(valid && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                    {
                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                        room_tween->setFloorConfig();
                    }
                }

                current_heightmap = &m_sectors[w][h];
                next_heightmap = &m_sectors[w + 1][h];
                if(!joined_ceilings && (!current_heightmap->portal_to_room || !next_heightmap->portal_to_room))
                {
                    bool valid = false;
                    if(next_heightmap->portal_to_room && current_heightmap->sector_below != nullptr && current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid)
                    {
                        next_heightmap = next_heightmap->checkPortalPointer(*getWorld());
                        if(next_heightmap->owner_room->getId() == current_heightmap->sector_below->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->position);
                            if(rs && *rs->portal_to_room == next_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(current_heightmap->portal_to_room && next_heightmap->sector_below != nullptr && next_heightmap->floor_penetration_config == PenetrationConfig::Solid)
                    {
                        current_heightmap = current_heightmap->checkPortalPointer(*getWorld());
                        if(current_heightmap->owner_room->getId() == next_heightmap->sector_below->owner_room->getId())
                        {
                            valid = true;
                        }
                        if(!valid)
                        {
                            const RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->position);
                            if(rs && *rs->portal_to_room == current_heightmap->owner_room->getId())
                            {
                                valid = true;
                            }
                        }
                    }

                    if(valid && current_heightmap->floor_penetration_config != PenetrationConfig::Wall && next_heightmap->floor_penetration_config != PenetrationConfig::Wall)
                    {
                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                        room_tween->setCeilingConfig();
                    }
                }
            }
        }    ///END for
    }    ///END for
    return result;
}

void Room::generateCollisionShape()
{
    // Inbetween polygons array is later filled by loop which scans adjacent
    // sector heightmaps and fills the gaps between them, thus creating inbetween
    // polygon. Inbetweens can be either quad (if all four corner heights are
    // different), triangle (if one corner height is similar to adjacent) or
    // ghost (if corner heights are completely similar). In case of quad inbetween,
    // two triangles are added to collisional trimesh, in case of triangle inbetween,
    // we add only one, and in case of ghost inbetween, we ignore it.

    // Most difficult task with converting floordata collision to trimesh collision is
    // building inbetween polygons which will block out gaps between sector heights.
    std::vector<SectorTween> room_tween = generateTweens();

    // Final step is sending actual sectors to Bullet collision model. We do it here.

    btCollisionShape *cshape = BT_CSfromHeightmap(m_sectors, room_tween, true, true);

    if(!cshape)
        return;

    btVector3 localInertia(0, 0, 0);
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(getModelMatrix()));
    btDefaultMotionState* motionState = new btDefaultMotionState(tr);
    m_btBody.reset(new btRigidBody(0.0, motionState, cshape, localInertia));
    getWorld()->m_engine->bullet.dynamicsWorld->addRigidBody(m_btBody.get(), COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
    m_btBody->setUserPointer(this);
    m_btBody->setRestitution(1.0);
    m_btBody->setFriction(1.0);
    setCollisionType(CollisionType::Static);                    // meshtree
    setCollisionShape(CollisionShape::TriMesh);
}

namespace
{
bool TR_IsSectorsIn2SideOfPortal(const RoomSector& s1, const RoomSector& s2, const Portal& p)
{
    if(util::fuzzyEqual(s1.position[0], s2.position[0]) && !util::fuzzyEqual(s1.position[1], s2.position[1]) && glm::abs(p.normal[1]) > 0.99)
    {
        glm::float_t min_x, max_x, min_y, max_y;
        max_x = min_x = p.vertices.front().x;
        for(const auto& v : p.vertices)
        {
            if(v.x > max_x)
            {
                max_x = v.x;
            }
            if(v.x < min_x)
            {
                min_x = v.x;
            }
        }
        if(s1.position[1] > s2.position[1])
        {
            min_y = s2.position[1];
            max_y = s1.position[1];
        }
        else
        {
            min_y = s1.position[1];
            max_y = s2.position[1];
        }

        if(s1.position[0] < max_x && s1.position[0] > min_x && p.center[1] < max_y && p.center[1] > min_y)
        {
            return true;
        }
    }
    else if(!util::fuzzyEqual(s1.position[0], s2.position[0]) && util::fuzzyEqual(s1.position[1], s2.position[1]) && glm::abs(p.normal[0]) > 0.99)
    {
        glm::float_t min_x, max_x, min_y, max_y;
        max_y = min_y = p.vertices.front().y;
        for(const auto& v : p.vertices)
        {
            if(v.y > max_y)
            {
                max_y = v.y;
            }
            if(v.y < min_y)
            {
                min_y = v.y;
            }
        }
        if(s1.position[0] > s2.position[0])
        {
            min_x = s2.position[0];
            max_x = s1.position[0];
        }
        else
        {
            min_x = s1.position[0];
            max_x = s2.position[0];
        }

        if(p.center[0] < max_x && p.center[0] > min_x && s1.position[1] < max_y && s1.position[1] > min_y)
        {
            return true;
        }
    }

    return false;
}
} // anonymous namespace

void Room::initVerticalSectorRelations(const loader::Room& tr)
{
    for(size_t i = 0; i < m_sectors.num_elements(); i++)
    {
        RoomSector& sector = m_sectors[i / m_sectors.shape()[1]][i%m_sectors.shape()[1]];
        /*
         * Let us fill pointers to sectors above and sectors below
         */

        uint8_t rp = tr.sector_list[i].room_below;
        sector.sector_below = nullptr;
        if(rp < getWorld()->m_rooms.size() && rp != 255)
        {
            sector.sector_below = getWorld()->m_rooms[rp]->getSectorRaw(sector.position);
        }
        rp = tr.sector_list[i].room_above;
        sector.sector_above = nullptr;
        if(rp < getWorld()->m_rooms.size() && rp != 255)
        {
            sector.sector_above = getWorld()->m_rooms[rp]->getSectorRaw(sector.position);
        }

        int dx = 0, dy = 0;
        /**** OX *****/
        if(sector.index_y > 0 && sector.index_y < m_sectors.shape()[1] - 1 && sector.index_x == 0)
        {
            dx = 1;
        }
        if(sector.index_y > 0 && sector.index_y < m_sectors.shape()[1] - 1 && sector.index_x == m_sectors.shape()[0] - 1)
        {
            dx = -1;
        }
        /**** OY *****/
        if(sector.index_x > 0 && sector.index_x < m_sectors.shape()[0] - 1 && sector.index_y == 0)
        {
            dy = 1;
        }
        if(sector.index_x > 0 && sector.index_x < m_sectors.shape()[0] - 1 && sector.index_y == m_sectors.shape()[1] - 1)
        {
            dy = -1;
        }

        const RoomSector& near_sector = m_sectors[i / m_sectors.shape()[1] + dx][i%m_sectors.shape()[1] + dy];

        if(dx == 0 || dy == 0 || sector.portal_to_room)
            continue;

        for(const Portal& p : m_portals)
        {
            if(!util::fuzzyZero(p.normal[2]))
                continue;

            const RoomSector* dst = p.destination ? p.destination->getSectorRaw(sector.position) : nullptr;
            if(dst == nullptr)
                continue;

            const RoomSector* orig_dst = getWorld()->m_rooms[*sector.portal_to_room]->getSectorRaw(sector.position);

            if(!dst->portal_to_room && dst->floor != MeteringWallHeight && dst->ceiling != MeteringWallHeight && *sector.portal_to_room != p.destination->getId() && dst->floor < orig_dst->floor && TR_IsSectorsIn2SideOfPortal(near_sector, *dst, p))
            {
                sector.portal_to_room = p.destination->getId();
            }
        }
    }
}

void Room::generateSpritesBuffer()
{
    // Find the number of different texture pages used and the number of non-null sprites
    size_t highestTexturePageFound = 0;
    int actualSpritesFound = 0;
    for(RoomSprite& sp : m_sprites)
    {
        if(!sp.sprite)
            continue;

        actualSpritesFound += 1;
        highestTexturePageFound = std::max(highestTexturePageFound, sp.sprite->texture);
    }
    if(actualSpritesFound == 0)
    {
        m_spriteBuffer = nullptr;
        return;
    }

    m_spriteBuffer = new core::SpriteBuffer();
    m_spriteBuffer->num_texture_pages = highestTexturePageFound + 1;
    m_spriteBuffer->element_count_per_texture.resize(m_spriteBuffer->num_texture_pages, 0);

    // First collect indices on a per-texture basis
    std::vector<std::vector<uint16_t>> elements_for_texture(highestTexturePageFound + 1);

    std::vector<glm::float_t> spriteData(actualSpritesFound * 4 * 7, 0);

    int writeIndex = 0;
    for(const RoomSprite& room_sprite : getSprites())
    {
        if(!room_sprite.sprite)
            continue;

        int vertexStart = writeIndex;
        // top right
        std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
        std::copy_n(glm::value_ptr(room_sprite.sprite->tex_coord[0]), 2, &spriteData[writeIndex * 7 + 3]);
        spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
        spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

        writeIndex += 1;

        // top left
        std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
        std::copy_n(glm::value_ptr(room_sprite.sprite->tex_coord[1]), 2, &spriteData[writeIndex * 7 + 3]);
        spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
        spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

        writeIndex += 1;

        // bottom left
        std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
        std::copy_n(glm::value_ptr(room_sprite.sprite->tex_coord[2]), 2, &spriteData[writeIndex * 7 + 3]);
        spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
        spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

        writeIndex += 1;

        // bottom right
        std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
        std::copy_n(glm::value_ptr(room_sprite.sprite->tex_coord[3]), 2, &spriteData[writeIndex * 7 + 3]);
        spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
        spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

        writeIndex += 1;

        // Assign indices
        size_t texture = room_sprite.sprite->texture;
        size_t start = m_spriteBuffer->element_count_per_texture[texture];
        size_t newElementCount = start + 6;
        m_spriteBuffer->element_count_per_texture[texture] = newElementCount;
        elements_for_texture[texture].resize(newElementCount);

        elements_for_texture[texture][start + 0] = vertexStart + 0;
        elements_for_texture[texture][start + 1] = vertexStart + 1;
        elements_for_texture[texture][start + 2] = vertexStart + 2;
        elements_for_texture[texture][start + 3] = vertexStart + 2;
        elements_for_texture[texture][start + 4] = vertexStart + 3;
        elements_for_texture[texture][start + 5] = vertexStart + 0;
    }

    // Now flatten all these indices to a single array
    std::vector<uint16_t> elements;
    for(uint32_t i = 0; i <= highestTexturePageFound; i++)
    {
        if(elements_for_texture[i].empty())
        {
            continue;
        }
        BOOST_ASSERT(elements_for_texture[i].size() >= getSpriteBuffer()->element_count_per_texture[i]);
        std::copy_n(elements_for_texture[i].begin(), getSpriteBuffer()->element_count_per_texture[i], std::back_inserter(elements));
        elements_for_texture[i].clear();
    }
    elements_for_texture.clear();

    // Now load into OpenGL
    GLuint arrayBuffer, elementBuffer;
    glGenBuffers(1, &arrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[7]) * 4 * actualSpritesFound, spriteData.data(), GL_STATIC_DRAW);
    spriteData.clear();

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements[0]) * elements.size(), elements.data(), GL_STATIC_DRAW);
    elements.clear();

    render::VertexArrayAttribute attribs[3] = {
        render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::position,      3, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), 0),
        render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::tex_coord,     2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), sizeof(GLfloat[3])),
        render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::corner_offset, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), sizeof(GLfloat[5]))
    };

    m_spriteBuffer->data.reset(new render::VertexArray(elementBuffer, 3, attribs));
}

void Room::generateProperties(const loader::Room& tr, const loader::FloorData& floorData, loader::Engine engine)
{
    if(m_alternateRoom != nullptr)
    {
        m_alternateRoom->m_baseRoom = this;   // Refill base room pointer.
    }

    // Fill heightmap and translate floordata.
    for(auto column : m_sectors)
    {
        for(RoomSector& sector : column)
        {
            TR_Sector_TranslateFloorData(*getWorld(), sector, floorData, engine);
            Res_Sector_FixHeights(sector);
        }
    }

    // Generate links to the near rooms.
    buildNearRoomsList();
    // Generate links to the overlapped rooms.
    buildOverlappedRoomsList();

    // Basic sector calculations.
    initVerticalSectorRelations(tr);
}
} // namespace *getWorld()
