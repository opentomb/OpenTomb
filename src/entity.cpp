#include "entity.h"

#include <cmath>
#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "LuaState.h"

#include "anim_state_control.h"
#include "character_controller.h"
#include "console.h"
#include "engine.h"
#include "helpers.h"
#include "mesh.h"
#include "obb.h"
#include "ragdoll.h"
#include "script.h"
#include "system.h"
#include "vmath.h"
#include "world.h"

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
        btVector3 box = COLLISION_GHOST_VOLUME_COEFFICIENT * (m_bf.bone_tags[i].mesh_base->m_bbMax - m_bf.bone_tags[i].mesh_base->m_bbMin);
        m_bt.shapes.emplace_back(new btBoxShape(box));
        m_bt.shapes.back()->setMargin(COLLISION_MARGIN_DEFAULT);
        m_bf.bone_tags[i].mesh_base->m_radius = btMin(btMin(box.x(), box.y()), box.z());

        m_bt.ghostObjects.emplace_back(new btPairCachingGhostObject());

        m_bt.ghostObjects.back()->setIgnoreCollisionCheck(m_bt.bt_body[i].get(), true);

        btTransform gltr = m_transform * m_bf.bone_tags[i].full_transform;
        gltr.setOrigin(gltr * m_bf.bone_tags[i].mesh_base->m_center);

        m_bt.ghostObjects.back()->setWorldTransform(gltr);
        m_bt.ghostObjects.back()->setCollisionFlags(m_bt.ghostObjects.back()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        m_bt.ghostObjects.back()->setUserPointer(m_self.get());
        m_bt.ghostObjects.back()->setCollisionShape(m_bt.shapes.back().get());
        bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects.back().get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);

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
                bt_engine_dynamicsWorld->addRigidBody(b.get());
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
                bt_engine_dynamicsWorld->removeRigidBody(b.get());
            }
        }
    }
}

void Entity::genRigidBody()
{
    if((m_bf.animations.model == nullptr) || (m_self->collision_type == COLLISION_TYPE_NONE))
        return;

    m_bt.bt_body.clear();

    for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        std::shared_ptr<BaseMesh> mesh = m_bf.animations.model->mesh_tree[i].mesh_base;
        btCollisionShape *cshape;
        switch(m_self->collision_shape)
        {
            case COLLISION_SHAPE_BOX:
                cshape = BT_CSfromBBox(mesh->m_bbMin, mesh->m_bbMax, true, true);
                break;

            case COLLISION_SHAPE_SPHERE:
                cshape = BT_CSfromSphere(mesh->m_radius);
                break;

            case COLLISION_SHAPE_TRIMESH_CONVEX:
                cshape = BT_CSfromMesh(mesh, true, true, false);
                break;

            case COLLISION_SHAPE_TRIMESH:
            default:
                cshape = BT_CSfromMesh(mesh, true, true, true);
                break;
        };

        m_bt.bt_body.emplace_back();

        if(cshape)
        {
            btVector3 localInertia(0, 0, 0);
            cshape->calculateLocalInertia(0.0, localInertia);

            btTransform startTransform = m_transform * m_bf.bone_tags[i].full_transform;
            btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
            m_bt.bt_body.back().reset(new btRigidBody(0.0, motionState, cshape, localInertia));

            switch(m_self->collision_type)
            {
                case COLLISION_TYPE_KINEMATIC:
                    m_bt.bt_body.back()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
                    break;

                case COLLISION_TYPE_ACTOR:
                    m_bt.bt_body.back()->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
                    break;

                case COLLISION_TYPE_GHOST:
                    m_bt.bt_body.back()->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
                    break;

                case COLLISION_TYPE_STATIC:
                    m_bt.bt_body.back()->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
                    break;

                default:
                    break;
            }

            bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
            m_bt.bt_body.back()->setUserPointer(m_self.get());
        }
    }
}

/**
 * It is from bullet_character_controller
 */
int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btVector3* correction)
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
    btVector3 aabb_min, aabb_max, t;

    ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
    bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
    bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

    correction->setZero();
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
            btScalar directionSign = manifold->getBody0() == ghost ? btScalar(-1.0) : btScalar(1.0);
            EngineContainer* cont0 = static_cast<EngineContainer*>(manifold->getBody0()->getUserPointer());
            EngineContainer* cont1 = static_cast<EngineContainer*>(manifold->getBody1()->getUserPointer());
            if((cont0->collision_type == COLLISION_TYPE_GHOST) || (cont1->collision_type == COLLISION_TYPE_GHOST))
            {
                continue;
            }
            for(int k = 0; k < manifold->getNumContacts(); k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                btScalar dist = pt.getDistance();

                if(dist < 0.0)
                {
                    t = pt.m_normalWorldOnB * dist * directionSign;
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

    assert(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_transform * m_bf.bone_tags[i].full_transform;
            auto v = m_bf.animations.model->mesh_tree[i].mesh_base->m_center;
            m_bt.ghostObjects[i]->getWorldTransform() = tr;
            auto pos = tr * v;
            m_bt.ghostObjects[i]->getWorldTransform().setOrigin(pos);
        }
    }
    else
    {
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            tr.setOrigin(tr * m_bf.bone_tags[i].mesh_base->m_center);
            m_bt.ghostObjects[i]->getWorldTransform() = tr;
        }
    }
}

void Entity::updateCurrentCollisions()
{
    if(m_bt.ghostObjects.empty())
        return;

    assert(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        const std::unique_ptr<btPairCachingGhostObject>& ghost = m_bt.ghostObjects[i];
        EntityCollisionNode& cn = m_bt.last_collisions[i];

        cn.obj.clear();
        auto tr = m_transform * m_bf.bone_tags[i].full_transform;
        auto v = m_bf.animations.model->mesh_tree[i].mesh_base->m_center;
        auto orig_tr = ghost->getWorldTransform();
        ghost->getWorldTransform() = tr;
        auto pos = tr * v;
        ghost->getWorldTransform().setOrigin(pos);

        btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
        btVector3 aabb_min, aabb_max;

        ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
        bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
        bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

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
                        if(m_self.get() == static_cast<EngineContainer*>(cn.obj.back()->getUserPointer()))
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
int Entity::getPenetrationFixVector(btVector3* reaction, bool hasMove)
{
    reaction->setZero();
    if(m_bt.ghostObjects.empty() || m_bt.no_fix_all)
        return 0;

    assert(m_bt.ghostObjects.size() == m_bf.bone_tags.size());

    auto orig_pos = m_transform.getOrigin();
    int ret = 0;
    for(size_t i = 0; i < m_bf.animations.model->collision_map.size(); i++)
    {
        uint16_t m = m_bf.animations.model->collision_map[i];
        SSBoneTag* btag = &m_bf.bone_tags[m];

        if(btag->body_part & m_bt.no_fix_body_parts)
        {
            continue;
        }

        // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
        btVector3 from;
        if((btag->parent == nullptr) || (hasMove && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
        {
            from = m_bt.ghostObjects[m]->getWorldTransform().getOrigin();
            from += m_transform.getOrigin() - orig_pos;
        }
        else
        {
            auto parent_from = btag->parent->full_transform * btag->parent->mesh_base->m_center;
            from = m_transform * parent_from;
        }

        auto tr = m_transform * btag->full_transform;
        auto to = tr * btag->mesh_base->m_center;
        auto curr = from;
        auto move = to - from;
        auto move_len = move.length();
        if((i == 0) && (move_len > 1024.0))                                 ///@FIXME: magick const 1024.0!
        {
            break;
        }
        int iter = static_cast<int>((4.0 * move_len / btag->mesh_base->m_radius) + 1);     ///@FIXME (not a critical): magick const 4.0!
        move /= (btScalar)iter;

        for(int j = 0; j <= iter; j++)
        {
            tr.setOrigin(curr);
            auto tr_current = tr;
            m_bt.ghostObjects[m]->setWorldTransform(tr_current);
            btVector3 tmp;
            if(Ghost_GetPenetrationFixVector(m_bt.ghostObjects[m].get(), m_bt.manifoldArray.get(), &tmp))
            {
                m_transform.getOrigin() += tmp;
                curr += tmp;
                from += tmp;
                ret++;
            }
            curr += move;
        }
    }
    *reaction = m_transform.getOrigin() - orig_pos;
    m_transform.setOrigin(orig_pos);

    return ret;
}

void Entity::fixPenetrations(const btVector3* move)
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

    btVector3 reaction;
    getPenetrationFixVector(&reaction, move != nullptr);
    m_transform.getOrigin() += reaction;

    ghostUpdate();
}

void Entity::transferToRoom(Room* room)
{
    if(m_self->room && !m_self->room->isOverlapped(room))
    {
        if(m_self->room)
            m_self->room->removeEntity(this);
        if(room)
            room->addEntity(this);
    }
}

std::shared_ptr<BtEngineClosestConvexResultCallback> Entity::callbackForCamera() const
{
    auto cb = std::make_shared<BtEngineClosestConvexResultCallback>(m_self);
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
        int type = -1;
        EngineContainer* cont = static_cast<EngineContainer*>(cobj->getUserPointer());
        if(cont != nullptr)
        {
            type = cont->object_type;
        }

        if(type == OBJECT_ENTITY)
        {
            Entity* activator = static_cast<Entity*>(cont->object);

            if(activator->m_callbackFlags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                engine_lua.execEntity(ENTITY_CALLBACK_COLLISION, activator->m_id, m_id);
                //ConsoleInfo::instance().printf("char_body_flag = 0x%X, collider_type = %d", curr_flag, type);
            }
        }
        else if((m_callbackFlags & ENTITY_CALLBACK_ROOMCOLLISION) &&
                (type == OBJECT_ROOM_BASE))
        {
            Room* activator = static_cast<Room*>(cont->object);
            engine_lua.execEntity(ENTITY_CALLBACK_ROOMCOLLISION, m_id, activator->id);
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
    btVector3 pos = getRoomPos();
    auto new_room = Room_FindPosCogerrence(pos, m_self->room);
    if(!new_room)
    {
        m_currentSector = nullptr;
        return;
    }

    auto new_sector = new_room->getSectorXYZ(pos);
    if(new_room != new_sector->owner_room.get())
    {
        new_room = new_sector->owner_room.get();
    }

    transferToRoom(new_room);

    m_self->room = new_room;
    m_lastSector = m_currentSector;

    if(m_currentSector != new_sector)
    {
        m_triggerLayout &= static_cast<uint8_t>((~ENTITY_TLAYOUT_SSTATUS)); // Reset sector status.
        m_currentSector = new_sector;
    }
}

void Entity::updateRigidBody(bool force)
{
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        //btVector3 pos = bt.bt_body[0]->getWorldTransform().getOrigin();
        //vec3_copy(transform+12, pos);
        m_transform = m_bt.bt_body[0]->getWorldTransform();
        updateRoomPos();
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            m_bf.bone_tags[i].full_transform = m_transform.inverse() * tr;
        }

        // that cycle is necessary only for skinning models;
        for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            if(m_bf.bone_tags[i].parent != nullptr)
            {
                m_bf.bone_tags[i].transform = m_bf.bone_tags[i].parent->full_transform.inverse() * m_bf.bone_tags[i].full_transform;
            }
            else
            {
                m_bf.bone_tags[i].transform = m_bf.bone_tags[i].full_transform;
            }
        }

        updateGhostRigidBody();

        if(m_bf.bone_tags.size() == 1)
        {
            m_bf.bb_min = m_bf.bone_tags[0].mesh_base->m_bbMin;
            m_bf.bb_max = m_bf.bone_tags[0].mesh_base->m_bbMax;
        }
        else
        {
            m_bf.bb_min = m_bf.bone_tags[0].mesh_base->m_bbMin;
            m_bf.bb_max = m_bf.bone_tags[0].mesh_base->m_bbMax;
            for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
            {
                auto& pos = m_bf.bone_tags[i].full_transform.getOrigin();
                auto& bb_min = m_bf.bone_tags[i].mesh_base->m_bbMin;
                auto& bb_max = m_bf.bone_tags[i].mesh_base->m_bbMax;
                btScalar r = bb_max[0] - bb_min[0];
                btScalar t = bb_max[1] - bb_min[1];
                r = (t > r) ? (t) : (r);
                t = bb_max[2] - bb_min[2];
                r = (t > r) ? (t) : (r);
                r *= 0.5;

                if(m_bf.bb_min[0] > pos[0] - r)
                {
                    m_bf.bb_min[0] = pos[0] - r;
                }
                if(m_bf.bb_min[1] > pos[1] - r)
                {
                    m_bf.bb_min[1] = pos[1] - r;
                }
                if(m_bf.bb_min[2] > pos[2] - r)
                {
                    m_bf.bb_min[2] = pos[2] - r;
                }

                if(m_bf.bb_max[0] < pos[0] + r)
                {
                    m_bf.bb_max[0] = pos[0] + r;
                }
                if(m_bf.bb_max[1] < pos[1] + r)
                {
                    m_bf.bb_max[1] = pos[1] + r;
                }
                if(m_bf.bb_max[2] < pos[2] + r)
                {
                    m_bf.bb_max[2] = pos[2] + r;
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
        if(m_self->collision_type != COLLISION_TYPE_STATIC)
        {
            for(uint16_t i = 0; i < m_bf.bone_tags.size(); i++)
            {
                if(m_bt.bt_body[i])
                {
                    m_bt.bt_body[i]->getWorldTransform() = m_transform * m_bf.bone_tags[i].full_transform;
                }
            }
        }
    }
    rebuildBV();
}

void Entity::updateTransform()
{
    m_angles[0] = WrapAngle(m_angles[0]);
    m_angles[1] = WrapAngle(m_angles[1]);
    m_angles[2] = WrapAngle(m_angles[2]);

    m_transform.getBasis().setEulerZYX(m_angles[1] * RadPerDeg, m_angles[2] * RadPerDeg, m_angles[0] * RadPerDeg);

    fixPenetrations(nullptr);
}

void Entity::updateCurrentSpeed(bool zeroVz)
{
    btScalar t = m_currentSpeed * m_speedMult;
    btScalar vz = (zeroVz) ? (0.0) : (m_speed[2]);

    if(m_dirFlag & ENT_MOVE_FORWARD)
    {
        m_speed = m_transform.getBasis().getColumn(1) * t;
    }
    else if(m_dirFlag & ENT_MOVE_BACKWARD)
    {
        m_speed = m_transform.getBasis().getColumn(1) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_LEFT)
    {
        m_speed = m_transform.getBasis().getColumn(0) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_RIGHT)
    {
        m_speed = m_transform.getBasis().getColumn(0) * t;
    }
    else
    {
        m_speed.setZero();
    }

    m_speed[2] = vz;
}

void Entity::addOverrideAnim(int model_id)
{
    SkeletalModel* sm = engine_world.getModelByID(model_id);

    if((sm != nullptr) && (sm->mesh_count == m_bf.bone_tags.size()))
    {
        SSAnimation* ss_anim = new SSAnimation();

        ss_anim->model = sm;
        ss_anim->onFrame = nullptr;
        ss_anim->next = m_bf.animations.next;
        m_bf.animations.next = ss_anim;

        ss_anim->frame_time = 0.0;
        ss_anim->next_state = 0;
        ss_anim->lerp = 0.0;
        ss_anim->current_animation = 0;
        ss_anim->current_frame = 0;
        ss_anim->next_animation = 0;
        ss_anim->next_frame = 0;
        ss_anim->period = 1.0f / TR_FRAME_RATE;
    }
}

void Entity::updateCurrentBoneFrame(SSBoneFrame *bf, const btTransform* etr)
{
    SSBoneTag* btag = bf->bone_tags.data();
    BoneTag* src_btag, *next_btag;
    SkeletalModel* model = bf->animations.model;
    BoneFrame* curr_bf, *next_bf;

    next_bf = &model->animations[bf->animations.next_animation].frames[bf->animations.next_frame];
    curr_bf = &model->animations[bf->animations.current_animation].frames[bf->animations.current_frame];

    btVector3 tr, cmd_tr;
    if(etr && (curr_bf->command & ANIM_CMD_MOVE))
    {
        tr = etr->getBasis() * curr_bf->move;
        cmd_tr = tr * bf->animations.lerp;
    }
    else
    {
        tr.setZero();
        cmd_tr.setZero();
    }

    bf->bb_max = curr_bf->bb_max.lerp(next_bf->bb_max, bf->animations.lerp) + cmd_tr;
    bf->bb_min = curr_bf->bb_min.lerp(next_bf->bb_min, bf->animations.lerp) + cmd_tr;
    bf->centre = curr_bf->centre.lerp(next_bf->centre, bf->animations.lerp) + cmd_tr;
    bf->pos = curr_bf->pos.lerp(next_bf->pos, bf->animations.lerp) + cmd_tr;

    next_btag = next_bf->bone_tags.data();
    src_btag = curr_bf->bone_tags.data();
    for(uint16_t k = 0; k < curr_bf->bone_tags.size(); k++, btag++, src_btag++, next_btag++)
    {
        btag->offset = src_btag->offset.lerp(next_btag->offset, bf->animations.lerp);
        btag->transform.getOrigin() = btag->offset;
        btag->transform.getOrigin()[3] = 1.0;
        if(k == 0)
        {
            btag->transform.getOrigin() += bf->pos;
            btag->qrotate = Quat_Slerp(src_btag->qrotate, next_btag->qrotate, bf->animations.lerp);
        }
        else
        {
            BoneTag* ov_src_btag = src_btag;
            BoneTag* ov_next_btag = next_btag;
            btScalar ov_lerp = bf->animations.lerp;
            for(SSAnimation* ov_anim = bf->animations.next; ov_anim != nullptr; ov_anim = ov_anim->next)
            {
                if((ov_anim->model != nullptr) && (ov_anim->model->mesh_tree[k].replace_anim != 0))
                {
                    BoneFrame* ov_curr_bf = &ov_anim->model->animations[ov_anim->current_animation].frames[ov_anim->current_frame];
                    BoneFrame* ov_next_bf = &ov_anim->model->animations[ov_anim->next_animation].frames[ov_anim->next_frame];
                    ov_src_btag = &ov_curr_bf->bone_tags[k];
                    ov_next_btag = &ov_next_bf->bone_tags[k];
                    ov_lerp = ov_anim->lerp;
                    break;
                }
            }
            btag->qrotate = Quat_Slerp(ov_src_btag->qrotate, ov_next_btag->qrotate, ov_lerp);
        }
        btag->transform.setRotation(btag->qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    btag = bf->bone_tags.data();
    btag->full_transform = btag->transform;
    btag++;
    for(uint16_t k = 1; k < curr_bf->bone_tags.size(); k++, btag++)
    {
        btag->full_transform = btag->parent->full_transform * btag->transform;
    }
}

btScalar Entity::findDistance(const Entity& other)
{
    return (m_transform.getOrigin() - other.m_transform.getOrigin()).length();
}

void Entity::doAnimCommands(struct SSAnimation *ss_anim, int /*changing*/)
{
    if(engine_world.anim_commands.empty() || (ss_anim->model == nullptr))
    {
        return;  // If no anim commands
    }

    AnimationFrame* af = &ss_anim->model->animations[ss_anim->current_animation];
    if(af->num_anim_commands > 0 && af->num_anim_commands <= 255)
    {
        assert(af->anim_command < engine_world.anim_commands.size());
        int16_t *pointer = &engine_world.anim_commands[af->anim_command];

        for(uint32_t i = 0; i < af->num_anim_commands; i++)
        {
            assert(pointer < &engine_world.anim_commands.back());
            const auto command = *pointer;
            ++pointer;
            switch(command)
            {
                case TR_ANIMCOMMAND_SETPOSITION:
                    // This command executes ONLY at the end of animation.
                    pointer += 3; // Parse through 3 operands.
                    break;

                case TR_ANIMCOMMAND_JUMPDISTANCE:
                    // This command executes ONLY at the end of animation.
                    pointer += 2; // Parse through 2 operands.
                    break;

                case TR_ANIMCOMMAND_EMPTYHANDS:
                    ///@FIXME: Behaviour is yet to be discovered.
                    break;

                case TR_ANIMCOMMAND_KILL:
                    // This command executes ONLY at the end of animation.
                    if(ss_anim->current_frame == static_cast<int>(af->frames.size()) - 1)
                    {
                        kill();
                    }

                    break;

                case TR_ANIMCOMMAND_PLAYSOUND:
                    if(ss_anim->current_frame == pointer[0])
                    {
                        int16_t sound_index = pointer[1] & 0x3FFF;

                        // Quick workaround for TR3 quicksand.
                        if((getSubstanceState() == Substance::QuicksandConsumed) ||
                           (getSubstanceState() == Substance::QuicksandShallow))
                        {
                            sound_index = 18;
                        }

                        if(pointer[1] & TR_ANIMCOMMAND_CONDITION_WATER)
                        {
                            if(getSubstanceState() == Substance::WaterShallow)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                        }
                        else if(pointer[1] & TR_ANIMCOMMAND_CONDITION_LAND)
                        {
                            if(getSubstanceState() != Substance::WaterShallow)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                        }
                        else
                        {
                            Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                        }
                    }
                    pointer += 2;
                    break;

                case TR_ANIMCOMMAND_PLAYEFFECT:
                    if(ss_anim->current_frame == pointer[0])
                    {
                        uint16_t effect_id = pointer[1] & 0x3FFF;
                        if(effect_id > 0)
                            engine_lua.execEffect(effect_id, m_id);
                    }
                    pointer += 2;
                    break;
            }
        }
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

    assert(m_currentSector != nullptr);
    RoomSector* lowest_sector = m_currentSector->getLowestSector();
    assert(lowest_sector != nullptr);

    processSectorImpl();

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if((m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR) || (m_typeFlags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.
		try
		{
			if (engine_lua["tlist_RunTrigger"].is<lua::Callable>())
				engine_lua["tlist_RunTrigger"].call(lowest_sector->trig_index, ((m_bf.animations.model->id == 0) ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC), m_id);
		}
		catch (lua::RuntimeError& error)
		{
			Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
		}
    }
}

void Entity::setAnimation(int animation, int frame, int another_model)
{
    if(!m_bf.animations.model || animation >= static_cast<int>(m_bf.animations.model->animations.size()))
    {
        return;
    }

    animation = (animation < 0) ? (0) : (animation);
    m_bt.no_fix_all = false;

    if(another_model >= 0)
    {
        SkeletalModel* model = engine_world.getModelByID(another_model);
        if(!model || animation >= static_cast<int>(model->animations.size()))
            return;
        m_bf.animations.model = model;
    }

    AnimationFrame* anim = &m_bf.animations.model->animations[animation];

    m_bf.animations.lerp = 0.0;
    frame %= anim->frames.size();
    frame = (frame >= 0) ? (frame) : (anim->frames.size() - 1 + frame);
    m_bf.animations.period = 1.0 / TR_FRAME_RATE;

    m_bf.animations.last_state = anim->state_id;
    m_bf.animations.next_state = anim->state_id;
    m_bf.animations.current_animation = animation;
    m_bf.animations.current_frame = frame;
    m_bf.animations.next_animation = animation;
    m_bf.animations.next_frame = frame;

    //long int t = (bf.animations.frame_time) / bf.animations.period;
    //btScalar dt = bf.animations.frame_time - (btScalar)t * bf.animations.period;
    m_bf.animations.frame_time = static_cast<btScalar>(frame) * m_bf.animations.period;// + dt;

    updateCurrentBoneFrame(&m_bf, &m_transform);
    updateRigidBody(false);
}

struct StateChange *Anim_FindStateChangeByAnim(struct AnimationFrame *anim, int state_change_anim)
{
    if(state_change_anim >= 0)
    {
        StateChange* ret = anim->state_change.data();
        for(uint16_t i = 0; i < anim->state_change.size(); i++, ret++)
        {
            for(uint16_t j = 0; j < ret->anim_dispatch.size(); j++)
            {
                if(ret->anim_dispatch[j].next_anim == state_change_anim)
                {
                    return ret;
                }
            }
        }
    }

    return nullptr;
}

struct StateChange *Anim_FindStateChangeByID(struct AnimationFrame *anim, uint32_t id)
{
    StateChange* ret = anim->state_change.data();
    for(uint16_t i = 0; i < anim->state_change.size(); i++, ret++)
    {
        if(ret->id == id)
        {
            return ret;
        }
    }

    return nullptr;
}

int Entity::getAnimDispatchCase(uint32_t id)
{
    AnimationFrame* anim = &m_bf.animations.model->animations[m_bf.animations.current_animation];
    StateChange* stc = anim->state_change.data();

    for(uint16_t i = 0; i < anim->state_change.size(); i++, stc++)
    {
        if(stc->id == id)
        {
            AnimDispatch* disp = stc->anim_dispatch.data();
            for(uint16_t j = 0; j < stc->anim_dispatch.size(); j++, disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (m_bf.animations.current_frame >= disp->frame_low) && (m_bf.animations.current_frame <= disp->frame_high))// ||
                    //(disp->frame_high <  disp->frame_low) && ((bf.current_frame >= disp->frame_low) || (bf.current_frame <= disp->frame_high)))
                {
                    return static_cast<int>(j);
                }
            }
        }
    }

    return -1;
}

/*
 * Next frame and next anim calculation function.
 */
void Entity::getNextFrame(SSBoneFrame *bf, btScalar time, struct StateChange *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags)
{
    AnimationFrame* curr_anim = &bf->animations.model->animations[bf->animations.current_animation];

    *frame = (bf->animations.frame_time + time) / bf->animations.period;
    *frame = (*frame >= 0.0) ? (*frame) : (0.0);                                    // paranoid checking
    *anim = bf->animations.current_animation;

    /*
     * Flag has a highest priority
     */
    if(anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(*frame >= static_cast<int>(curr_anim->frames.size() - 1))
        {
            *frame = curr_anim->frames.size() - 1;
            *anim = bf->animations.current_animation;                          // paranoid dublicate
        }
        return;
    }
    else if(anim_flags == ANIM_LOCK)
    {
        *frame = 0;
        *anim = bf->animations.current_animation;
        return;
    }

    /*
     * Check next anim if frame >= frames.size()
     */
    if(*frame >= static_cast<int>(curr_anim->frames.size()))
    {
        if(curr_anim->next_anim)
        {
            *frame = curr_anim->next_frame;
            *anim = curr_anim->next_anim->id;
            return;
        }

        *frame %= curr_anim->frames.size();
        *anim = bf->animations.current_animation;                             // paranoid dublicate
        return;
    }

    /*
     * State change check
     */
    if(stc != nullptr)
    {
        AnimDispatch* disp = stc->anim_dispatch.data();
        for(uint16_t i = 0; i < stc->anim_dispatch.size(); i++, disp++)
        {
            if((disp->frame_high >= disp->frame_low) && (*frame >= disp->frame_low) && (*frame <= disp->frame_high))
            {
                *anim = disp->next_anim;
                *frame = disp->next_frame;
                //*frame = (disp->next_frame + (*frame - disp->frame_low)) % bf->model->animations[disp->next_anim].frames.size();
                return;                                                         // anim was changed
            }
        }
    }
}

void Entity::doAnimMove(int16_t *anim, int16_t *frame)
{
    if(m_bf.animations.model != nullptr)
    {
        AnimationFrame* curr_af = &m_bf.animations.model->animations[m_bf.animations.current_animation];
        BoneFrame* curr_bf = &curr_af->frames[m_bf.animations.current_frame];

        if(curr_bf->command & ANIM_CMD_JUMP)
        {
            jump(-curr_bf->v_Vertical, curr_bf->v_Horizontal);
        }
        if(curr_bf->command & ANIM_CMD_CHANGE_DIRECTION)
        {
            m_angles[0] += 180.0;
            if(m_moveType == MoveType::Underwater)
            {
                m_angles[1] = -m_angles[1];                         // for underwater case
            }
            if(m_dirFlag == ENT_MOVE_BACKWARD)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(m_dirFlag == ENT_MOVE_FORWARD)
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
            }
            updateTransform();
            setAnimation(curr_af->next_anim->id, curr_af->next_frame);
            *anim = m_bf.animations.current_animation;
            *frame = m_bf.animations.current_frame;
        }
        if(curr_bf->command & ANIM_CMD_MOVE)
        {
            btVector3 tr = m_transform.getBasis() * curr_bf->move;
            m_transform.getOrigin() += tr;
        }
    }
}

/**
 * In original engine (+ some information from anim_commands) the anim_commands implement in beginning of frame
 */
 ///@TODO: rewrite as a cycle through all bf.animations list
int Entity::frame(btScalar time)
{
    int16_t frame, anim, ret = ENTITY_ANIM_NONE;
    long int t;
    btScalar dt;
    StateChange* stc;
    SSAnimation* ss_anim;

    if((m_typeFlags & ENTITY_TYPE_DYNAMIC) || !m_active || !m_enabled ||
       (m_bf.animations.model == nullptr) || ((m_bf.animations.model->animations.size() == 1) && (m_bf.animations.model->animations.front().frames.size() == 1)))
    {
        return ENTITY_ANIM_NONE;
    }

    if(m_bf.animations.anim_flags & ANIM_LOCK) return ENTITY_ANIM_NEWFRAME;  // penetration fix will be applyed in Character_Move... functions

    ss_anim = &m_bf.animations;

    ghostUpdate();

    m_bf.animations.lerp = 0.0;
    stc = Anim_FindStateChangeByID(&m_bf.animations.model->animations[m_bf.animations.current_animation], m_bf.animations.next_state);
    getNextFrame(&m_bf, time, stc, &frame, &anim, m_bf.animations.anim_flags);
    if(m_bf.animations.current_animation != anim)
    {
        m_bf.animations.last_animation = m_bf.animations.current_animation;

        ret = ENTITY_ANIM_NEWANIM;
        doAnimCommands(&m_bf.animations, ret);
        doAnimMove(&anim, &frame);

        setAnimation(anim, frame);
        stc = Anim_FindStateChangeByID(&m_bf.animations.model->animations[m_bf.animations.current_animation], m_bf.animations.next_state);
    }
    else if(m_bf.animations.current_frame != frame)
    {
        if(m_bf.animations.current_frame == 0)
        {
            m_bf.animations.last_animation = m_bf.animations.current_animation;
        }

        ret = ENTITY_ANIM_NEWFRAME;
        doAnimCommands(&m_bf.animations, ret);
        doAnimMove(&anim, &frame);
    }

    // AnimationFrame* af = &m_bf.animations.model->animations[ m_bf.animations.current_animation ];
    m_bf.animations.frame_time += time;

    t = (m_bf.animations.frame_time) / m_bf.animations.period;
    dt = m_bf.animations.frame_time - static_cast<btScalar>(t) * m_bf.animations.period;
    m_bf.animations.frame_time = static_cast<btScalar>(frame) * m_bf.animations.period + dt;
    m_bf.animations.lerp = dt / m_bf.animations.period;
    getNextFrame(&m_bf, m_bf.animations.period, stc, &m_bf.animations.next_frame, &m_bf.animations.next_animation, ss_anim->anim_flags);

    frameImpl(time, frame, ret);

    updateCurrentBoneFrame(&m_bf, &m_transform);
    fixPenetrations(nullptr);

    return ret;
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
        m_obb->rebuild(m_bf.bb_min, m_bf.bb_max);
        m_obb->doTransform();
    }
}

void Entity::checkActivators()
{
	if (m_self->room == nullptr)
		return;

    btVector3 ppos = m_transform.getOrigin() + m_transform.getBasis().getColumn(1) * m_bf.bb_max[1];
	auto containers = m_self->room->containers;
    for(const std::shared_ptr<EngineContainer>& cont : containers)
    {
		if (cont->object_type != OBJECT_ENTITY || !cont->object)
			continue;

        Entity* e = static_cast<Entity*>(cont->object);
		if (!e->m_enabled)
			continue;

        if(e->m_typeFlags & ENTITY_TYPE_INTERACTIVE)
        {
            //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
            if((e != this) && (OBB_OBB_Test(*e, *this) == 1))//(vec3_dist_sq(transform+12, pos) < r))
            {
                engine_lua.execEntity(ENTITY_CALLBACK_ACTIVATE, e->m_id, m_id);
            }
        }
        else if(e->m_typeFlags & ENTITY_TYPE_PICKABLE)
        {
			btScalar r = e->m_activationRadius;
			r *= r;
			const btVector3& v = e->m_transform.getOrigin();
            if(    (e != this)
				&& ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r)
				&& (v[2] + 32.0 > m_transform.getOrigin()[2] + m_bf.bb_min[2])
				&& (v[2] - 32.0 < m_transform.getOrigin()[2] + m_bf.bb_max[2]))
            {
                engine_lua.execEntity(ENTITY_CALLBACK_ACTIVATE, e->m_id, m_id);
            }
        }
    }
}

void Entity::moveForward(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis().getColumn(1) * dist;
}

void Entity::moveStrafe(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis().getColumn(0) * dist;
}

void Entity::moveVertical(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis().getColumn(2) * dist;
}

Entity::Entity(uint32_t id)
    : Object()
    , m_id(id)
    , m_moveType(MoveType::OnFloor)
    , m_obb(new OBB())
    , m_self(new EngineContainer())
{
    m_transform.setIdentity();
    m_self->object = this;
    m_self->object_type = OBJECT_ENTITY;
    m_self->room = nullptr;
    m_self->collision_type = 0;
    m_obb->transform = &m_transform;
    m_bt.bt_body.clear();
    m_bt.bt_joints.clear();
    m_bt.no_fix_all = false;
    m_bt.no_fix_body_parts = 0x00000000;
    m_bt.manifoldArray = nullptr;
    m_bt.shapes.clear();
    m_bt.ghostObjects.clear();
    m_bt.last_collisions.clear();

    m_bf.animations.model = nullptr;
    m_bf.animations.onFrame = nullptr;
    m_bf.animations.frame_time = 0.0;
    m_bf.animations.last_state = 0;
    m_bf.animations.next_state = 0;
    m_bf.animations.lerp = 0.0;
    m_bf.animations.current_animation = 0;
    m_bf.animations.current_frame = 0;
    m_bf.animations.next_animation = 0;
    m_bf.animations.next_frame = 0;
    m_bf.animations.next = nullptr;
    m_bf.bone_tags.clear();
    m_bf.bb_max.setZero();
    m_bf.bb_min.setZero();
    m_bf.centre.setZero();
    m_bf.pos.setZero();
    m_speed.setZero();
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
        bt_engine_dynamicsWorld->removeCollisionObject(ghost.get());
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

                bt_engine_dynamicsWorld->removeRigidBody(body.get());
            }
        }
        m_bt.bt_body.clear();
    }

    m_self.reset();

    m_bf.bone_tags.clear();

    for(SSAnimation* ss_anim = m_bf.animations.next; ss_anim != nullptr;)
    {
        SSAnimation* ss_anim_next = ss_anim->next;
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
    Entity::updateCurrentBoneFrame(&m_bf, &m_transform);
    m_bt.no_fix_all = false;
    m_bt.no_fix_body_parts = 0x00000000;
#if 0
    int map_size = m_bf.animations.model->collision_map.size();             // does not works, strange...
    m_bf.animations.model->collision_map.size() = m_bf.animations.model->mesh_count;
    fixPenetrations(nullptr);
    m_bf.animations.model->collision_map.size() = map_size;
#else
    fixPenetrations(nullptr);
#endif

    for(size_t i = 0; i < setup->body_setup.size(); i++)
    {
        if(i >= m_bf.bone_tags.size() || !m_bt.bt_body[i])
        {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }

        btVector3 inertia(0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].mass;

        bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());

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
            btScalar r = getInnerBBRadius(m_bf.bone_tags[i].mesh_base->m_bbMin, m_bf.bone_tags[i].mesh_base->m_bbMax);
            m_bt.bt_body[i]->setCcdMotionThreshold(0.8f * r);
            m_bt.bt_body[i]->setCcdSweptSphereRadius(r);
        }
    }

    updateRigidBody(true);
    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->activate();
        m_bt.bt_body[i]->setLinearVelocity(m_speed);
        if(i < m_bt.ghostObjects.size() && m_bt.ghostObjects[i])
        {
            bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i].get());
            bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i].get(), COLLISION_NONE, COLLISION_NONE);
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

        btTransform localA, localB;
        SSBoneTag* btB = &m_bf.bone_tags[setup->joint_setup[i].body_index];
        SSBoneTag* btA = btB->parent;
        if(!btA)
        {
            result = false;
            break;
        }
#if 0
        localA.setFromOpenGLMatrix(btB->transform);
        localB.setIdentity();
#else
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        //localA.setOrigin(setup->joint_setup[i].body1_offset);
        localA.setOrigin(btB->transform.getOrigin());

        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        //localB.setOrigin(setup->joint_setup[i].body2_offset);
        localB.setOrigin(btVector3(0.0, 0.0, 0.0));
#endif

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
                hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9, 0.3, 0.3);
                m_bt.bt_joints[i] = hingeC;
            }
            break;

            case RDJointSetup::Cone:
            {
                std::shared_ptr<btConeTwistConstraint> coneC = std::make_shared<btConeTwistConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA, localB);
                coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9, 0.3, 0.7);
                m_bt.bt_joints[i] = coneC;
            }
            break;
        }

        m_bt.bt_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, -1);
        m_bt.bt_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, -1);

        m_bt.bt_joints[i]->setDbgDrawSize(64.0);
        bt_engine_dynamicsWorld->addConstraint(m_bt.bt_joints[i].get(), true);
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
            bt_engine_dynamicsWorld->removeConstraint(joint.get());
            joint.reset();
        }
    }

    for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
    {
        bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(i < m_bt.ghostObjects.size() && m_bt.ghostObjects[i])
        {
            bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i].get());
            bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i].get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    m_bt.bt_joints.clear();

    m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}

btVector3 Entity::applyGravity(btScalar time)
{
    const btVector3 gravityAccelleration = bt_engine_dynamicsWorld->getGravity();
    const btVector3 gravitySpeed = gravityAccelleration * time;
    btVector3 move = (m_speed + gravitySpeed/2) * time;
    m_speed += gravitySpeed;
    return move;
}
