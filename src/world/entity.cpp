#include "entity.h"

#include "LuaState.h"

#include "character_controller.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "gui/console.h"
#include "ragdoll.h"
#include "script/script.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world.h"
#include "world/animation/animcommands.h"
#include "world/core/orientedboundingbox.h"
#include "world/core/util.h"
#include "world/room.h"
#include "world/skeletalmodel.h"

#include "core/basemesh.h"

#include <cmath>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/log/trivial.hpp>

constexpr float GhostVolumeCollisionCoefficient = 0.4f;

namespace world
{

void Entity::createGhosts()
{
    if(!m_bf.animations.model || m_bf.animations.model->mesh_count <= 0)
        return;

    m_bt.manifoldArray.reset(new btManifoldArray());
    m_bt.shapes.clear();
    m_bt.ghostObjects.clear();
    m_bt.last_collisions.clear();
    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        glm::vec3 box = GhostVolumeCollisionCoefficient * m_bf.bone_tags[i].mesh_base->boundingBox.getDiameter();
        m_bt.shapes.emplace_back(new btBoxShape(util::convert(box)));
        m_bt.shapes.back()->setMargin(COLLISION_MARGIN_DEFAULT);
        m_bf.bone_tags[i].mesh_base->m_radius = std::min(std::min(box.x, box.y), box.z);

        m_bt.ghostObjects.emplace_back(new btPairCachingGhostObject());

        m_bt.ghostObjects.back()->setIgnoreCollisionCheck(m_bt.bt_body[i].get(), true);

        glm::mat4 gltr = m_transform * m_bf.bone_tags[i].full_transform;
        gltr[3] = glm::vec4( glm::mat3(gltr) * m_bf.bone_tags[i].mesh_base->m_center, 1.0f );

        m_bt.ghostObjects.back()->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(gltr));
        m_bt.ghostObjects.back()->setCollisionFlags(m_bt.ghostObjects.back()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_CHARACTER_OBJECT);
        m_bt.ghostObjects.back()->setUserPointer(this);
        m_bt.ghostObjects.back()->setCollisionShape(m_bt.shapes.back().get());
        engine::bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects.back().get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);

        m_bt.last_collisions.emplace_back();
    }
}

void Entity::enable()
{
    if(!m_enabled)
    {
        enableCollision();
        m_enabled = m_active = m_visible = true;
    }
}

void Entity::disable()
{
    if(m_enabled)
    {
        disableCollision();
        m_active = m_enabled = m_visible = false;
    }
}

/**
 * This function enables collision for entity_p in all cases exept NULL models.
 * If collision models does not exists, function will create them;
 * @param ent - pointer to the entity.
 */
void Entity::enableCollision()
{
    if(!m_bt.bt_body.empty())
    {
        for(const auto& b : m_bt.bt_body)
        {
            if(b && !b->isInWorld())
            {
                engine::bt_engine_dynamicsWorld->addRigidBody(b.get());
            }
        }
    }
}

void Entity::disableCollision()
{
    if(!m_bt.bt_body.empty())
    {
        for(const auto& b : m_bt.bt_body)
        {
            if(b && b->isInWorld())
            {
                engine::bt_engine_dynamicsWorld->removeRigidBody(b.get());
            }
        }
    }
}

void Entity::genRigidBody()
{
    if(!m_bf.animations.model || getCollisionType() == world::CollisionType::None)
        return;

    m_bt.bt_body.clear();

    for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        std::shared_ptr<core::BaseMesh> mesh = m_bf.animations.model->mesh_tree[i].mesh_base;
        btCollisionShape *cshape;
        switch(getCollisionShape())
        {
            case world::CollisionShape::Sphere:
                cshape = core::BT_CSfromSphere(mesh->m_radius);
                break;

            case world::CollisionShape::TriMeshConvex:
                cshape = core::BT_CSfromMesh(mesh, true, true, false);
                break;

            case world::CollisionShape::TriMesh:
                cshape = core::BT_CSfromMesh(mesh, true, true, true);
                break;

            case world::CollisionShape::Box:
            default:
                cshape = core::BT_CSfromBBox(mesh->boundingBox, true, true);
                break;
        };

        m_bt.bt_body.emplace_back();

        if(cshape)
        {
            btVector3 localInertia(0, 0, 0);
            if(getCollisionShape() != world::CollisionShape::TriMesh)
                cshape->calculateLocalInertia(0.0, localInertia);

            btTransform startTransform;
            startTransform.setFromOpenGLMatrix(glm::value_ptr( m_transform * m_bf.bone_tags[i].full_transform ));
            m_bt.bt_body.back().reset(new btRigidBody(0.0, new btDefaultMotionState(startTransform), cshape, localInertia));

            switch(getCollisionType())
            {
                case world::CollisionType::Kinematic:
                    m_bt.bt_body.back()->setCollisionFlags(m_bt.bt_body.back()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                    break;

                case world::CollisionType::Ghost:
                    m_bt.bt_body.back()->setCollisionFlags(m_bt.bt_body.back()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
                    break;

                case world::CollisionType::Actor:
                case world::CollisionType::Vehicle:
                    m_bt.bt_body.back()->setCollisionFlags(m_bt.bt_body.back()->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
                    break;

                case world::CollisionType::Static:
                default:
                    m_bt.bt_body.back()->setCollisionFlags(m_bt.bt_body.back()->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
                    break;
            }

            engine::bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
            m_bt.bt_body.back()->setUserPointer(this);
        }
    }
}

/**
 * It is from bullet_character_controller
 */
int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, glm::vec3* correction)
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    int ret = 0;
    int num_pairs, manifolds_size;
    btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
    btVector3 aabb_min, aabb_max;

    ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
    engine::bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, engine::bt_engine_dynamicsWorld->getDispatcher());
    engine::bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), engine::bt_engine_dynamicsWorld->getDispatchInfo(), engine::bt_engine_dynamicsWorld->getDispatcher());

    *correction = {0,0,0};
    num_pairs = ghost->getOverlappingPairCache()->getNumOverlappingPairs();
    for(int i = 0; i < num_pairs; i++)
    {
        manifoldArray->clear();
        // do not use commented code: it prevents to collision skips.
        //btBroadphasePair &pair = pairArray[i];
        //btBroadphasePair* collisionPair = bt_engine_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
        btBroadphasePair *collisionPair = &pairArray[i];

        if(!collisionPair)
        {
            continue;
        }

        if(collisionPair->m_algorithm)
        {
            collisionPair->m_algorithm->getAllContactManifolds(*manifoldArray);
        }

        manifolds_size = manifoldArray->size();
        for(int j = 0; j < manifolds_size; j++)
        {
            btPersistentManifold* manifold = (*manifoldArray)[j];
            glm::float_t directionSign = manifold->getBody0() == ghost ? glm::float_t(-1.0) : glm::float_t(1.0);
            Object* cont0 = static_cast<Object*>(manifold->getBody0()->getUserPointer());
            Object* cont1 = static_cast<Object*>(manifold->getBody1()->getUserPointer());
            if(cont0->getCollisionType() == world::CollisionType::Ghost || cont1->getCollisionType() == world::CollisionType::Ghost)
            {
                continue;
            }
            for(int k = 0; k < manifold->getNumContacts(); k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                glm::float_t dist = pt.getDistance();

                if(dist < 0.0)
                {
                    glm::vec3 t = util::convert(pt.m_normalWorldOnB) * dist * directionSign;
                    *correction += t;
                    ret++;
                }
            }
        }
    }

    return ret;
}

void Entity::ghostUpdate()
{
    if(m_bt.ghostObjects.empty())
        return;

    BOOST_ASSERT(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_transform * m_bf.bone_tags[i].full_transform;
            auto v = m_bf.animations.model->mesh_tree[i].mesh_base->m_center;
            m_bt.ghostObjects[i]->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr( tr ));
            auto pos = tr * glm::vec4(v, 1);
            m_bt.ghostObjects[i]->getWorldTransform().setOrigin(util::convert(glm::vec3(pos)));
        }
    }
    else
    {
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            tr.setOrigin(tr * util::convert(m_bf.bone_tags[i].mesh_base->m_center));
            m_bt.ghostObjects[i]->getWorldTransform() = tr;
        }
    }
}

void Entity::updateCurrentCollisions()
{
    if(m_bt.ghostObjects.empty())
        return;

    BOOST_ASSERT(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        const std::unique_ptr<btPairCachingGhostObject>& ghost = m_bt.ghostObjects[i];
        EntityCollisionNode& cn = m_bt.last_collisions[i];

        cn.obj.clear();
        auto tr = m_transform * m_bf.bone_tags[i].full_transform;
        auto v = m_bf.animations.model->mesh_tree[i].mesh_base->m_center;
        auto orig_tr = ghost->getWorldTransform();
        ghost->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr));
        auto pos = glm::vec3(tr * glm::vec4(v, 1.0f));
        ghost->getWorldTransform().setOrigin(util::convert(pos));

        btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
        btVector3 aabb_min, aabb_max;

        ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
        engine::bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, engine::bt_engine_dynamicsWorld->getDispatcher());
        engine::bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), engine::bt_engine_dynamicsWorld->getDispatchInfo(), engine::bt_engine_dynamicsWorld->getDispatcher());

        int num_pairs = ghost->getOverlappingPairCache()->getNumOverlappingPairs();
        for(int j = 0; j < num_pairs; j++)
        {
            m_bt.manifoldArray->clear();
            btBroadphasePair *collisionPair = &pairArray[j];

            if(!collisionPair)
            {
                continue;
            }

            if(collisionPair->m_algorithm)
            {
                collisionPair->m_algorithm->getAllContactManifolds(*m_bt.manifoldArray);
            }

            for(int k = 0; k < m_bt.manifoldArray->size(); k++)
            {
                btPersistentManifold* manifold = (*m_bt.manifoldArray)[k];
                for(int c = 0; c < manifold->getNumContacts(); c++)
                {
                    if(manifold->getContactPoint(c).getDistance() < 0.0)
                    {
                        cn.obj.emplace_back();
                        cn.obj.back() = const_cast<btCollisionObject*>((*m_bt.manifoldArray)[k]->getBody0());
                        if(this == static_cast<Entity*>(cn.obj.back()->getUserPointer()))
                        {
                            cn.obj.back() = const_cast<btCollisionObject*>((*m_bt.manifoldArray)[k]->getBody1());
                        }
                        break;
                    }
                }
            }
        }
        ghost->setWorldTransform(orig_tr);
    }
}

///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity::getPenetrationFixVector(glm::vec3* reaction, bool hasMove)
{
    *reaction = { 0,0,0 };
    if(m_bt.ghostObjects.empty() || m_bt.no_fix_all)
        return 0;

    BOOST_ASSERT(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    auto orig_pos = m_transform[3];
    int ret = 0;
    for(size_t i = 0; i < m_bf.animations.model->collision_map.size(); i++)
    {
        uint16_t m = m_bf.animations.model->collision_map[i];
        animation::SSBoneTag* btag = &m_bf.bone_tags[m];

        if(btag->body_part & m_bt.no_fix_body_parts)
        {
            continue;
        }

        // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
        glm::vec3 from;
        if((btag->parent == nullptr) || (hasMove && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
        {
            from = util::convert( m_bt.ghostObjects[m]->getWorldTransform().getOrigin() );
            from += glm::vec3(m_transform[3] - orig_pos);
        }
        else
        {
            glm::vec4 parent_from = btag->parent->full_transform * glm::vec4(btag->parent->mesh_base->m_center,1);
            from = glm::vec3(m_transform * parent_from);
        }

        auto tr = m_transform * btag->full_transform;
        auto to = glm::vec3(tr * glm::vec4(btag->mesh_base->m_center, 1.0f));
        auto curr = from;
        auto move = to - from;
        auto move_len = move.length();
        if((i == 0) && (move_len > 1024.0))                                 ///@FIXME: magick const 1024.0!
        {
            break;
        }
        int iter = static_cast<int>((4.0 * move_len / btag->mesh_base->m_radius) + 1);     ///@FIXME (not a critical): magick const 4.0!
        move /= static_cast<glm::float_t>(iter);

        for(int j = 0; j <= iter; j++)
        {
            tr[3] = glm::vec4(curr, 1.0f);
            auto tr_current = tr;
            m_bt.ghostObjects[m]->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr_current));
            glm::vec3 tmp;
            if(Ghost_GetPenetrationFixVector(m_bt.ghostObjects[m].get(), m_bt.manifoldArray.get(), &tmp))
            {
                m_transform[3] += glm::vec4(tmp,0);
                curr += tmp;
                from += tmp;
                ret++;
            }
            curr += move;
        }
    }
    *reaction = glm::vec3(m_transform[3] - orig_pos);
    m_transform[3] = orig_pos;

    return ret;
}

void Entity::fixPenetrations(const glm::vec3* move)
{
    if(m_bt.ghostObjects.empty())
        return;

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    if(m_bt.no_fix_all)
    {
        ghostUpdate();
        return;
    }

    glm::vec3 reaction;
    getPenetrationFixVector(&reaction, move != nullptr);
    m_transform[3] += glm::vec4(reaction, 0);

    ghostUpdate();
}

void Entity::transferToRoom(Room* room)
{
    if(getRoom() && !getRoom()->isOverlapped(room))
    {
        if(getRoom())
            getRoom()->removeEntity(this);
        if(room)
            room->addEntity(this);
    }
}

std::shared_ptr<engine::BtEngineClosestConvexResultCallback> Entity::callbackForCamera() const
{
    auto cb = std::make_shared<engine::BtEngineClosestConvexResultCallback>(this);
    cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    return cb;
}

void Entity::checkCollisionCallbacks()
{
    if(m_bt.ghostObjects.empty())
        return;

    btCollisionObject *cobj;
    uint32_t curr_flag;
    updateCurrentCollisions();
    while((cobj = getRemoveCollisionBodyParts(0xFFFFFFFF, &curr_flag)) != nullptr)
    {
        // do callbacks here:
        Object* cont = static_cast<Object*>(cobj->getUserPointer());

        if(Entity* activator = dynamic_cast<Entity*>(cont))
        {
            if(activator->m_callbackFlags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                engine_lua.execEntity(ENTITY_CALLBACK_COLLISION, activator->getId(), getId());
                //ConsoleInfo::instance().printf("char_body_flag = 0x%X, collider_type = %d", curr_flag, type);
            }
        }
        else if((m_callbackFlags & ENTITY_CALLBACK_ROOMCOLLISION) && dynamic_cast<Room*>(cont))
        {
            engine_lua.execEntity(ENTITY_CALLBACK_ROOMCOLLISION, getId(), static_cast<Room*>(cont)->getId());
        }
    }
}

bool Entity::wasCollisionBodyParts(uint32_t parts_flags)
{
    if(m_bt.last_collisions.empty())
        return false;

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        if((m_bf.bone_tags[i].body_part & parts_flags) && !m_bt.last_collisions[i].obj.empty())
        {
            return true;
        }
    }

    return false;
}

void Entity::cleanCollisionAllBodyParts()
{
    if(m_bt.last_collisions.empty())
        return;

    for(auto& coll : m_bt.last_collisions)
    {
        coll.obj.clear();
    }
}

void Entity::cleanCollisionBodyParts(uint32_t parts_flags)
{
    if(m_bt.last_collisions.empty())
        return;

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        if(m_bf.bone_tags[i].body_part & parts_flags)
        {
            m_bt.last_collisions[i].obj.clear();
        }
    }
}

btCollisionObject* Entity::getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t *curr_flag)
{
    *curr_flag = 0x00;
    if(m_bt.last_collisions.empty())
        return nullptr;

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        if(m_bf.bone_tags[i].body_part & parts_flags)
        {
            EntityCollisionNode& cn = m_bt.last_collisions[i];
            if(!cn.obj.empty())
            {
                *curr_flag = m_bf.bone_tags[i].body_part;
                auto res = cn.obj.back();
                cn.obj.pop_back();
                return res;
            }
        }
    }

    return nullptr;
}

void Entity::updateRoomPos()
{
    glm::vec3 pos = getRoomPos();
    auto new_room = Room_FindPosCogerrence(pos, getRoom());
    if(!new_room)
    {
        m_currentSector = nullptr;
        return;
    }

    RoomSector* new_sector = new_room->getSectorXYZ(pos);
    if(new_room != new_sector->owner_room.get())
    {
        new_room = new_sector->owner_room.get();
    }

    transferToRoom(new_room);

    setRoom( new_room );
    m_lastSector = m_currentSector;

    if(m_currentSector != new_sector)
    {
        m_triggerLayout &= static_cast<uint8_t>(~ENTITY_TLAYOUT_SSTATUS); // Reset sector status.
        m_currentSector = new_sector;
    }
}

void Entity::updateRigidBody(bool force)
{
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        //glm::vec3 pos = bt.bt_body[0]->getWorldTransform()[3];
        //vec3_copy(transform+12, pos);
        m_bt.bt_body[0]->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_transform));
        updateRoomPos();
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            glm::mat4 tr;
            m_bt.bt_body[i]->getWorldTransform().getOpenGLMatrix(glm::value_ptr(tr));
            m_bf.bone_tags[i].full_transform = glm::inverse(m_transform) * tr;
        }

        // that cycle is necessary only for skinning models;
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            if(m_bf.bone_tags[i].parent != nullptr)
            {
                m_bf.bone_tags[i].transform = glm::inverse(m_bf.bone_tags[i].parent->full_transform) * m_bf.bone_tags[i].full_transform;
            }
            else
            {
                m_bf.bone_tags[i].transform = m_bf.bone_tags[i].full_transform;
            }
        }

        updateGhostRigidBody();

        if(m_bf.bone_tags.size() == 1)
        {
            m_bf.boundingBox = m_bf.bone_tags[0].mesh_base->boundingBox;
        }
        else
        {
            m_bf.boundingBox = m_bf.bone_tags[0].mesh_base->boundingBox;
            for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
            {
                auto& pos = m_bf.bone_tags[i].full_transform[3];
                auto& boundingBox = m_bf.bone_tags[i].mesh_base->boundingBox;
                glm::float_t r = boundingBox.max[0] - boundingBox.min[0];
                glm::float_t t = boundingBox.max[1] - boundingBox.min[1];
                r = (t > r) ? (t) : (r);
                t = boundingBox.max[2] - boundingBox.min[2];
                r = (t > r) ? (t) : (r);
                r *= 0.5;

                if(m_bf.boundingBox.min[0] > pos[0] - r)
                {
                    m_bf.boundingBox.min[0] = pos[0] - r;
                }
                if(m_bf.boundingBox.min[1] > pos[1] - r)
                {
                    m_bf.boundingBox.min[1] = pos[1] - r;
                }
                if(m_bf.boundingBox.min[2] > pos[2] - r)
                {
                    m_bf.boundingBox.min[2] = pos[2] - r;
                }

                if(m_bf.boundingBox.max[0] < pos[0] + r)
                {
                    m_bf.boundingBox.max[0] = pos[0] + r;
                }
                if(m_bf.boundingBox.max[1] < pos[1] + r)
                {
                    m_bf.boundingBox.max[1] = pos[1] + r;
                }
                if(m_bf.boundingBox.max[2] < pos[2] + r)
                {
                    m_bf.boundingBox.max[2] = pos[2] + r;
                }
            }
        }
    }
    else
    {
        if((m_bf.animations.model == nullptr) ||
           m_bt.bt_body.empty() ||
           (!force && (m_bf.animations.model->animations.size() == 1) && (m_bf.animations.model->animations.front().frames.size() == 1)))
        {
            return;
        }

        updateRoomPos();
        if(getCollisionType() != world::CollisionType::Static)
        {
            for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
            {
                if(m_bt.bt_body[i])
                {
                    m_bt.bt_body[i]->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(m_transform * m_bf.bone_tags[i].full_transform));
                }
            }
        }
    }
    rebuildBV();
}

void Entity::updateTransform()
{
    m_angles[0] = util::wrapAngle(m_angles[0]);
    m_angles[1] = util::wrapAngle(m_angles[1]);
    m_angles[2] = util::wrapAngle(m_angles[2]);

    glm::vec3 ang = glm::radians(m_angles);

    auto pos = m_transform[3];
    m_transform = glm::yawPitchRoll(ang[2], ang[1], ang[0]);
    m_transform[3] = pos;
}

void Entity::updateCurrentSpeed(bool zeroVz)
{
    glm::float_t t = m_currentSpeed * animation::FrameRate;
    glm::float_t vz = (zeroVz) ? (0.0f) : (m_speed[2]);

    if(m_moveDir == MoveDirection::Forward)
    {
        m_speed = glm::vec3(m_transform[1]) * t;
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        m_speed = glm::vec3(m_transform[1]) * -t;
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        m_speed = glm::vec3(m_transform[0]) * -t;
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        m_speed = glm::vec3(m_transform[0]) * t;
    }
    else
    {
        m_speed = {0,0,0};
    }

    m_speed[2] = vz;
}

void Entity::addOverrideAnim(int model_id)
{
    SkeletalModel* sm = engine::engine_world.getModelByID(model_id);

    if((sm != nullptr) && (sm->mesh_count == m_bf.bone_tags.size()))
    {
        animation::SSAnimation* ss_anim = new animation::SSAnimation();

        ss_anim->model = sm;
        ss_anim->onFrame = nullptr;
        ss_anim->next = m_bf.animations.next;
        m_bf.animations.next = ss_anim;

        ss_anim->frame_time = 0.0;
        ss_anim->next_state = LaraState::WALK_FORWARD;
        ss_anim->lerp = 0.0;
        ss_anim->current_animation = 0;
        ss_anim->current_frame = 0;
    }
}

glm::float_t Entity::findDistance(const Entity& other)
{
    return glm::distance(m_transform[3], other.m_transform[3]);
}


/**
 * Callback to handle anim commands
 * @param command
 */
void Entity::doAnimCommand(const animation::AnimCommand& command)
{
    switch(command.cmdId)
    {
        case TR_ANIMCOMMAND_SETPOSITION:    // (tr_x, tr_y, tr_z)
            {
                // x=x, y=z, z=-y
                const glm::float_t x = glm::float_t(command.param[0]);
                const glm::float_t y = glm::float_t(command.param[2]);
                const glm::float_t z = -glm::float_t(command.param[1]);
                glm::vec3 ofs(x, y, z);
                m_transform[3] += glm::vec4(glm::mat3(m_transform) * ofs, 0);
                m_lerp_skip = true;
            }
            break;

        case TR_ANIMCOMMAND_SETVELOCITY:    // (float vertical, float horizontal)
            {
                glm::float_t vert;
                const glm::float_t horiz = glm::float_t(command.param[1]);
                if(btFuzzyZero(m_vspeed_override))
                {
                    vert  = -glm::float_t(command.param[0]);
                }
                else
                {
                    vert  = m_vspeed_override;
                    m_vspeed_override = 0.0f;
                }
                jump(vert, horiz);
            }
            break;

        case TR_ANIMCOMMAND_EMPTYHANDS:     // ()
            // This command is used only for Lara.
            // Reset interaction-blocking
            break;

        case TR_ANIMCOMMAND_KILL:           // ()
            // This command is usually used only for non-Lara items,
            // although there seem to be Lara-anims with this cmd id (tr4 anim 415, shotgun overlay)
            // TODO: for switches, this command indicates the trigger-frame
            if(!isPlayer())
            {
                kill();
            }
            break;

        case TR_ANIMCOMMAND_PLAYSOUND:      // (sndParam)
            {
                int16_t sound_index = command.param[0] & 0x3FFF;

                // Quick workaround for TR3 quicksand.
                if((getSubstanceState() == Substance::QuicksandConsumed) ||
                   (getSubstanceState() == Substance::QuicksandShallow))
                {
                    sound_index = 18;
                }

                if(command.param[0] & TR_ANIMCOMMAND_CONDITION_WATER)
                {
                    if(getSubstanceState() == Substance::WaterShallow)
                        engine::engine_world.audioEngine.send(sound_index, audio::EmitterType::Entity, getId());
                }
                else if(command.param[0] & TR_ANIMCOMMAND_CONDITION_LAND)
                {
                    if(getSubstanceState() != Substance::WaterShallow)
                        engine::engine_world.audioEngine.send(sound_index, audio::EmitterType::Entity, getId());
                }
                else
                {
                    engine::engine_world.audioEngine.send(sound_index, audio::EmitterType::Entity, getId());
                }
            }
            break;

        case TR_ANIMCOMMAND_PLAYEFFECT:     // (flipeffectParam)
            {
                const uint16_t effect_id = command.param[0] & 0x3FFF;
                if(effect_id == 0)  // rollflip
                {
                    // FIXME: wrapping angles are bad for quat lerp:
                    m_angles[0] += 180.0;

                    if(m_moveType == MoveType::Underwater)
                    {
                        m_angles[1] = -m_angles[1];                         // for underwater case
                    }
                    if(m_moveDir == MoveDirection::Backward)
                    {
                        m_moveDir = MoveDirection::Forward;
                    }
                    else if(m_moveDir == MoveDirection::Forward)
                    {
                        m_moveDir = MoveDirection::Backward;
                    }
                    updateTransform();
                    m_lerp_skip = true;
                }
                else
                {
                    engine_lua.execEffect(effect_id, getId());
                }
            }
            break;

        default:
            // Unknown command
            break;
    }
}

void Entity::processSector()
{
    if(!m_currentSector) return;

    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    BOOST_ASSERT(m_currentSector != nullptr);
    RoomSector* lowest_sector = m_currentSector->getLowestSector();
    BOOST_ASSERT(lowest_sector != nullptr);

    processSectorImpl();

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if((m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR) || (m_typeFlags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.
        try
        {
            if (engine_lua["tlist_RunTrigger"].is<lua::Callable>())
                engine_lua["tlist_RunTrigger"].call(lowest_sector->trig_index, ((m_bf.animations.model->id == 0) ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC), getId());
        }
        catch (lua::RuntimeError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }
}

void Entity::setAnimation(int animation, int frame, int another_model)
{
    m_bf.animations.setAnimation(animation, frame, another_model);

    m_bt.no_fix_all = false;

    // some items (jeep) need this here...
    m_bf.updateCurrentBoneFrame();
//    updateRigidBody(false);
}

int Entity::getAnimDispatchCase(LaraState id)
{
    animation::AnimationFrame* anim = &m_bf.animations.model->animations[m_bf.animations.current_animation];

    for(const animation::StateChange& stc : anim->stateChanges)
    {
        if(stc.id != id)
            continue;

        for(size_t j = 0; j < stc.anim_dispatch.size(); j++)
        {
            const animation::AnimDispatch& disp = stc.anim_dispatch[j];
            if((disp.frame_high >= disp.frame_low) && (m_bf.animations.current_frame >= disp.frame_low) && (m_bf.animations.current_frame <= disp.frame_high))
            {
                return static_cast<int>(j);
            }
        }
    }

    return -1;
}


// ----------------------------------------
void Entity::slerpBones(glm::float_t lerp)
{
    for(animation::SSAnimation* ss_anim = m_bf.animations.next; ss_anim != nullptr; ss_anim = ss_anim->next)
    {
        // TODO: should we have independent timing for overlay animations?
        ss_anim->lerp = lerp;
        ss_anim->frame_time = m_bf.animations.frame_time;
    }
    m_bf.animations.lerp = lerp;
    m_bf.updateCurrentBoneFrame();
}

void Entity::lerpTransform(glm::float_t lerp)
{
    if(!m_lerp_valid)
        return;

    m_transform = glm::interpolate(m_lerp_last_transform, m_lerp_curr_transform, lerp);
}

void Entity::updateInterpolation(float time)
{
    if(!(m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        // Bone animation interp:
        glm::float_t lerp = m_bf.animations.lerp;
        slerpBones(lerp);
        lerp += time * animation::FrameRate;
        if( lerp > 1.0 )
        {
            lerp = 1.0;
        }
        m_bf.animations.lerp = lerp;

        // Entity transform interp:
        lerpTransform(m_lerp);
        m_lerp += time * animation::GameLogicFrameRate;
        if( m_lerp > 1.0 )
        {
            m_lerp = 1.0;
        }
    }
}

animation::AnimUpdate Entity::stepAnimation(float time)
{
    if((m_typeFlags & ENTITY_TYPE_DYNAMIC) || !m_active || !m_enabled ||
       (m_bf.animations.model == nullptr) || ((m_bf.animations.model->animations.size() == 1) && (m_bf.animations.model->animations.front().frames.size() == 1)))
    {
        return animation::AnimUpdate::None;
    }
    if(m_bf.animations.mode == animation::SSAnimationMode::Locked)
        return animation::AnimUpdate::NewFrame;  // penetration fix will be applyed in Character_Move... functions

    animation::AnimUpdate stepResult = m_bf.animations.stepAnimation(time, this);

//    setAnimation(m_bf.animations.current_animation, m_bf.animations.current_frame);

    m_bf.updateCurrentBoneFrame();
    fixPenetrations(nullptr);

    return stepResult;
}

/**
 * Entity framestep actions
 * @param time      frame time
 */
void Entity::frame(float time)
{
    if(!m_enabled )
    {
        return;
    }
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        // Ragdoll
        updateRigidBody(false); // bbox update, room update, m_transform from btBody...
        return;
    }

    fixPenetrations(nullptr);
    processSector();    // triggers
    engine_lua.loopEntity(getId());

    if(m_typeFlags & ENTITY_TYPE_COLLCHECK)
        checkCollisionCallbacks();

    stepAnimation(time);

    // TODO: check rigidbody update requirements.
    //       If m_transform changes, rigid body must be updated regardless of anim frame change...
    //if(animStepResult != ENTITY_ANIM_NONE)
    //{ }
    m_bf.updateCurrentBoneFrame();
    updateRigidBody(false);
}


/**
 * The function rebuild / renew entity's BV
 */
void Entity::rebuildBV()
{
    if(m_bf.animations.model)
    {
        /*
         * get current BB from animation
         */
        m_obb.rebuild(m_bf.boundingBox);
        m_obb.doTransform();
    }
}

void Entity::checkActivators()
{
    if (getRoom() == nullptr)
            return;

    glm::vec4 ppos = m_transform[3] + m_transform[1] * m_bf.boundingBox.max[1];
    auto containers = getRoom()->containers;
    for(Object* cont : containers)
    {
        Entity* e = dynamic_cast<Entity*>(cont);
        if(!e)
                continue;

        if (!e->m_enabled)
                continue;

        if(e->m_typeFlags & ENTITY_TYPE_INTERACTIVE)
        {
            //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
            if(e != this && core::testOverlap(*e, *this)) //(vec3_dist_sq(transform+12, pos) < r))
            {
                engine_lua.execEntity(ENTITY_CALLBACK_ACTIVATE, e->getId(), getId());
            }
        }
        else if(e->m_typeFlags & ENTITY_TYPE_PICKABLE)
        {
            glm::float_t r = e->m_activationRadius;
            r *= r;
            const glm::vec4& v = e->m_transform[3];
            if(    e != this
                && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r)
                && (v[2] + 32.0 > m_transform[3][2] + m_bf.boundingBox.min[2])
                && (v[2] - 32.0 < m_transform[3][2] + m_bf.boundingBox.max[2]))
            {
                engine_lua.execEntity(ENTITY_CALLBACK_ACTIVATE, e->getId(), getId());
            }
        }
    }
}

void Entity::moveForward(glm::float_t dist)
{
    m_transform[3] += m_transform[1] * dist;
}

void Entity::moveStrafe(glm::float_t dist)
{
    m_transform[3] += m_transform[0] * dist;
}

void Entity::moveVertical(glm::float_t dist)
{
    m_transform[3] += m_transform[2] * dist;
}

Entity::Entity(uint32_t id)
    : Object(id)
{
    m_obb.transform = &m_transform;
}

Entity::~Entity()
{
    m_bt.last_collisions.clear();

    if(!m_bt.bt_joints.empty())
    {
        deleteRagdoll();
    }

    for(std::unique_ptr<btPairCachingGhostObject>& ghost : m_bt.ghostObjects)
    {
        ghost->setUserPointer(nullptr);
        engine::bt_engine_dynamicsWorld->removeCollisionObject(ghost.get());
    }
    m_bt.ghostObjects.clear();

    m_bt.shapes.clear();

    m_bt.manifoldArray.reset();

    if(!m_bt.bt_body.empty())
    {
        for(const auto& body : m_bt.bt_body)
        {
            if(body)
            {
                body->setUserPointer(nullptr);
                if(body->getMotionState())
                {
                    delete body->getMotionState();
                    body->setMotionState(nullptr);
                }
                body->setCollisionShape(nullptr);

                engine::bt_engine_dynamicsWorld->removeRigidBody(body.get());
            }
        }
        m_bt.bt_body.clear();
    }

    m_bf.bone_tags.clear();

    for(animation::SSAnimation* ss_anim = m_bf.animations.next; ss_anim != nullptr;)
    {
        animation::SSAnimation* ss_anim_next = ss_anim->next;
        ss_anim->next = nullptr;
        delete ss_anim;
        ss_anim = ss_anim_next;
    }
    m_bf.animations.next = nullptr;
}

bool Entity::createRagdoll(RDSetup* setup)
{
    // No entity, setup or body count overflow - bypass function.

    if(!setup || setup->body_setup.size() > m_bf.bone_tags.size())
    {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(!m_bt.bt_joints.empty())
    {
        result = deleteRagdoll();
    }

    // Setup bodies.
    m_bt.bt_joints.clear();
    // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
    m_bf.updateCurrentBoneFrame();
    m_bt.no_fix_all = false;
    m_bt.no_fix_body_parts = 0x00000000;
    fixPenetrations(nullptr);

    for(size_t i = 0; i < setup->body_setup.size(); i++)
    {
        if(i >= m_bf.bone_tags.size() || !m_bt.bt_body[i])
        {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }

        btVector3 inertia(0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].mass;

        engine::bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());

        m_bt.bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);
        m_bt.bt_body[i]->setMassProps(mass, inertia);

        m_bt.bt_body[i]->updateInertiaTensor();
        m_bt.bt_body[i]->clearForces();

        m_bt.bt_body[i]->setLinearFactor(btVector3(1.0, 1.0, 1.0));
        m_bt.bt_body[i]->setAngularFactor(btVector3(1.0, 1.0, 1.0));

        m_bt.bt_body[i]->setDamping(setup->body_setup[i].damping[0], setup->body_setup[i].damping[1]);
        m_bt.bt_body[i]->setRestitution(setup->body_setup[i].restitution);
        m_bt.bt_body[i]->setFriction(setup->body_setup[i].friction);

        m_bt.bt_body[i]->setSleepingThresholds(RD_DEFAULT_SLEEPING_THRESHOLD, RD_DEFAULT_SLEEPING_THRESHOLD);

        if(!m_bf.bone_tags[i].parent)
        {
            glm::float_t r = m_bf.bone_tags[i].mesh_base->boundingBox.getInnerRadius();
            m_bt.bt_body[i]->setCcdMotionThreshold(0.8f * r);
            m_bt.bt_body[i]->setCcdSweptSphereRadius(r);
        }
    }

    updateRigidBody(true);
    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        engine::bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->activate();
        m_bt.bt_body[i]->setLinearVelocity(util::convert(m_speed));
        if(i < m_bt.ghostObjects.size() && m_bt.ghostObjects[i])
        {
            engine::bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i].get());
            engine::bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i].get(), COLLISION_NONE, COLLISION_NONE);
        }
    }

    // Setup constraints.
    m_bt.bt_joints.resize(setup->joint_setup.size());

    for(size_t i = 0; i < setup->joint_setup.size(); i++)
    {
        if(setup->joint_setup[i].body_index >= m_bf.bone_tags.size() || !m_bt.bt_body[setup->joint_setup[i].body_index])
        {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        animation::SSBoneTag* btB = &m_bf.bone_tags[setup->joint_setup[i].body_index];
        animation::SSBoneTag* btA = btB->parent;
        if(!btA)
        {
            result = false;
            break;
        }
        btTransform localA;
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        localA.setOrigin(util::convert(glm::vec3(btB->transform[3])));

        btTransform localB;
        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        localB.setOrigin({ 0, 0, 0 });

        switch(setup->joint_setup[i].joint_type)
        {
            case RDJointSetup::Point:
            {
                m_bt.bt_joints[i] = std::make_shared<btPoint2PointConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA.getOrigin(), localB.getOrigin());
            }
            break;

            case RDJointSetup::Hinge:
            {
                std::shared_ptr<btHingeConstraint> hingeC = std::make_shared<btHingeConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA, localB);
                hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9f, 0.3f, 0.3f);
                m_bt.bt_joints[i] = hingeC;
            }
            break;

            case RDJointSetup::Cone:
            {
                std::shared_ptr<btConeTwistConstraint> coneC = std::make_shared<btConeTwistConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA, localB);
                coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9f, 0.3f, 0.7f);
                m_bt.bt_joints[i] = coneC;
            }
            break;
        }

        m_bt.bt_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, -1);
        m_bt.bt_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, -1);

        m_bt.bt_joints[i]->setDbgDrawSize(64.0);
        engine::bt_engine_dynamicsWorld->addConstraint(m_bt.bt_joints[i].get(), true);
    }

    if(!result)
    {
        deleteRagdoll();  // PARANOID: Clean up the mess, if something went wrong.
    }
    else
    {
        m_typeFlags |= ENTITY_TYPE_DYNAMIC;
    }
    return result;
}

bool Entity::deleteRagdoll()
{
    if(m_bt.bt_joints.empty())
        return false;

    for(auto& joint : m_bt.bt_joints)
    {
        if(joint)
        {
            engine::bt_engine_dynamicsWorld->removeConstraint(joint.get());
            joint.reset();
        }
    }

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        engine::bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        engine::bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(i < m_bt.ghostObjects.size() && m_bt.ghostObjects[i])
        {
            engine::bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i].get());
            engine::bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i].get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    m_bt.bt_joints.clear();

    m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}

glm::vec3 Entity::applyGravity(float time)
{
    const glm::vec3 gravityAccelleration = util::convert(engine::bt_engine_dynamicsWorld->getGravity());
    const glm::vec3 gravitySpeed = gravityAccelleration * time;
    glm::vec3 move = (m_speed + gravitySpeed*0.5f) * time;
    m_speed += gravitySpeed;
    return move;
}

} // namespace world
