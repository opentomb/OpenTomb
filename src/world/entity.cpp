#include "entity.h"

#include "LuaState.h"

#include "character_controller.h"
#include "engine/bullet.h"
#include "engine/engine.h"
#include "ragdoll.h"
#include "script/script.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world.h"
#include "world/animation/animcommands.h"
#include "world/core/orientedboundingbox.h"
#include "world/room.h"
#include "world/skeletalmodel.h"

#include "core/basemesh.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <boost/log/trivial.hpp>

namespace world
{
void Entity::enable()
{
    if(!m_enabled)
    {
        m_skeleton.enableCollision();
        m_enabled = m_active = m_visible = true;
    }
}

void Entity::disable()
{
    if(m_enabled)
    {
        m_skeleton.disableCollision();
        m_active = m_enabled = m_visible = false;
    }
}

/**
 * It is from bullet_character_controller
 */
int Ghost_GetPenetrationFixVector(btPairCachingGhostObject& ghost, btManifoldArray& manifoldArray, glm::vec3& correction)
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    btBroadphasePairArray &pairArray = ghost.getOverlappingPairCache()->getOverlappingPairArray();

    btVector3 aabb_min, aabb_max;
    ghost.getCollisionShape()->getAabb(ghost.getWorldTransform(), aabb_min, aabb_max);
    engine::BulletEngine::instance->dynamicsWorld->getBroadphase()->setAabb(ghost.getBroadphaseHandle(), aabb_min, aabb_max, engine::BulletEngine::instance->dynamicsWorld->getDispatcher());
    engine::BulletEngine::instance->dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost.getOverlappingPairCache(), engine::BulletEngine::instance->dynamicsWorld->getDispatchInfo(), engine::BulletEngine::instance->dynamicsWorld->getDispatcher());

    correction = { 0,0,0 };
    int num_pairs = ghost.getOverlappingPairCache()->getNumOverlappingPairs();
    int ret = 0;
    for(int i = 0; i < num_pairs; i++)
    {
        manifoldArray.clear();
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
            collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
        }

        int manifolds_size = manifoldArray.size();
        for(int j = 0; j < manifolds_size; j++)
        {
            btPersistentManifold* manifold = manifoldArray[j];
            Object* obj0 = static_cast<Object*>(manifold->getBody0()->getUserPointer());
            Object* obj1 = static_cast<Object*>(manifold->getBody1()->getUserPointer());
            if(obj0->getCollisionType() == world::CollisionType::Ghost || obj1->getCollisionType() == world::CollisionType::Ghost)
            {
                continue;
            }
            for(int k = 0; k < manifold->getNumContacts(); k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                glm::float_t dist = pt.getDistance();

                if(dist < 0.0)
                {
                    glm::vec3 t = -util::convert(pt.m_normalWorldOnB) * dist;
                    correction += t;
                    ret++;
                }
            }
        }
    }

    return ret;
}

void Entity::ghostUpdate()
{
    if(!m_skeleton.hasGhosts())
        return;

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        for(const animation::Bone& bone : m_skeleton.getBones())
        {
            auto tr = m_transform * bone.full_transform;
            bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr));
            auto pos = tr * glm::vec4(bone.mesh->m_center, 1);
            bone.ghostObject->getWorldTransform().setOrigin(util::convert(glm::vec3(pos)));
        }
    }
    else
    {
        for(const animation::Bone& bone : m_skeleton.getBones())
        {
            auto tr = bone.bt_body->getWorldTransform();
            tr.setOrigin(tr * util::convert(bone.mesh->m_center));
            bone.ghostObject->getWorldTransform() = tr;
        }
    }
}

///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity::getPenetrationFixVector(glm::vec3& reaction, bool hasMove)
{
    reaction = { 0,0,0 };
    if(!m_skeleton.hasGhosts() || m_skeleton.getModel()->no_fix_all)
        return 0;

    auto orig_pos = m_transform[3];
    int ret = 0;
    for(size_t i = 0; i < m_skeleton.getModel()->collision_map.size(); i++)
    {
        size_t m = m_skeleton.getModel()->collision_map[i];
        const animation::Bone* btag = &m_skeleton.getBones()[m];

        if(btag->body_part & m_skeleton.getModel()->no_fix_body_parts)
        {
            continue;
        }

        // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
        glm::vec3 from;
        if(!btag->parent || (hasMove && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
        {
            BOOST_ASSERT(m_skeleton.getBones()[m].ghostObject);
            from = util::convert(m_skeleton.getBones()[m].ghostObject->getWorldTransform().getOrigin());
            from += glm::vec3(m_transform[3] - orig_pos);
        }
        else
        {
            glm::vec4 parent_from = btag->parent->full_transform * glm::vec4(btag->parent->mesh->m_center, 1);
            from = glm::vec3(m_transform * parent_from);
        }

        auto tr = m_transform * btag->full_transform;
        auto to = glm::vec3(tr * glm::vec4(btag->mesh->m_center, 1.0f));
        auto curr = from;
        auto move = to - from;
        auto move_len = move.length();
        if(i == 0 && move_len > 1024.0)                                 ///@FIXME: magick const 1024.0!
        {
            break;
        }
        int iter = static_cast<int>(4.0 * move_len / btag->mesh->m_radius + 1);     ///@FIXME (not a critical): magick const 4.0!
        move /= static_cast<glm::float_t>(iter);

        for(int j = 0; j <= iter; j++)
        {
            tr[3] = glm::vec4(curr, 1.0f);
            m_skeleton.getBones()[m].ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr));
            glm::vec3 tmp;
            if(Ghost_GetPenetrationFixVector(*m_skeleton.getBones()[m].ghostObject, m_skeleton.manifoldArray(), tmp))
            {
                m_transform[3] += glm::vec4(tmp, 0);
                curr += tmp;
                from += tmp;
                ret++;
            }
            curr += move;
        }
    }
    reaction = glm::vec3(m_transform[3] - orig_pos);
    m_transform[3] = orig_pos;

    return ret;
}

void Entity::fixPenetrations(const glm::vec3* move)
{
    if(!m_skeleton.hasGhosts())
        return;

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    if(m_skeleton.getModel()->no_fix_all)
    {
        ghostUpdate();
        return;
    }

    glm::vec3 reaction;
    getPenetrationFixVector(reaction, move != nullptr);
    m_transform[3] += glm::vec4(reaction, 0);

    ghostUpdate();
}

void Entity::transferToRoom(Room* room)
{
    if(getRoom() && !getRoom()->overlaps(room))
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
    if(!m_skeleton.hasGhosts())
        return;

    uint32_t curr_flag;
    m_skeleton.updateCurrentCollisions(*this, m_transform);
    while(btCollisionObject* cobj = m_skeleton.getRemoveCollisionBodyParts(0xFFFFFFFF, curr_flag))
    {
        // do callbacks here:
        Object* object = static_cast<Object*>(cobj->getUserPointer());

        if(Entity* activator = dynamic_cast<Entity*>(object))
        {
            if(activator->m_callbackFlags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                engine_lua.execEntity(ENTITY_CALLBACK_COLLISION, activator->getId(), getId());
                //ConsoleInfo::instance().printf("char_body_flag = 0x%X, collider_type = %d", curr_flag, type);
            }
        }
        else if((m_callbackFlags & ENTITY_CALLBACK_ROOMCOLLISION) && dynamic_cast<Room*>(object))
        {
            engine_lua.execEntity(ENTITY_CALLBACK_ROOMCOLLISION, getId(), object->getId());
        }
    }
}

bool Entity::wasCollisionBodyParts(uint32_t parts_flags) const
{
    if(!m_skeleton.hasGhosts())
        return false;

    for(const animation::Bone& bone : m_skeleton.getBones())
    {
        if((bone.body_part & parts_flags) && !bone.last_collisions.empty())
        {
            return true;
        }
    }

    return false;
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

    setRoom(new_room);
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
        m_skeleton.getBones()[0].bt_body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_transform));
        updateRoomPos();
        m_skeleton.updateTransform(m_transform);
        updateGhostRigidBody();
        m_skeleton.updateBoundingBox();
    }
    else
    {
        if(m_skeleton.getModel() == nullptr
           || (!force && m_skeleton.getModel()->animations.size() == 1 && m_skeleton.getModel()->animations.front().getFrameDuration() == 1))
        {
            return;
        }

        updateRoomPos();
        if(getCollisionType() != world::CollisionType::Static)
        {
            m_skeleton.updateRigidBody(m_transform);
        }
    }
    rebuildBoundingBox();
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
    glm::float_t t = m_currentSpeed * animation::AnimationFrameRate;
    glm::float_t vz = zeroVz ? 0.0f : m_speed[2];

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
        m_speed = { 0,0,0 };
    }

    m_speed[2] = vz;
}

void Entity::addOverrideAnim(ModelId model_id)
{
    SkeletalModel* sm = engine::Engine::instance.m_world.getModelByID(model_id);

    if(!sm || sm->meshes.size() != m_skeleton.getBoneCount())
        return;

    m_skeleton.setModel(sm);
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
        case animation::AnimCommandOpcode::SetPosition:    // (tr_x, tr_y, tr_z)
        {
            // x=x, y=z, z=-y
            const glm::float_t x = glm::float_t(command.param[0]);
            const glm::float_t y = glm::float_t(command.param[2]);
            const glm::float_t z = -glm::float_t(command.param[1]);
            glm::vec3 ofs(x, y, z);
            m_transform[3] += glm::vec4(glm::mat3(m_transform) * ofs, 0);
#if 0
            m_lerp_skip = true;
#endif
        }
        break;

        case animation::AnimCommandOpcode::SetVelocity:    // (float vertical, float horizontal)
        {
            glm::float_t vert;
            const glm::float_t horiz = glm::float_t(command.param[1]);
            if(btFuzzyZero(m_vspeed_override))
            {
                vert = -glm::float_t(command.param[0]);
            }
            else
            {
                vert = m_vspeed_override;
                m_vspeed_override = 0.0f;
            }
            jump(vert, horiz);
        }
        break;

        case animation::AnimCommandOpcode::EmptyHands:
            // This command is used only for Lara.
            // Reset interaction-blocking
            break;

        case animation::AnimCommandOpcode::Kill:
            // This command is usually used only for non-Lara items,
            // although there seem to be Lara-anims with this cmd id (tr4 anim 415, shotgun overlay)
            // TODO: for switches, this command indicates the trigger-frame
            if(!isPlayer())
            {
                kill();
            }
            break;

        case animation::AnimCommandOpcode::PlaySound:      // (sndParam)
        {
            audio::SoundId soundId = command.param[0] & 0x3FFF;

            // Quick workaround for TR3 quicksand.
            if(getSubstanceState() == Substance::QuicksandConsumed || getSubstanceState() == Substance::QuicksandShallow)
            {
                soundId = audio::SoundWadeShallow;
            }

            if(command.param[0] & animation::TR_ANIMCOMMAND_CONDITION_WATER)
            {
                if(getSubstanceState() == Substance::WaterShallow)
                    engine::Engine::instance.m_world.audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
            else if(command.param[0] & animation::TR_ANIMCOMMAND_CONDITION_LAND)
            {
                if(getSubstanceState() != Substance::WaterShallow)
                    engine::Engine::instance.m_world.audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
            else
            {
                engine::Engine::instance.m_world.audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
        }
        break;

        case animation::AnimCommandOpcode::PlayEffect:     // (flipeffectParam)
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
#if 0
                m_lerp_skip = true;
#endif
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
    if(!m_currentSector)
        return;

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
            if(engine_lua["tlist_RunTrigger"].is<lua::Callable>())
                engine_lua["tlist_RunTrigger"].call(int(lowest_sector->trig_index), m_skeleton.getModel()->id == 0 ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC, getId());
        }
        catch(lua::RuntimeError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }
}

void Entity::setAnimation(animation::AnimationId animation, int frame)
{
    m_skeleton.setAnimation(animation, frame);

    m_skeleton.model()->no_fix_all = false;

    // some items (jeep) need this here...
    m_skeleton.updatePose();
    //    updateRigidBody(false);
}

boost::optional<size_t> Entity::getAnimDispatchCase(LaraState id) const
{
    const animation::Animation* anim = &m_skeleton.getModel()->animations[m_skeleton.getCurrentAnimation()];
    const animation::StateChange* stc = anim->findStateChangeByID(id);
    if(!stc)
        return boost::none;

    for(size_t j = 0; j < stc->dispatches.size(); j++)
    {
        const animation::AnimationDispatch& disp = stc->dispatches[j];

        if(disp.end >= disp.start
           && m_skeleton.getCurrentFrame() >= disp.start
           && m_skeleton.getCurrentFrame() <= disp.end)
        {
            return j;
        }
    }

    return boost::none;
}

void Entity::updateInterpolation()
{
    if((m_typeFlags & ENTITY_TYPE_DYNAMIC) != 0)
        return;

    // Bone animation interp:
    m_skeleton.updatePose();
}

animation::AnimUpdate Entity::stepAnimation(util::Duration time)
{
    if((m_typeFlags & ENTITY_TYPE_DYNAMIC)
       || !m_active
       || !m_enabled
       || m_skeleton.getModel() == nullptr
       || (m_skeleton.getModel()->animations.size() == 1 && m_skeleton.getModel()->animations.front().getFrameDuration() == 1))
    {
        return animation::AnimUpdate::None;
    }
    if(m_skeleton.getAnimationMode() == animation::AnimationMode::Locked)
        return animation::AnimUpdate::NewFrame;  // penetration fix will be applyed in Character_Move... functions

    animation::AnimUpdate stepResult = m_skeleton.stepAnimation(time, this);

    //    setAnimation(m_bf.animations.current_animation, m_bf.animations.current_frame);

    m_skeleton.updatePose();
    fixPenetrations(nullptr);

    return stepResult;
}

/**
 * Entity framestep actions
 * @param time      frame time
 */
void Entity::frame(util::Duration time)
{
    if(!m_enabled)
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
    m_skeleton.updatePose();
    updateRigidBody(false);
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity::rebuildBoundingBox()
{
    if(!m_skeleton.getModel())
        return;

    m_obb.rebuild(m_skeleton.getBoundingBox());
    m_obb.doTransform();
}

void Entity::checkActivators()
{
    if(getRoom() == nullptr)
        return;

    glm::vec4 ppos = m_transform[3] + m_transform[1] * m_skeleton.getBoundingBox().max[1];
    auto containers = getRoom()->m_objects;
    for(Object* object : containers)
    {
        Entity* e = dynamic_cast<Entity*>(object);
        if(!e)
            continue;

        if(!e->m_enabled)
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
            if(e != this
               && (v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r
               && v[2] + 32.0 > m_transform[3][2] + m_skeleton.getBoundingBox().min[2]
               && v[2] - 32.0 < m_transform[3][2] + m_skeleton.getBoundingBox().max[2])
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

Entity::Entity(ObjectId id)
    : Object(id)
{
    m_obb.transform = &m_transform;
}

Entity::~Entity()
{
    if(!m_skeleton.getModel()->bt_joints.empty())
    {
        deleteRagdoll();
    }
}

bool Entity::createRagdoll(RagdollSetup* setup)
{
    // No entity, setup or body count overflow - bypass function.

    if(!setup || setup->body_setup.size() > m_skeleton.getBoneCount())
    {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(!m_skeleton.getModel()->bt_joints.empty())
    {
        result = deleteRagdoll();
    }

    // Setup bodies.
    m_skeleton.model()->bt_joints.clear();
    // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
    m_skeleton.updatePose();
    m_skeleton.model()->no_fix_all = false;
    m_skeleton.model()->no_fix_body_parts = 0x00000000;
    fixPenetrations(nullptr);

    result &= m_skeleton.createRagdoll(*setup);

    updateRigidBody(true);

    m_skeleton.initCollisions(m_speed);

    // Setup constraints.
    m_skeleton.model()->bt_joints.resize(setup->joint_setup.size());

    for(size_t i = 0; i < setup->joint_setup.size(); i++)
    {
        if(setup->joint_setup[i].body_index >= m_skeleton.getBoneCount() || !m_skeleton.getBones()[setup->joint_setup[i].body_index].bt_body)
        {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        const animation::Bone* btB = &m_skeleton.getBones()[setup->joint_setup[i].body_index];
        const animation::Bone* btA = btB->parent;
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
            case RagdollJointSetup::Point:
            {
                m_skeleton.model()->bt_joints[i] = std::make_shared<btPoint2PointConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA.getOrigin(), localB.getOrigin());
            }
            break;

            case RagdollJointSetup::Hinge:
            {
                std::shared_ptr<btHingeConstraint> hingeC = std::make_shared<btHingeConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA, localB);
                hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9f, 0.3f, 0.3f);
                m_skeleton.model()->bt_joints[i] = hingeC;
            }
            break;

            case RagdollJointSetup::Cone:
            {
                std::shared_ptr<btConeTwistConstraint> coneC = std::make_shared<btConeTwistConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA, localB);
                coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9f, 0.3f, 0.7f);
                m_skeleton.model()->bt_joints[i] = coneC;
            }
            break;
        }

        m_skeleton.getModel()->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, -1);
        m_skeleton.getModel()->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, -1);

        m_skeleton.getModel()->bt_joints[i]->setDbgDrawSize(64.0);
        engine::BulletEngine::instance->dynamicsWorld->addConstraint(m_skeleton.getModel()->bt_joints[i].get(), true);
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
    if(m_skeleton.getModel()->bt_joints.empty())
        return false;

    for(std::shared_ptr<btTypedConstraint> joint : m_skeleton.getModel()->bt_joints)
    {
        if(joint)
        {
            engine::BulletEngine::instance->dynamicsWorld->removeConstraint(joint.get());
            joint.reset();
        }
    }

    for(const animation::Bone& bone : m_skeleton.getBones())
    {
        engine::BulletEngine::instance->dynamicsWorld->removeRigidBody(bone.bt_body.get());
        bone.bt_body->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        engine::BulletEngine::instance->dynamicsWorld->addRigidBody(bone.bt_body.get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(bone.ghostObject)
        {
            engine::BulletEngine::instance->dynamicsWorld->removeCollisionObject(bone.ghostObject.get());
            engine::BulletEngine::instance->dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    m_skeleton.model()->bt_joints.clear();

    m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}

glm::vec3 Entity::applyGravity(util::Duration time)
{
    const glm::vec3 gravityAccelleration = util::convert(engine::BulletEngine::instance->dynamicsWorld->getGravity());
    const glm::vec3 gravitySpeed = gravityAccelleration * util::toSeconds(time);
    glm::vec3 move = (m_speed + gravitySpeed*0.5f) * util::toSeconds(time);
    m_speed += gravitySpeed;
    return move;
}
} // namespace world