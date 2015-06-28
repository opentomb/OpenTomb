#include <cstdlib>
#include <cmath>

#include "vmath.h"
#include "mesh.h"
#include "entity.h"
#include "render.h"
#include "camera.h"
#include "world.h"
#include "engine.h"
#include "console.h"
#include "script.h"
#include "gui.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "obb.h"
#include "gameflow.h"
#include "string.h"
#include "ragdoll.h"
#include "hair.h"

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>


void Entity::createGhosts()
{
    if(m_bf.animations.model->mesh_count <= 0)
        return;

    m_bt.manifoldArray = new btManifoldArray();
    m_bt.shapes = new btCollisionShape*[m_bf.bone_tags.size()];
    m_bt.ghostObjects = new btPairCachingGhostObject*[m_bf.bone_tags.size()];
    m_bt.last_collisions = new EntityCollisionNode[m_bf.bone_tags.size()];
    for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
    {
        btVector3 box;
        box[0] = 0.40 * (m_bf.bone_tags[i].mesh_base->m_bbMax[0] - m_bf.bone_tags[i].mesh_base->m_bbMin[0]);
        box[1] = 0.40 * (m_bf.bone_tags[i].mesh_base->m_bbMax[1] - m_bf.bone_tags[i].mesh_base->m_bbMin[1]);
        box[2] = 0.40 * (m_bf.bone_tags[i].mesh_base->m_bbMax[2] - m_bf.bone_tags[i].mesh_base->m_bbMin[2]);
        m_bt.shapes[i] = new btBoxShape(box);
        m_bf.bone_tags[i].mesh_base->m_radius = (box[0] < box[1])?(box[0]):(box[1]);
        m_bf.bone_tags[i].mesh_base->m_radius = (m_bf.bone_tags[i].mesh_base->m_radius < box[2])?(m_bf.bone_tags[i].mesh_base->m_radius):(box[2]);

        m_bt.ghostObjects[i] = new btPairCachingGhostObject();
        // FIXME bt.ghostObjects[i]->setIgnoreCollisionCheck(bt.bt_body[i], true);

        btTransform gltr = m_transform * m_bf.bone_tags[i].full_transform;
        gltr.setOrigin( gltr * m_bf.bone_tags[i].mesh_base->m_center );

        m_bt.ghostObjects[i]->setWorldTransform(gltr);
        m_bt.ghostObjects[i]->setCollisionFlags(m_bt.ghostObjects[i]->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
        m_bt.ghostObjects[i]->setUserPointer(m_self.get());
        m_bt.ghostObjects[i]->setCollisionShape(m_bt.shapes[i]);
        bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i], COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);

        m_bt.last_collisions[i].obj_count = 0;
    }
}


void Entity::enable()
{
    if(!(m_stateFlags & ENTITY_STATE_ENABLED)) {
        for(const auto& b : m_bt.bt_body) {
            if(b && !b->isInWorld()) {
                bt_engine_dynamicsWorld->addRigidBody(b.get());
            }
        }
        m_stateFlags |= ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    }
}


void Entity::disable()
{
    if(m_stateFlags & ENTITY_STATE_ENABLED) {
        for(const auto& b : m_bt.bt_body) {
            if(b && b->isInWorld()) {
                bt_engine_dynamicsWorld->removeRigidBody(b.get());
            }
        }
        m_stateFlags = 0x0000;
    }
}

/**
 * This function enables collision for entity_p in all cases exept NULL models.
 * If collision models does not exists, function will create them;
 * @param ent - pointer to the entity.
 */
void Entity::enableCollision()
{
    if(!m_bt.bt_body.empty()) {
        m_self->collide_flag = 0x01;
        for(const auto& b : m_bt.bt_body) {
            if(b && !b->isInWorld()) {
                bt_engine_dynamicsWorld->addRigidBody(b.get());
            }
        }
    }
    else {
        m_self->collide_flag = COLLISION_TRIMESH;                            ///@TODO: order collision shape and entity collision type flags! it is a different things!
        genEntityRigidBody();
    }
}


void Entity::disableCollision()
{
    if(!m_bt.bt_body.empty()) {
        m_self->collide_flag = 0x00;
        for(const auto& b : m_bt.bt_body) {
            if(b && b->isInWorld()) {
                bt_engine_dynamicsWorld->removeRigidBody(b.get());
            }
        }
    }
}


void Entity::genEntityRigidBody()
{
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;

    if(m_bf.animations.model == NULL)
    {
        return;
    }

    m_bt.bt_body.clear();

    for(uint16_t i=0; i<m_bf.bone_tags.size(); i++)
    {
        btCollisionShape *cshape = BT_CSfromMesh(m_bf.animations.model->mesh_tree[i].mesh_base, true, true, false);
        m_bt.bt_body.emplace_back();

        if(cshape)
        {
            cshape->calculateLocalInertia(0.0, localInertia);

            auto tr = m_transform * m_bf.bone_tags[i].full_transform;
            startTransform = tr;
            btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
            m_bt.bt_body.back().reset( new btRigidBody(0.0, motionState, cshape, localInertia) );
            //bt.bt_body[i]->setCollisionFlags(bt.bt_body[i]->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
            m_bt.bt_body.back()->setUserPointer(m_self.get());
            m_bt.bt_body.back()->setUserIndex(i);
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
    for(int i=0;i<num_pairs;i++)
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
        for(int j=0;j<manifolds_size;j++)
        {
            btPersistentManifold* manifold = (*manifoldArray)[j];
            btScalar directionSign = manifold->getBody0() == ghost ? btScalar(-1.0) : btScalar(1.0);
            for(int k=0;k<manifold->getNumContacts();k++)
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
    if(m_bt.ghostObjects != NULL)
    {
        if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
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
            for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
            {
                auto tr = m_bt.bt_body[i]->getWorldTransform();
                tr.setOrigin(tr * m_bf.bone_tags[i].mesh_base->m_center);
                m_bt.ghostObjects[i]->getWorldTransform() = tr;
            }
        }
    }
}


void Entity::updateCurrentCollisions()
{
    if(m_bt.ghostObjects != NULL)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            btPairCachingGhostObject *ghost = m_bt.ghostObjects[i];
            EntityCollisionNode* cn = m_bt.last_collisions + i;

            cn->obj_count = 0;
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
            for(int j=0;j<num_pairs;j++)
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

                for(int k=0;k<m_bt.manifoldArray->size();k++)
                {
                    if(cn->obj_count < MAX_OBJECTS_IN_COLLSION_NODE - 1)
                    {
                        btPersistentManifold* manifold = (*m_bt.manifoldArray)[k];
                        for(int c=0;c<manifold->getNumContacts();c++)               // c++ in C++
                        {
                            //const btManifoldPoint &pt = manifold->getContactPoint(c);
                            if(manifold->getContactPoint(c).getDistance() < 0.0)
                            {
                                cn->obj[cn->obj_count] = (btCollisionObject*)(*m_bt.manifoldArray)[k]->getBody0();
                                if(m_self.get() == (EngineContainer*)(cn->obj[cn->obj_count]->getUserPointer()))
                                {
                                    cn->obj[cn->obj_count] = (btCollisionObject*)(*m_bt.manifoldArray)[k]->getBody1();
                                }
                                cn->obj_count++;                                    // do it once in current cycle, so condition (cn->obj_count < MAX_OBJECTS_IN_COLLSION_NODE - 1) located in correct place
                                break;
                            }
                        }
                    }
                }
            }
            ghost->setWorldTransform(orig_tr);
        }
    }
}


///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity::getPenetrationFixVector(btVector3* reaction, bool hasMove)
{
    int ret = 0;

    reaction->setZero();
    if((m_bt.ghostObjects != NULL) && (m_bt.no_fix_all == 0))
    {
        auto orig_pos = m_transform.getOrigin();
        for(uint16_t i=0;i<m_bf.animations.model->collision_map.size();i++)
        {
            uint16_t m = m_bf.animations.model->collision_map[i];
            SSBoneTag* btag = &m_bf.bone_tags[m];

            if(btag->body_part & m_bt.no_fix_body_parts)
            {
                continue;
            }

            // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
            btVector3 from;
            if((btag->parent == NULL) || (hasMove && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
            {
                from = m_bt.ghostObjects[m]->getWorldTransform().getOrigin();
                from += m_transform.getOrigin() - orig_pos;
            }
            else
            {
                auto parent_from = btag->full_transform * btag->mesh_base->m_center;
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
            int iter = (btScalar)(4.0 * move_len / btag->mesh_base->m_radius) + 1;     ///@FIXME (not a critical): magick const 4.0!
            move[0] /= (btScalar)iter;
            move[1] /= (btScalar)iter;
            move[2] /= (btScalar)iter;

            for(int j=0;j<=iter;j++)
            {
                tr.setOrigin(curr);
                auto tr_current = tr;
                m_bt.ghostObjects[m]->setWorldTransform(tr_current);
                btVector3 tmp;
                if(Ghost_GetPenetrationFixVector(m_bt.ghostObjects[m], m_bt.manifoldArray, &tmp))
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
    }

    return ret;
}


void Entity::fixPenetrations(btVector3* move)
{
    if(m_bt.ghostObjects != NULL)
    {
        if((move != NULL) && (m_character != NULL))
        {
            m_character->m_response.horizontal_collide    = 0x00;
            m_character->m_response.vertical_collide      = 0x00;
        }

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
        int numPenetrationLoops = getPenetrationFixVector(&reaction, move!=nullptr);
        m_transform.getOrigin() += reaction;

        if(m_character != NULL)
        {
            Character_UpdateCurrentHeight(std::static_pointer_cast<Entity>(shared_from_this()));
            if((move != NULL) && (numPenetrationLoops > 0))
            {
                btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
                btScalar t2 = move->x() * move->x() + move->y() * move->y();
                if((reaction[2] * reaction[2] < t1) && (move->z() * move->z() < t2))    // we have horizontal move and horizontal correction
                {
                    t2 *= t1;
                    t1 = (reaction[0] * move->x() + reaction[1] * move->y()) / sqrtf(t2);
                    if(t1 < m_character->m_criticalWallComponent)
                    {
                        m_character->m_response.horizontal_collide |= 0x01;
                    }
                }
                else if((reaction[2] * reaction[2] > t1) && (move->z() * move->z() > t2))
                {
                    if((reaction[2] > 0.0) && (move->z() < 0.0))
                    {
                        m_character->m_response.vertical_collide |= 0x01;
                    }
                    else if((reaction[2] < 0.0) && (move->z() > 0.0))
                    {
                        m_character->m_response.vertical_collide |= 0x02;
                    }
                }
            }

            if(m_character->m_heightInfo.ceiling_hit && (reaction[2] < -0.1))
            {
                m_character->m_response.vertical_collide |= 0x02;
            }

            if(m_character->m_heightInfo.floor_hit && (reaction[2] > 0.1))
            {
                m_character->m_response.vertical_collide |= 0x01;
            }
        }

        ghostUpdate();
    }
}

/**
 * we check walls and other collision objects reaction. if reaction more then critacal
 * then cmd->horizontal_collide |= 0x01;
 * @param ent - cheked entity
 * @param cmd - here we fill cmd->horizontal_collide field
 * @param move - absolute 3d move vector
 */
int Entity::checkNextPenetration(const btVector3& move)
{
    int ret = 0;
    if(m_bt.ghostObjects != NULL)
    {
        CharacterResponse* resp = &m_character->m_response;

        ghostUpdate();
        m_transform.getOrigin() += move;
        //resp->horizontal_collide = 0x00;
        btVector3 reaction;
        ret = getPenetrationFixVector(&reaction, true);
        if((ret > 0) && (m_character != NULL))
        {
            btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
            btScalar t2 = move[0] * move[0] + move[1] * move[1];
            if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))
            {
                t2 *= t1;
                t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
                if(t1 < m_character->m_criticalWallComponent)
                {
                    resp->horizontal_collide |= 0x01;
                }
            }
        }
        m_transform.getOrigin() -= move;
        ghostUpdate();
        cleanCollisionAllBodyParts();
    }

    return ret;
}


void Entity::checkCollisionCallbacks()
{
    if(m_bt.ghostObjects != NULL)
    {
        btCollisionObject *cobj;
        uint32_t curr_flag;
        updateCurrentCollisions();
        while((cobj = getRemoveCollisionBodyParts(0xFFFFFFFF, &curr_flag)) != NULL)
        {
            // do callbacks here:
            int type = -1;
            EngineContainer* cont = (EngineContainer*)cobj->getUserPointer();
            if(cont != NULL)
            {
                type = cont->object_type;
            }

            if(type == OBJECT_ENTITY)
            {
                std::shared_ptr<Entity> activator = std::static_pointer_cast<Entity>(cont->object);
                
                if(activator->m_callbackFlags & ENTITY_CALLBACK_COLLISION)
                {
                    // Activator and entity IDs are swapped in case of collision callback.
                    lua_ExecEntity(engine_lua, ENTITY_CALLBACK_COLLISION, activator->m_id, m_id);
                    ConsoleInfo::instance().printf("char_body_flag = 0x%X, collider_bone_index = %d, collider_type = %d", curr_flag, cobj->getUserIndex(), type);
                }
            }
        }
    }
}


bool Entity::wasCollisionBodyParts(uint32_t parts_flags)
{
    if(m_bt.last_collisions != NULL)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            if((m_bf.bone_tags[i].body_part & parts_flags) && (m_bt.last_collisions[i].obj_count > 0))
            {
                return true;
            }
        }
    }

    return false;
}


void Entity::cleanCollisionAllBodyParts()
{
    if(m_bt.last_collisions != NULL)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            m_bt.last_collisions[i].obj_count = 0;
        }
    }
}


void Entity::cleanCollisionBodyParts(uint32_t parts_flags)
{
    if(m_bt.last_collisions != NULL)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            if(m_bf.bone_tags[i].body_part & parts_flags)
            {
                m_bt.last_collisions[i].obj_count = 0;
            }
        }
    }
}


btCollisionObject* Entity::getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t *curr_flag)
{
    *curr_flag = 0x00;
    if(m_bt.last_collisions != NULL)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            if(m_bf.bone_tags[i].body_part & parts_flags)
            {
                EntityCollisionNode* cn = m_bt.last_collisions + i;
                if(cn->obj_count > 0)
                {
                    *curr_flag = m_bf.bone_tags[i].body_part;
                    return cn->obj[--cn->obj_count];
                }
            }
        }
    }

    return NULL;
}


void Entity::updateRoomPos()
{
    RoomSector* new_sector;

    auto v = (m_bf.bb_min + m_bf.bb_max)/2;
    auto pos = m_transform * v;
    std::shared_ptr<Room> new_room = Room_FindPosCogerrence(pos, m_self->room);
    if(new_room)
    {
        new_sector = Room_GetSectorXYZ(new_room, pos);
        if(new_room != new_sector->owner_room)
        {
            new_room = new_sector->owner_room;
        }

        if(!m_character && (m_self->room != new_room))
        {
            if((m_self->room != NULL) && !Room_IsOverlapped(m_self->room, new_room))
            {
                if(m_self->room)
                {
                    Room_RemoveEntity(m_self->room, std::static_pointer_cast<Entity>(shared_from_this()));
                }
                if(new_room)
                {
                    Room_AddEntity(new_room, std::static_pointer_cast<Entity>(shared_from_this()));
                }
            }
        }

        m_self->room = new_room;
        m_lastSector = m_currentSector;

        if(m_currentSector != new_sector)
        {
            m_triggerLayout &= (uint8_t)(~ENTITY_TLAYOUT_SSTATUS); // Reset sector status.
            m_currentSector = new_sector;
        }
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
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            m_bf.bone_tags[i].full_transform = m_transform.inverse() * tr;
        }

        // that cycle is necessary only for skinning models;
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            if(m_bf.bone_tags[i].parent != NULL)
            {
                m_bf.bone_tags[i].transform = m_bf.bone_tags[i].full_transform.inverse() * m_bf.bone_tags[i].full_transform;
            }
            else
            {
                m_bf.bone_tags[i].transform = m_bf.bone_tags[i].full_transform;
            }
        }

        if(m_character && m_bt.ghostObjects)
        {
            for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
            {
                auto tr = m_bt.bt_body[i]->getWorldTransform();
                auto v = tr * m_bf.bone_tags[i].mesh_base->m_center;
                tr.setOrigin(v);
                m_bt.ghostObjects[i]->getWorldTransform() = tr;
            }
        }

        if(m_bf.bone_tags.size() == 1)
        {
            m_bf.bb_min = m_bf.bone_tags[0].mesh_base->m_bbMin;
            m_bf.bb_max = m_bf.bone_tags[0].mesh_base->m_bbMax;
        }
        else
        {
            m_bf.bb_min = m_bf.bone_tags[0].mesh_base->m_bbMin;
            m_bf.bb_max = m_bf.bone_tags[0].mesh_base->m_bbMax;
            for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
            {
                auto& pos = m_bf.bone_tags[i].full_transform.getOrigin();
                auto& bb_min = m_bf.bone_tags[i].mesh_base->m_bbMin;
                auto& bb_max = m_bf.bone_tags[i].mesh_base->m_bbMax;
                btScalar r = bb_max[0] - bb_min[0];
                btScalar t = bb_max[1] - bb_min[1];
                r = (t > r)?(t):(r);
                t = bb_max[2] - bb_min[2];
                r = (t > r)?(t):(r);
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
        if((m_bf.animations.model == NULL) ||
                m_bt.bt_body.empty() ||
                (!force && (m_bf.animations.model->animations.size() == 1) && (m_bf.animations.model->animations.front().frames.size() == 1)))
        {
            return;
        }

        updateRoomPos();
        if(m_self->collide_flag != 0x00)
        {
            for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
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


void Entity::updateRotation()
{
    auto& up_dir = m_transform.getBasis()[2];                                   // OZ
    auto& view_dir = m_transform.getBasis()[1];                                 // OY
    auto& right_dir = m_transform.getBasis()[0];                                // OX

    if(m_character != NULL)
        ghostUpdate();

    int i = m_angles[0] / 360.0;
    i = (m_angles[0] < 0.0)?(i-1):(i);
    m_angles[0] -= 360.0 * i;

    i = m_angles[1] / 360.0;
    i = (m_angles[1] < 0.0)?(i-1):(i);
    m_angles[1] -= 360.0 * i;

    i = m_angles[2] / 360.0;
    i = (m_angles[2] < 0.0)?(i-1):(i);
    m_angles[2] -= 360.0 * i;

    btScalar t = m_angles[0] * M_PI / 180.0;
    btScalar sin_t2 = sin(t);
    btScalar cos_t2 = cos(t);

    /*
     * LEFT - RIGHT INIT
     */

    view_dir[0] =-sin_t2;                                                       // OY - view
    view_dir[1] = cos_t2;
    view_dir[2] = 0.0;
    view_dir[3] = 0.0;

    right_dir[0] = cos_t2;                                                      // OX - right
    right_dir[1] = sin_t2;
    right_dir[2] = 0.0;
    right_dir[3] = 0.0;

    up_dir[0] = 0.0;                                                            // OZ - up
    up_dir[1] = 0.0;
    up_dir[2] = 1.0;
    up_dir[3] = 0.0;

    if(m_angles[1] != 0.0)
    {
        t = m_angles[1] * M_PI / 360.0;                                   // UP - DOWN
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        btVector3 R;
        R[3] = cos_t2;
        R[0] = right_dir[0] * sin_t2;
        R[1] = right_dir[1] * sin_t2;
        R[2] = right_dir[2] * sin_t2;
        auto Rt = -R;
        up_dir = R * up_dir * Rt;
        view_dir = R * view_dir * Rt;
    }

    if(m_angles[2] != 0.0)
    {
        t = m_angles[2] * M_PI / 360.0;                                   // ROLL
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        btVector3 R;
        R[3] = cos_t2;
        R[0] = view_dir[0] * sin_t2;
        R[1] = view_dir[1] * sin_t2;
        R[2] = view_dir[2] * sin_t2;
        auto Rt = -R;

        right_dir = R * right_dir * Rt;
        up_dir = R * up_dir * Rt;
    }

    view_dir[3] = 0.0;
    right_dir[3] = 0.0;
    up_dir[3] = 0.0;

    if(m_character != NULL)
        fixPenetrations(nullptr);
}


void Entity::updateCurrentSpeed(bool zeroVz)
{
    btScalar t  = m_currentSpeed * m_character->m_speedMult;
    btScalar vz = (zeroVz)?(0.0):(m_speed[2]);

    if(m_dirFlag & ENT_MOVE_FORWARD)
    {
        m_speed = m_transform.getBasis()[1] * t;
    }
    else if(m_dirFlag & ENT_MOVE_BACKWARD)
    {
        m_speed = m_transform.getBasis()[1] * -t;
    }
    else if(m_dirFlag & ENT_MOVE_LEFT)
    {
        m_speed = m_transform.getBasis()[0] * -t;
    }
    else if(m_dirFlag & ENT_MOVE_RIGHT)
    {
        m_speed = m_transform.getBasis()[0] * t;
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

    if((sm != NULL) && (sm->mesh_count == m_bf.bone_tags.size()))
    {
        SSAnimation* ss_anim = new SSAnimation();

        ss_anim->model = sm;
        ss_anim->onFrame = NULL;
        ss_anim->next = m_bf.animations.next;
        m_bf.animations.next = ss_anim;

        ss_anim->frame_time = 0.0;
        ss_anim->next_state = 0;
        ss_anim->lerp = 0.0;
        ss_anim->current_animation = 0;
        ss_anim->current_frame = 0;
        ss_anim->next_animation = 0;
        ss_anim->next_frame = 0;
        ss_anim->period = 1.0 / 30.0;;
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
        tr = *etr * curr_bf->move;
        cmd_tr = tr * bf->animations.lerp;
    }
    else
    {
        tr.setZero();
        cmd_tr.setZero();
    }

    bf->bb_max = curr_bf->bb_max.lerp(next_bf->bb_max, bf->animations.lerp) + cmd_tr;
    bf->bb_min = curr_bf->bb_min.lerp( next_bf->bb_min, bf->animations.lerp ) + cmd_tr;
    bf->centre = curr_bf->centre.lerp( next_bf->centre, bf->animations.lerp ) + cmd_tr;
    bf->pos = curr_bf->pos.lerp(next_bf->pos, bf->animations.lerp) + cmd_tr;

    next_btag = next_bf->bone_tags.data();
    src_btag = curr_bf->bone_tags.data();
    for(uint16_t k=0;k<curr_bf->bone_tags.size();k++,btag++,src_btag++,next_btag++)
    {
        btag->offset = src_btag->offset.lerp( next_btag->offset, bf->animations.lerp );
        btag->transform.getOrigin() = btag->offset;
        btag->transform.getOrigin()[3] = 1.0;
        if(k == 0)
        {
            btag->transform.getOrigin() += bf->pos;
            btag->qrotate = src_btag->qrotate.slerp( next_btag->qrotate, bf->animations.lerp );
        }
        else
        {
            BoneTag* ov_src_btag = src_btag;
            BoneTag* ov_next_btag = next_btag;
            btScalar ov_lerp = bf->animations.lerp;
            for(SSAnimation* ov_anim=bf->animations.next;ov_anim!=NULL;ov_anim = ov_anim->next)
            {
                if((ov_anim->model != NULL) && (ov_anim->model->mesh_tree[k].replace_anim != 0))
                {
                    BoneFrame* ov_curr_bf = &ov_anim->model->animations[ov_anim->current_animation].frames[ov_anim->current_frame];
                    BoneFrame* ov_next_bf = &ov_anim->model->animations[ov_anim->next_animation].frames[ov_anim->next_frame];
                    ov_src_btag = &ov_curr_bf->bone_tags[k];
                    ov_next_btag = &ov_next_bf->bone_tags[k];
                    ov_lerp = ov_anim->lerp;
                    break;
                }
            }
            btag->qrotate = ov_src_btag->qrotate.slerp(ov_next_btag->qrotate, ov_lerp);
        }
        btag->transform.setRotation(btag->qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    btag = bf->bone_tags.data();
    btag->full_transform = btag->transform;
    btag++;
    for(uint16_t k=1;k<curr_bf->bone_tags.size();k++,btag++)
    {
        btag->full_transform = btag->parent->full_transform * btag->transform;
    }
}


int Entity::getSubstanceState()
{
    if(!m_character)
    {
        return 0;
    }

    if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
    {
        if(m_character->m_heightInfo.transition_level > m_transform.getOrigin()[2] + m_character->m_height)
        {
            return ENTITY_SUBSTANCE_QUICKSAND_CONSUMED;
        }
        else
        {
            return ENTITY_SUBSTANCE_QUICKSAND_SHALLOW;
        }
    }
    else if(!m_character->m_heightInfo.water)
    {
        return ENTITY_SUBSTANCE_NONE;
    }
    else if( m_character->m_heightInfo.water &&
             (m_character->m_heightInfo.transition_level > m_transform.getOrigin()[2]) &&
             (m_character->m_heightInfo.transition_level < m_transform.getOrigin()[2] + m_character->m_wadeDepth) )
    {
        return ENTITY_SUBSTANCE_WATER_SHALLOW;
    }
    else if( m_character->m_heightInfo.water &&
             (m_character->m_heightInfo.transition_level > m_transform.getOrigin()[2] + m_character->m_wadeDepth) )
    {
        return ENTITY_SUBSTANCE_WATER_WADE;
    }
    else
    {
        return ENTITY_SUBSTANCE_WATER_SWIM;
    }
}

btScalar Entity::findDistance(const Entity& other)
{
    return (m_transform.getOrigin() - other.m_transform.getOrigin()).length();
}

void Entity::doAnimCommands(struct SSAnimation *ss_anim, int changing)
{
    if(engine_world.anim_commands.empty() || (ss_anim->model == NULL))
    {
        return;  // If no anim commands
    }

    AnimationFrame* af  = &ss_anim->model->animations[ss_anim->current_animation];
    if(af->num_anim_commands <= 255)
    {
        uint32_t count        = af->num_anim_commands;
        int16_t *pointer      = &engine_world.anim_commands[af->anim_command];
        int8_t   random_value = 0;

        for(uint32_t i = 0; i < count; i++, pointer++)
        {
            switch(*pointer)
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
                if(ss_anim->current_frame == af->frames.size() - 1)
                {
                    if(m_character)
                    {
                        m_character->m_response.kill = 1;
                    }
                }

                break;

            case TR_ANIMCOMMAND_PLAYSOUND:
                int16_t sound_index;

                if(ss_anim->current_frame == *++pointer)
                {
                    sound_index = *++pointer & 0x3FFF;

                    // Quick workaround for TR3 quicksand.
                    if((getSubstanceState() == ENTITY_SUBSTANCE_QUICKSAND_CONSUMED) ||
                            (getSubstanceState() == ENTITY_SUBSTANCE_QUICKSAND_SHALLOW)   )
                    {
                        sound_index = 18;
                    }

                    if(*pointer & TR_ANIMCOMMAND_CONDITION_WATER)
                    {
                        if(getSubstanceState() == ENTITY_SUBSTANCE_WATER_SHALLOW)
                            Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                    }
                    else if(*pointer & TR_ANIMCOMMAND_CONDITION_LAND)
                    {
                        if(getSubstanceState() != ENTITY_SUBSTANCE_WATER_SHALLOW)
                            Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                    }
                    else
                    {
                        Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, m_id);
                    }

                }
                else
                {
                    pointer++;
                }
                break;

            case TR_ANIMCOMMAND_PLAYEFFECT:
                // Effects (flipeffects) are various non-typical actions which vary
                // across different TR game engine versions. There are common ones,
                // however, and currently only these are supported.
                if(ss_anim->current_frame == *++pointer)
                {
                    switch(*++pointer & 0x3FFF)
                    {
                    case TR_EFFECT_SHAKESCREEN:
                        if(engine_world.Character)
                        {
                            btScalar dist = engine_world.Character->findDistance(*this);
                            dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):((TR_CAM_MAX_SHAKE_DISTANCE - dist) / 1024.0);
                            if(dist > 0)
                                renderer.camera()->shake(dist * TR_CAM_DEFAULT_SHAKE_POWER, 0.5);
                        }
                        break;

                    case TR_EFFECT_CHANGEDIRECTION:
                        break;

                    case TR_EFFECT_HIDEOBJECT:
                        m_stateFlags &= ~ENTITY_STATE_VISIBLE;
                        break;

                    case TR_EFFECT_SHOWOBJECT:
                        m_stateFlags |= ENTITY_STATE_VISIBLE;
                        break;

                    case TR_EFFECT_PLAYSTEPSOUND:
                        // Please note that we bypass land/water mask, as TR3-5 tends to ignore
                        // this flag and play step sound in any case on land, ignoring it
                        // completely in water rooms.
                        if(!getSubstanceState())
                        {
                            // TR3-5 footstep map.
                            // We define it here as a magic numbers array, because TR3-5 versions
                            // fortunately have no differences in footstep sounds order.
                            // Also note that some footstep types mutually share same sound IDs
                            // across different TR versions.
                            switch(m_currentSector->material)
                            {
                            case SECTOR_MATERIAL_MUD:
                                Audio_Send(288, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_SNOW:  // TR3 & TR5 only
                                if(engine_world.version != TR_IV)
                                {
                                    Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, m_id);
                                }
                                break;

                            case SECTOR_MATERIAL_SAND:  // Same as grass
                                Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_GRAVEL:
                                Audio_Send(290, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_ICE:   // TR3 & TR5 only
                                if(engine_world.version != TR_IV)
                                {
                                    Audio_Send(289, TR_AUDIO_EMITTER_ENTITY, m_id);
                                }
                                break;

                            case SECTOR_MATERIAL_WATER: // BYPASS!
                                // Audio_Send(17, TR_AUDIO_EMITTER_ENTITY, id);
                                break;

                            case SECTOR_MATERIAL_STONE: // DEFAULT SOUND, BYPASS!
                                // Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, id);
                                break;

                            case SECTOR_MATERIAL_WOOD:
                                Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_METAL:
                                Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_MARBLE:    // TR4 only
                                if(engine_world.version == TR_IV)
                                {
                                    Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, m_id);
                                }
                                break;

                            case SECTOR_MATERIAL_GRASS:     // Same as sand
                                Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_CONCRETE:  // DEFAULT SOUND, BYPASS!
                                Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_OLDWOOD:   // Same as wood
                                Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;

                            case SECTOR_MATERIAL_OLDMETAL:  // Same as metal
                                Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, m_id);
                                break;
                            }
                        }
                        break;

                    case TR_EFFECT_BUBBLE:
                        ///@FIXME: Spawn bubble particle here, when particle system is developed.
                        random_value = rand() % 100;
                        if(random_value > 60)
                        {
                            Audio_Send(TR_AUDIO_SOUND_BUBBLE, TR_AUDIO_EMITTER_ENTITY, m_id);
                        }
                        break;

                    default:
                        ///@FIXME: TODO ALL OTHER EFFECTS!
                        break;
                    }
                }
                else
                {
                    pointer++;
                }
                break;
            }
        }
    }
}


RoomSector* Entity::getLowestSector(RoomSector* sector)
{
    RoomSector* lowest_sector = sector;

    for(RoomSector* rs=sector;rs!=NULL;rs=rs->sector_below)
    { lowest_sector = rs; }

    return lowest_sector;
}


RoomSector* Entity::getHighestSector(RoomSector* sector)
{
    RoomSector* highest_sector = sector;

    for(RoomSector* rs=sector;rs!=NULL;rs=rs->sector_above)
    { highest_sector = rs; }

    return highest_sector;
}


void Entity::processSector()
{
    if(!m_currentSector) return;

    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    RoomSector* highest_sector = getHighestSector(m_currentSector);
    RoomSector* lowest_sector  = getLowestSector(m_currentSector);

    if(m_character)
    {
        m_character->m_heightInfo.walls_climb_dir  = 0;
        m_character->m_heightInfo.walls_climb_dir |= lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST  |
                                                                             SECTOR_FLAG_CLIMB_EAST  |
                                                                             SECTOR_FLAG_CLIMB_NORTH |
                                                                             SECTOR_FLAG_CLIMB_SOUTH );

        m_character->m_heightInfo.walls_climb     = (m_character->m_heightInfo.walls_climb_dir > 0);
        m_character->m_heightInfo.ceiling_climb   = 0x00;

        if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
        {
            m_character->m_heightInfo.ceiling_climb = 0x01;
        }

        if(lowest_sector->flags & SECTOR_FLAG_DEATH)
        {
            if((m_moveType == MOVE_ON_FLOOR)    ||
                    (m_moveType == MOVE_UNDERWATER) ||
                    (m_moveType == MOVE_WADE)        ||
                    (m_moveType == MOVE_ON_WATER)    ||
                    (m_moveType == MOVE_QUICKSAND))
            {
                Character_SetParam(std::static_pointer_cast<Entity>(shared_from_this()), PARAM_HEALTH, 0.0);
                m_character->m_response.kill = 1;
            }
        }
    }

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if((m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR) || (m_typeFlags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.

        int top = lua_gettop(engine_lua);
        lua_getglobal(engine_lua, "tlist_RunTrigger");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_pushnumber(engine_lua, lowest_sector->trig_index);
            lua_pushnumber(engine_lua, ((m_bf.animations.model->id == 0) ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC));
            lua_pushnumber(engine_lua, m_id);
            lua_CallAndLog(engine_lua, 3, 1, 0);
        }
        lua_settop(engine_lua, top);
    }
}


void Entity::setAnimation(int animation, int frame, int another_model)
{
    if(!m_bf.animations.model || (animation >= m_bf.animations.model->animations.size()))
    {
        return;
    }

    animation = (animation < 0)?(0):(animation);
    m_bt.no_fix_all = 0x00;

    if(another_model >= 0)
    {
        SkeletalModel* model = engine_world.getModelByID(another_model);
        if((!model) || (animation >= model->animations.size())) return;
        m_bf.animations.model = model;
    }

    AnimationFrame* anim = &m_bf.animations.model->animations[animation];

    m_bf.animations.lerp = 0.0;
    frame %= anim->frames.size();
    frame = (frame >= 0)?(frame):(anim->frames.size() - 1 + frame);
    m_bf.animations.period = 1.0 / 30.0;

    m_bf.animations.last_state = anim->state_id;
    m_bf.animations.next_state = anim->state_id;
    m_currentSpeed = anim->speed_x;
    m_bf.animations.current_animation = animation;
    m_bf.animations.current_frame = frame;
    m_bf.animations.next_animation = animation;
    m_bf.animations.next_frame = frame;

    m_bf.animations.frame_time = (btScalar)frame * m_bf.animations.period;
    //long int t = (bf.animations.frame_time) / bf.animations.period;
    //btScalar dt = bf.animations.frame_time - (btScalar)t * bf.animations.period;
    m_bf.animations.frame_time = (btScalar)frame * m_bf.animations.period;// + dt;

    updateCurrentBoneFrame(&m_bf, &m_transform);
    updateRigidBody(false);
}


struct StateChange *Anim_FindStateChangeByAnim(struct AnimationFrame *anim, int state_change_anim)
{
    if(state_change_anim >= 0)
    {
        StateChange* ret = anim->state_change.data();
        for(uint16_t i=0;i<anim->state_change.size();i++,ret++)
        {
            for(uint16_t j=0;j<ret->anim_dispatch.size();j++)
            {
                if(ret->anim_dispatch[j].next_anim == state_change_anim)
                {
                    return ret;
                }
            }
        }
    }

    return NULL;
}


struct StateChange *Anim_FindStateChangeByID(struct AnimationFrame *anim, uint32_t id)
{
    StateChange* ret = anim->state_change.data();
    for(uint16_t i=0;i<anim->state_change.size();i++,ret++)
    {
        if(ret->id == id)
        {
            return ret;
        }
    }

    return NULL;
}


int Entity::getAnimDispatchCase(uint32_t id)
{
    AnimationFrame* anim = &m_bf.animations.model->animations[ m_bf.animations.current_animation ];
    StateChange* stc = anim->state_change.data();

    for(uint16_t i=0;i<anim->state_change.size();i++,stc++)
    {
        if(stc->id == id)
        {
            AnimDispatch* disp = stc->anim_dispatch.data();
            for(uint16_t j=0;j<stc->anim_dispatch.size();j++,disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (m_bf.animations.current_frame >= disp->frame_low) && (m_bf.animations.current_frame <= disp->frame_high))// ||
                    //(disp->frame_high <  disp->frame_low) && ((bf.current_frame >= disp->frame_low) || (bf.current_frame <= disp->frame_high)))
                {
                    return (int)j;
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
    AnimationFrame* curr_anim = &bf->animations.model->animations[ bf->animations.current_animation ];

    *frame = (bf->animations.frame_time + time) / bf->animations.period;
    *frame = (*frame >= 0.0)?(*frame):(0.0);                                    // paranoid checking
    *anim  = bf->animations.current_animation;

    /*
     * Flag has a highest priority
     */
    if(anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(*frame >= curr_anim->frames.size() - 1)
        {
            *frame = curr_anim->frames.size() - 1;
            *anim  = bf->animations.current_animation;                          // paranoid dublicate
        }
        return;
    }
    else if(anim_flags == ANIM_LOCK)
    {
        *frame = 0;
        *anim  = bf->animations.current_animation;
        return;
    }

    /*
     * Check next anim if frame >= frames.size()
     */
    if(*frame >= curr_anim->frames.size())
    {
        if(curr_anim->next_anim)
        {
            *frame = curr_anim->next_frame;
            *anim  = curr_anim->next_anim->id;
            return;
        }

        *frame %= curr_anim->frames.size();
        *anim   = bf->animations.current_animation;                             // paranoid dublicate
        return;
    }

    /*
     * State change check
     */
    if(stc != NULL)
    {
        AnimDispatch* disp = stc->anim_dispatch.data();
        for(uint16_t i=0;i<stc->anim_dispatch.size();i++,disp++)
        {
            if((disp->frame_high >= disp->frame_low) && (*frame >= disp->frame_low) && (*frame <= disp->frame_high))
            {
                *anim  = disp->next_anim;
                *frame = disp->next_frame;
                //*frame = (disp->next_frame + (*frame - disp->frame_low)) % bf->model->animations[disp->next_anim].frames.size();
                return;                                                         // anim was changed
            }
        }
    }
}


void Entity::doAnimMove(int16_t *anim, int16_t *frame)
{
    if(m_bf.animations.model != NULL)
    {
        AnimationFrame* curr_af = &m_bf.animations.model->animations[ m_bf.animations.current_animation ];
        BoneFrame* curr_bf = &curr_af->frames[ m_bf.animations.current_frame ];

        if(curr_bf->command & ANIM_CMD_JUMP)
        {
            Character_SetToJump(std::static_pointer_cast<Entity>(shared_from_this()), -curr_bf->v_Vertical, curr_bf->v_Horizontal);
        }
        if(curr_bf->command & ANIM_CMD_CHANGE_DIRECTION)
        {
            m_angles[0] += 180.0;
            if(m_moveType == MOVE_UNDERWATER)
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
            updateRotation();
            setAnimation(curr_af->next_anim->id, curr_af->next_frame);
            *anim = m_bf.animations.current_animation;
            *frame = m_bf.animations.current_frame;
        }
        if(curr_bf->command & ANIM_CMD_MOVE)
        {
            btVector3 tr = m_transform.getOrigin() * curr_bf->move;
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
    int16_t frame, anim, ret = 0x00;
    long int t;
    btScalar dt;
    AnimationFrame* af;
    StateChange* stc;
    SSAnimation* ss_anim;

    if((m_typeFlags & ENTITY_TYPE_DYNAMIC) || !(m_stateFlags & ENTITY_STATE_ACTIVE)  || !(m_stateFlags & ENTITY_STATE_ENABLED) ||
            (m_bf.animations.model == NULL) || ((m_bf.animations.model->animations.size() == 1) && (m_bf.animations.model->animations.front().frames.size() == 1)))
    {
        return 0;
    }

    if(m_bf.animations.anim_flags & ANIM_LOCK) return 1;                  // penetration fix will be applyed in Character_Move... functions

    ss_anim = &m_bf.animations;

    ghostUpdate();

    m_bf.animations.lerp = 0.0;
    stc = Anim_FindStateChangeByID(&ss_anim->model->animations[ss_anim->current_animation], ss_anim->next_state);
    getNextFrame(&m_bf, time, stc, &frame, &anim, ss_anim->anim_flags);
    if(ss_anim->current_animation != anim)
    {
        ss_anim->last_animation = ss_anim->current_animation;

        ret = 0x02;
        doAnimCommands(&m_bf.animations, ret);
        doAnimMove(&anim, &frame);

        setAnimation(anim, frame);
        stc = Anim_FindStateChangeByID(&ss_anim->model->animations[ss_anim->current_animation], ss_anim->next_state);
    }
    else if(ss_anim->current_frame != frame)
    {
        if(ss_anim->current_frame == 0)
        {
            ss_anim->last_animation = ss_anim->current_animation;
        }

        ret = 0x01;
        doAnimCommands(&m_bf.animations, ret);
        doAnimMove(&anim, &frame);
    }

    af = &m_bf.animations.model->animations[ m_bf.animations.current_animation ];
    m_bf.animations.frame_time += time;

    t = (m_bf.animations.frame_time) / m_bf.animations.period;
    dt = m_bf.animations.frame_time - (btScalar)t * m_bf.animations.period;
    m_bf.animations.frame_time = (btScalar)frame * m_bf.animations.period + dt;
    m_bf.animations.lerp = dt / m_bf.animations.period;
    getNextFrame(&m_bf, m_bf.animations.period, stc, &m_bf.animations.next_frame, &m_bf.animations.next_animation, ss_anim->anim_flags);

    // Update acceleration.
    // With variable framerate, we don't know when we'll reach final
    // frame for sure, so we use native frame number check to increase acceleration.

    if((m_character) && (ss_anim->current_frame != frame))
    {

        // NB!!! For Lara, we update ONLY X-axis speed/accel.

        if((af->accel_x == 0) || (frame < m_bf.animations.current_frame))
        {
            m_currentSpeed  = af->speed_x;
        }
        else
        {
            m_currentSpeed += af->accel_x;
        }
    }

    m_bf.animations.current_frame = frame;


    doWeaponFrame(time);

    if(m_bf.animations.onFrame != NULL)
    {
        m_bf.animations.onFrame(std::static_pointer_cast<Entity>(shared_from_this()), &m_bf.animations, ret);
    }

    updateCurrentBoneFrame(&m_bf, &m_transform);
    if(m_character != NULL)
    {
        fixPenetrations(nullptr);
    }

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
    if(m_self->room != NULL)
    {
        btVector3 ppos = m_transform.getOrigin() + m_transform.getBasis()[1] * m_bf.bb_max[1];
        for(const std::shared_ptr<EngineContainer>& cont : m_self->room->containers)
        {
            if((cont->object_type == OBJECT_ENTITY) && (cont->object))
            {
                std::shared_ptr<Entity> e = std::static_pointer_cast<Entity>(cont->object);
                btScalar r = e->m_activationRadius;
                r *= r;
                if((e->m_typeFlags & ENTITY_TYPE_INTERACTIVE) && (e->m_stateFlags & ENTITY_STATE_ENABLED))
                {
                    //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
                    if((e.get() != this) && (OBB_OBB_Test(e, std::static_pointer_cast<Entity>(shared_from_this())) == 1))//(vec3_dist_sq(transform+12, pos) < r))
                    {
                        lua_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->m_id, m_id);
                    }
                }
                else if((e->m_typeFlags & ENTITY_TYPE_PICKABLE) && (e->m_stateFlags & ENTITY_STATE_ENABLED))
                {
                    const btVector3& v = e->m_transform.getOrigin();
                    if((e.get() != this) && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r) &&
                            (v[2] + 32.0 > m_transform.getOrigin()[2] + m_bf.bb_min[2]) && (v[2] - 32.0 < m_transform.getOrigin()[2] + m_bf.bb_max[2]))
                    {
                        lua_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->m_id, m_id);
                    }
                }
            }
        }
    }
}


void Entity::moveForward(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis()[1] * dist;
}


void Entity::moveStrafe(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis()[0] * dist;
}


void Entity::moveVertical(btScalar dist)
{
    m_transform.getOrigin() += m_transform.getBasis()[2] * dist;
}


/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Entity::doWeaponFrame(btScalar time)
{
    if(m_character != NULL)
    {
        /* anims (TR_I - TR_V):
         * pistols:
         * 0: idle to fire;
         * 1: draw weapon (short?);
         * 2: draw weapon (full);
         * 3: fire process;
         *
         * shotgun, rifles, crossbow, harpoon, launchers (2 handed weapons):
         * 0: idle to fire;
         * 1: draw weapon;
         * 2: fire process;
         * 3: hide weapon;
         * 4: idle to fire (targeted);
         */
        if((m_character->m_command.ready_weapon != 0x00) && (m_character->m_currentWeapon > 0) && (m_character->m_weaponCurrentState == WEAPON_STATE_HIDE))
        {
            Character_SetWeaponModel(std::static_pointer_cast<Entity>(shared_from_this()), m_character->m_currentWeapon, 1);
        }

        btScalar dt;
        int t;

        for(SSAnimation* ss_anim=m_bf.animations.next;ss_anim!=NULL;ss_anim=ss_anim->next)
        {
            if((ss_anim->model != NULL) && (ss_anim->model->animations.size() > 4))
            {
                switch(m_character->m_weaponCurrentState)
                {
                case WEAPON_STATE_HIDE:
                    if(m_character->m_command.ready_weapon)   // ready weapon
                    {
                        ss_anim->current_animation = 1;
                        ss_anim->next_animation = 1;
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_HIDE_TO_READY;
                    }
                    break;

                case WEAPON_STATE_HIDE_TO_READY:
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                    }
                    else
                    {
                        ss_anim->current_frame = 0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE:
                    ss_anim->current_frame = 0;
                    ss_anim->current_animation = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                    ss_anim->frame_time = 0.0;
                    if(m_character->m_command.ready_weapon)
                    {
                        ss_anim->current_animation = 3;
                        ss_anim->next_animation = 3;
                        ss_anim->current_frame = ss_anim->next_frame = 0;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE_TO_HIDE;
                    }
                    else if(m_character->m_command.action)
                    {
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE_TO_FIRE;
                    }
                    else
                    {
                        // do nothing here, may be;
                    }
                    break;

                case WEAPON_STATE_FIRE_TO_IDLE:
                    // Yes, same animation, reverse frames order;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                    if(ss_anim->current_frame > 0)
                    {
                        ss_anim->next_frame = ss_anim->current_frame - 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        ss_anim->next_frame = ss_anim->current_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE_TO_FIRE:
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = ss_anim->current_frame + 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 2;
                    }
                    else if(m_character->m_command.action)
                    {
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 1;
                        ss_anim->current_animation = 2;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE;
                    }
                    else
                    {
                        ss_anim->frame_time = 0.0;
                        ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE_TO_IDLE;
                    }
                    break;

                case WEAPON_STATE_FIRE:
                    if(m_character->m_command.action)
                    {
                        // inc time, loop;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = ss_anim->current_frame + 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->frame_time = dt;
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 1;
                        }
                    }
                    else
                    {
                        ss_anim->frame_time = 0.0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                        ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE_TO_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE_TO_HIDE:
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = ss_anim->current_frame + 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        ss_anim->next_frame = ss_anim->current_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_HIDE;
                        Character_SetWeaponModel(std::static_pointer_cast<Entity>(shared_from_this()), m_character->m_currentWeapon, 0);
                    }
                    break;
                };
            }
            else if((ss_anim->model != NULL) && (ss_anim->model->animations.size() == 4))
            {
                switch(m_character->m_weaponCurrentState)
                {
                case WEAPON_STATE_HIDE:
                    if(m_character->m_command.ready_weapon)   // ready weapon
                    {
                        ss_anim->current_animation = 2;
                        ss_anim->next_animation = 2;
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_HIDE_TO_READY;
                    }
                    break;

                case WEAPON_STATE_HIDE_TO_READY:
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                    }
                    else
                    {
                        ss_anim->current_frame = 0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE:
                    ss_anim->current_frame = 0;
                    ss_anim->current_animation = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                    ss_anim->frame_time = 0.0;
                    if(m_character->m_command.ready_weapon)
                    {
                        ss_anim->current_animation = 2;
                        ss_anim->next_animation = 2;
                        ss_anim->current_frame = ss_anim->next_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                        ss_anim->frame_time = 0.0;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE_TO_HIDE;
                    }
                    else if(m_character->m_command.action)
                    {
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE_TO_FIRE;
                    }
                    else
                    {
                        // do nothing here, may be;
                    }
                    break;

                case WEAPON_STATE_FIRE_TO_IDLE:
                    // Yes, same animation, reverse frames order;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                    if(ss_anim->current_frame > 0)
                    {
                        ss_anim->next_frame = ss_anim->current_frame - 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        ss_anim->next_frame = ss_anim->current_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE_TO_FIRE:
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = ss_anim->current_frame + 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 3;
                    }
                    else if(m_character->m_command.action)
                    {
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 1;
                        ss_anim->current_animation = 3;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE;
                    }
                    else
                    {
                        ss_anim->frame_time = 0.0;
                        ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE_TO_IDLE;
                    }
                    break;

                case WEAPON_STATE_FIRE:
                    if(m_character->m_command.action)
                    {
                        // inc time, loop;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = ss_anim->current_frame + 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->frame_time = dt;
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 1;
                        }
                    }
                    else
                    {
                        ss_anim->frame_time = 0.0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                        ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                        m_character->m_weaponCurrentState = WEAPON_STATE_FIRE_TO_IDLE;
                    }
                    break;

                case WEAPON_STATE_IDLE_TO_HIDE:
                    // Yes, same animation, reverse frames order;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                    if(ss_anim->current_frame > 0)
                    {
                        ss_anim->next_frame = ss_anim->current_frame - 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        ss_anim->next_frame = ss_anim->current_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                        m_character->m_weaponCurrentState = WEAPON_STATE_HIDE;
                        Character_SetWeaponModel(std::static_pointer_cast<Entity>(shared_from_this()), m_character->m_currentWeapon, 0);
                    }
                    break;
                };
            }

            doAnimCommands(ss_anim, 0);
        }
    }
}


Entity::Entity()
    : m_id(0)
    , m_moveType(MOVE_ON_FLOOR)
    , m_stateFlags( ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE )
    , m_typeFlags( ENTITY_TYPE_GENERIC )
    , m_callbackFlags( 0 ) // no callbacks by default
    , m_OCB( 0 )
    , m_triggerLayout( 0x00 )
    , m_timer( 0.0 )
    , m_self( new EngineContainer() )
    , m_obb( new OBB() )
    , m_character( NULL )
    , m_currentSector( NULL )
    , m_activationOffset{ 0.0, 256.0, 0.0 }
{
    m_transform.setIdentity();
    m_self->object = shared_from_this();
    m_self->object_type = OBJECT_ENTITY;
    m_self->room = NULL;
    m_self->collide_flag = 0;
    m_obb->transform = &m_transform;
    m_bt.bt_body.clear();
    m_bt.bt_joints.clear();
    m_bt.no_fix_all = 0x00;
    m_bt.no_fix_body_parts = 0x00000000;
    m_bt.manifoldArray = NULL;
    m_bt.shapes = NULL;
    m_bt.ghostObjects = NULL;
    m_bt.last_collisions = NULL;

    m_bf.animations.model = NULL;
    m_bf.animations.onFrame = NULL;
    m_bf.animations.frame_time = 0.0;
    m_bf.animations.last_state = 0;
    m_bf.animations.next_state = 0;
    m_bf.animations.lerp = 0.0;
    m_bf.animations.current_animation = 0;
    m_bf.animations.current_frame = 0;
    m_bf.animations.next_animation = 0;
    m_bf.animations.next_frame = 0;
    m_bf.animations.next = NULL;
    m_bf.bone_tags.clear();
    m_bf.bb_max.setZero();
    m_bf.bb_min.setZero();
    m_bf.centre.setZero();
    m_bf.pos.setZero();
    m_speed.setZero();
}

Entity::~Entity() {
    if((m_self->room != NULL) && (this != engine_world.Character.get()))
    {
        Room_RemoveEntity(m_self->room, std::static_pointer_cast<Entity>(shared_from_this()));
    }

    if(m_bt.last_collisions)
    {
        free(m_bt.last_collisions);
        m_bt.last_collisions = NULL;
    }

    if(!m_bt.bt_joints.empty())
    {
        deleteRagdoll();
    }

    if(m_bt.ghostObjects)
    {
        for(int i=0;i<m_bf.bone_tags.size();i++)
        {
            m_bt.ghostObjects[i]->setUserPointer(NULL);
            bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i]);
            delete m_bt.ghostObjects[i];
            m_bt.ghostObjects[i] = NULL;
        }
        free(m_bt.ghostObjects);
        m_bt.ghostObjects = NULL;
    }

    if(m_bt.shapes)
    {
        for(uint16_t i=0;i<m_bf.bone_tags.size();i++)
        {
            delete m_bt.shapes[i];
        }
        free(m_bt.shapes);
        m_bt.shapes = NULL;
    }

    if(m_bt.manifoldArray)
    {
        m_bt.manifoldArray->clear();
        delete m_bt.manifoldArray;
        m_bt.manifoldArray = NULL;
    }

    m_character.reset();

    if(!m_bt.bt_body.empty()) {
        for(const auto& body : m_bt.bt_body) {
            if(body) {
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

                bt_engine_dynamicsWorld->removeRigidBody(body.get());
            }
        }
        m_bt.bt_body.clear();
    }

    m_self.reset();

    m_bf.bone_tags.clear();

    for(SSAnimation* ss_anim=m_bf.animations.next;ss_anim!=NULL;)
    {
        SSAnimation* ss_anim_next = ss_anim->next;
        ss_anim->next = NULL;
        free(ss_anim);
        ss_anim = ss_anim_next;
    }
    m_bf.animations.next = NULL;
}

void Entity::updateHair()
{
    if((!IsCharacter(std::static_pointer_cast<Entity>(shared_from_this()))) || m_character->m_hairs.empty())
        return;

    for(std::shared_ptr<Hair> hair : m_character->m_hairs)
    {
        if(!hair || hair->m_elements.empty())
            continue;

        /*btScalar new_transform[16];

        new_transform = transform * bf.bone_tags[hair->owner_body].full_transform;

        // Calculate mixed velocities.
        btVector3 mix_vel(new_transform[12+0] - hair->owner_body_transform[12+0],
                          new_transform[12+1] - hair->owner_body_transform[12+1],
                          new_transform[12+2] - hair->owner_body_transform[12+2]);
        mix_vel *= 1.0 / engine_frame_time;

        if(0)
        {
            btScalar sub_tr[16];
            btTransform ang_tr;
            btVector3 mix_ang;
            sub_tr = hair->owner_body_transform.inverse() * new_transform;
            ang_tr.setFromOpenGLMatrix(sub_tr);
            ang_tr.getBasis().getEulerYPR(mix_ang.m_floats[2], mix_ang.m_floats[1], mix_ang.m_floats[0]);
            mix_ang *= 1.0 / engine_frame_time;

            // Looks like angular velocity breaks up constraints on VERY fast moves,
            // like mid-air turn. Probably, I've messed up with multiplier value...

            hair->elements[hair->root_index].body->setAngularVelocity(mix_ang);
            hair->owner_char->bt_body[hair->owner_body]->setAngularVelocity(mix_ang);
        }
        Mat4_Copy(hair->owner_body_transform, new_transform);*/

        // Set mixed velocities to both parent body and first hair body.

        //hair->elements[hair->root_index].body->setLinearVelocity(mix_vel);
        //hair->owner_char->bt_body[hair->owner_body]->setLinearVelocity(mix_vel);

        /*mix_vel *= -10.0;                                                     ///@FIXME: magick speed coefficient (force air hair friction!);
        for(int j=0;j<hair->element_count;j++)
        {
            hair->elements[j].body->applyCentralForce(mix_vel);
        }*/

        hair->m_container->room = hair->m_ownerChar->m_self->room;
    }
}

bool Entity::createRagdoll(RDSetup* setup)
{
    // No entity, setup or body count overflow - bypass function.

    if( !setup || setup->body_setup.size() > m_bf.bone_tags.size() ) {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(!m_bt.bt_joints.empty()) {
        result = deleteRagdoll();
    }

    // Setup bodies.
    m_bt.bt_joints.clear();
    // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
    Entity::updateCurrentBoneFrame(&m_bf, &m_transform);
    m_bt.no_fix_all = 0x00;
    m_bt.no_fix_body_parts = 0x00000000;
#if 0
    int map_size = m_bf.animations.model->collision_map.size();             // does not works, strange...
    m_bf.animations.model->collision_map.size() = m_bf.animations.model->mesh_count;
    fixPenetrations(nullptr);
    m_bf.animations.model->collision_map.size() = map_size;
#else
    fixPenetrations(nullptr);
#endif

    for(int i=0; i<setup->body_setup.size(); i++) {
        if( i >= m_bf.bone_tags.size() || !m_bt.bt_body[i] ) {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }

        btVector3 inertia (0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].mass;

        bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());

        m_bt.bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);
        m_bt.bt_body[i]->setMassProps(mass, inertia);

        m_bt.bt_body[i]->updateInertiaTensor();
        m_bt.bt_body[i]->clearForces();

        m_bt.bt_body[i]->setLinearFactor (btVector3(1.0, 1.0, 1.0));
        m_bt.bt_body[i]->setAngularFactor(btVector3(1.0, 1.0, 1.0));

        m_bt.bt_body[i]->setDamping(setup->body_setup[i].damping[0], setup->body_setup[i].damping[1]);
        m_bt.bt_body[i]->setRestitution(setup->body_setup[i].restitution);
        m_bt.bt_body[i]->setFriction(setup->body_setup[i].friction);
        m_bt.bt_body[i]->setSleepingThresholds(RD_DEFAULT_SLEEPING_THRESHOLD, RD_DEFAULT_SLEEPING_THRESHOLD);

        if(!m_bf.bone_tags[i].parent) {
            m_bf.bone_tags[i].mesh_base;
            btScalar r = getInnerBBRadius(m_bf.bone_tags[i].mesh_base->m_bbMin, m_bf.bone_tags[i].mesh_base->m_bbMax);
            m_bt.bt_body[i]->setCcdMotionThreshold(0.8 * r);
            m_bt.bt_body[i]->setCcdSweptSphereRadius(r);
        }
    }

    updateRigidBody(true);
    for(uint16_t i=0;i<m_bf.bone_tags.size();i++) {
        bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->activate();
        m_bt.bt_body[i]->setLinearVelocity(m_speed);
        if(m_bt.ghostObjects[i]) {
            bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i]);
            bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i], COLLISION_NONE, COLLISION_NONE);
        }
    }

    // Setup constraints.
    m_bt.bt_joints.resize(setup->joint_setup.size());

    for(int i=0; i<setup->joint_setup.size(); i++) {
        if( setup->joint_setup[i].body_index >= m_bf.bone_tags.size() || !m_bt.bt_body[setup->joint_setup[i].body_index] ) {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        btTransform localA, localB;
        SSBoneTag* btB = &m_bf.bone_tags[ setup->joint_setup[i].body_index ];
        SSBoneTag* btA = btB->parent;
        if(!btA) {
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

        switch(setup->joint_setup[i].joint_type) {
        case RDJointSetup::Point: {
            m_bt.bt_joints[i] = std::make_shared<btPoint2PointConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA.getOrigin(), localB.getOrigin());
        }
            break;

        case RDJointSetup::Hinge: {
            std::shared_ptr<btHingeConstraint> hingeC = std::make_shared<btHingeConstraint>(*m_bt.bt_body[btA->index], *m_bt.bt_body[btB->index], localA, localB);
            hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9, 0.3, 0.3);
            m_bt.bt_joints[i] = hingeC;
        }
            break;

        case RDJointSetup::Cone: {
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

    if(!result) {
        deleteRagdoll();  // PARANOID: Clean up the mess, if something went wrong.
    }
    else {
        m_typeFlags |=  ENTITY_TYPE_DYNAMIC;
    }
    return result;
}

bool Entity::deleteRagdoll()
{
    if(m_bt.bt_joints.empty())
        return false;

    for(auto& joint : m_bt.bt_joints) {
        if(joint) {
            bt_engine_dynamicsWorld->removeConstraint(joint.get());
            joint.reset();
        }
    }

    for(size_t i=0; i<m_bf.bone_tags.size(); i++)
    {
        bt_engine_dynamicsWorld->removeRigidBody(m_bt.bt_body[i].get());
        m_bt.bt_body[i]->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        bt_engine_dynamicsWorld->addRigidBody(m_bt.bt_body[i].get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(m_bt.ghostObjects[i])
        {
            bt_engine_dynamicsWorld->removeCollisionObject(m_bt.ghostObjects[i]);
            bt_engine_dynamicsWorld->addCollisionObject(m_bt.ghostObjects[i], COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    m_bt.bt_joints.clear();

    m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}
