#include "skeleton.h"

#include "pose.h"
#include "world/core/util.h"
#include "world/entity.h"
#include "world/ragdoll.h"
#include "world/skeletalmodel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>

namespace world
{
namespace animation
{
constexpr float GhostVolumeCollisionCoefficient = 0.4f;

void Skeleton::fromModel(const std::shared_ptr<SkeletalModel>& model)
{
    m_hasSkin = false;
    m_boundingBox.min = { 0, 0, 0 };
    m_boundingBox.max = { 0, 0, 0 };
    m_position = { 0, 0, 0 };

    m_model = model;

    m_bones.resize(model->getMeshReferenceCount(), Bone(this));

    std::stack<Bone*> parents;
    for(size_t i = 0; i < m_bones.size(); i++)
    {
        const auto& meshReference = model->getMeshReference(i);
        Bone& bone = m_bones[i];

        bone.index = i;
        bone.mesh = meshReference.mesh_base;
        bone.mesh_skin = meshReference.mesh_skin;
        if(bone.mesh_skin)
            m_hasSkin = true;
        bone.mesh_slot = nullptr;
        bone.body_part = meshReference.body_part;

        bone.position = meshReference.position;
        bone.localTransform = glm::mat4(1.0f);
        bone.globalTransform = glm::mat4(1.0f);

        if(i > 0)
            bone.parent = &m_bones[i - 1];

        switch(meshReference.stackOperation)
        {
            case SkeletalModel::MeshReference::UsePredecessor:
                if(i == 0)
                    throw std::runtime_error("Invalid skeleton stack operation: trying to use predecessor of first bone");
                break;
            case SkeletalModel::MeshReference::Push:
                parents.push(bone.parent);
                break;
            case SkeletalModel::MeshReference::Pop:
                if(parents.empty())
                    throw std::runtime_error("Invalid skeleton stack operation: cannot pop from empty stack");
                bone.parent = parents.top();
                parents.pop();
                break;
            case SkeletalModel::MeshReference::Top:
                if(parents.empty())
                    throw std::runtime_error("Invalid skeleton stack operation: cannot take top of empty stack");
                bone.parent = parents.top();
                break;
            default:
                throw std::runtime_error("Invalid skeleton stack operation");
        }

#ifndef NDEBUG
        BOOST_LOG_TRIVIAL(debug) << " - Bone #" << bone.index << " op " << (int)meshReference.stackOperation;
        if(bone.parent != nullptr)
            BOOST_LOG_TRIVIAL(debug) << "    parent #" << bone.parent->index;
#endif
    }
}

void Skeleton::itemFrame(util::Duration time)
{
    stepAnimation(time, nullptr);
    updatePose();
}

void Skeleton::updatePose()
{
    BOOST_ASSERT(m_currentTarget.animation < m_model->getAnimationCount());
    BOOST_ASSERT(m_currentTarget.frame < getCurrentAnimationRef().getFrameDuration());
    const animation::SkeletonPose skeletonPose = getCurrentAnimationRef().getInterpolatedPose(m_currentTarget.frame);

    m_boundingBox = skeletonPose.boundingBox;
    m_position = skeletonPose.position;

    BOOST_ASSERT(skeletonPose.bonePoses.size() <= m_bones.size());

    for(size_t k = 0; k < skeletonPose.bonePoses.size(); k++)
    {
        Bone& bone = m_bones[k];

        bone.position = skeletonPose.bonePoses[k].position;
        if(bone.parent == nullptr)
        {
            bone.localTransform = glm::translate(glm::mat4(1.0f), m_position + bone.position) * glm::mat4_cast(skeletonPose.bonePoses[k].rotation);
            bone.globalTransform = bone.localTransform;
        }
        else
        {
            bone.localTransform = glm::translate(glm::mat4(1.0f), bone.position) * glm::mat4_cast(skeletonPose.bonePoses[k].rotation);
            bone.globalTransform = bone.parent->globalTransform * bone.localTransform;
        }
    }
}

void Skeleton::copyMeshBinding(const std::shared_ptr<SkeletalModel>& model, bool resetMeshSlot)
{
    size_t meshes_to_copy = std::min(m_bones.size(), m_model->getMeshReferenceCount());

    for(size_t i = 0; i < meshes_to_copy; i++)
    {
        m_bones[i].mesh = model->getMeshReference(i).mesh_base;
        m_bones[i].mesh_skin = model->getMeshReference(i).mesh_skin;
        if(resetMeshSlot)
            m_bones[i].mesh_slot = nullptr;
    }
}

void Skeleton::setAnimation(AnimationId animation, int frame)
{
    const Animation& anim = m_model->getAnimation(animation);

    if(frame < 0)
        frame = anim.getFrameDuration() - 1 - ((-frame - 1) % anim.getFrameDuration());
    else
        frame %= anim.getFrameDuration();

    m_currentTarget.animation = animation;
    m_currentTarget.frame = frame;
    m_currentTarget.state = anim.state_id;
    m_previousTarget.state = anim.state_id;
}

/**
* Advance animation frame
* @param time          time step for animation
* @param cmdEntity     optional entity for which doAnimCommand is called
* @return  ENTITY_ANIM_NONE if frame is unchanged (time<rate), ENTITY_ANIM_NEWFRAME or ENTITY_ANIM_NEWANIM
*/
AnimUpdate Skeleton::stepAnimation(util::Duration time, Entity* cmdEntity)
{
    // FIXME: hack for reverse framesteps (weaponanims):
    if(time.count() < 0)
    {
        m_animationTime -= time;
        if(m_animationTime >= util::Duration::zero())
        {
            return AnimUpdate::None;
        }

        m_previousTarget.animation = m_currentTarget.animation;
        m_previousTarget.frame = m_currentTarget.frame;
        m_animationTime += AnimationFrameTime;
        if(m_currentTarget.frame > 0)
        {
            m_currentTarget.frame--;
            return AnimUpdate::NewFrame;
        }
        else
        {
            m_currentTarget.frame = getCurrentAnimationFrame().getFrameDuration() - 1;
            return AnimUpdate::NewAnim;
        }
    }

    m_animationTime += time;
    if(m_animationTime < AnimationFrameTime)
    {
        return AnimUpdate::None;
    }

    m_previousTarget.animation = m_currentTarget.animation;
    m_previousTarget.frame = m_currentTarget.frame;
    m_animationTime -= AnimationFrameTime;

    size_t frame_id = m_currentTarget.frame + 1;

    // check anim flags:
    if(m_mode == AnimationMode::LoopLastFrame)
    {
        if(frame_id >= getCurrentAnimationFrame().getFrameDuration())
        {
            m_currentTarget.frame = getCurrentAnimationFrame().getFrameDuration() - 1;
            return AnimUpdate::NewFrame; // period time has passed so it's a new frame, or should this be none?
        }
    }
    else if(m_mode == AnimationMode::Locked)
    {
        m_currentTarget.frame = 0;
        return AnimUpdate::NewFrame;
    }
    else if(m_mode == AnimationMode::WeaponCompat)
    {
        if(frame_id >= getCurrentAnimationFrame().getFrameDuration())
        {
            return AnimUpdate::NewAnim;
        }
        else
        {
            m_currentTarget.frame = frame_id;
            return AnimUpdate::NewFrame;
        }
    }

    // check state change:
    AnimUpdate stepResult = AnimUpdate::NewFrame;
    AnimationId anim_id = m_currentTarget.animation;
    if(m_currentTarget.state != m_previousTarget.state)
    {
        if(m_model->findStateChange(m_currentTarget.state, anim_id, frame_id))
        {
            m_previousTarget.state = m_model->getAnimation(anim_id).state_id;
            m_currentTarget.state = m_previousTarget.state;
            stepResult = AnimUpdate::NewAnim;
        }
    }

    // check end of animation:
    if(frame_id >= m_model->getAnimation(anim_id).getFrameDuration())
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : m_model->getAnimation(anim_id).finalAnimCommands) // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }

        if(m_model->getAnimation(anim_id).next_anim)
        {
            frame_id = m_model->getAnimation(anim_id).nextFrame;
            anim_id = m_model->getAnimation(anim_id).next_anim->id;

            // some overlay anims may have invalid nextAnim/nextFrame values:
            if(anim_id < m_model->getAnimationCount() && frame_id < m_model->getAnimation(anim_id).getFrameDuration())
            {
                m_previousTarget.state = m_model->getAnimation(anim_id).state_id;
                m_currentTarget.state = m_previousTarget.state;
            }
            else
            {
                // invalid values:
                anim_id = m_currentTarget.animation;
                frame_id = 0;
            }
        }
        else
        {
            frame_id = 0;
        }
        stepResult = AnimUpdate::NewAnim;
    }

    m_currentTarget.animation = anim_id;
    m_currentTarget.frame = frame_id;

    if(cmdEntity)
    {
        for(const AnimCommand& acmd : m_model->getAnimation(anim_id).getAnimCommands(frame_id))
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }

    return stepResult;
}

const Animation& Skeleton::getCurrentAnimationFrame() const
{
    return m_model->getAnimation(m_currentTarget.animation);
}

void Skeleton::updateTransform(const glm::mat4& entityTransform)
{
    const glm::mat4 inverseTransform = glm::inverse(entityTransform);
    for(Bone& bone : m_bones)
    {
        glm::mat4 tr;
        bone.bt_body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(tr));
        bone.globalTransform = inverseTransform * tr;
    }

    // that cycle is necessary only for skinning models;
    for(Bone& bone : m_bones)
    {
        if(bone.parent != nullptr)
        {
            bone.localTransform = glm::inverse(bone.parent->globalTransform) * bone.globalTransform;
        }
        else
        {
            bone.localTransform = bone.globalTransform;
        }
    }
}

void Skeleton::updateBoundingBox()
{
    m_boundingBox = m_bones[0].mesh->m_boundingBox;
    for(const Bone& bone : m_bones)
    {
        m_boundingBox.adjust(glm::vec3(bone.globalTransform[3]), bone.mesh->m_boundingBox.getMaximumExtent() * 0.5f);
    }
}

void Skeleton::createGhosts(Entity& entity)
{
    if(!m_model || !m_model->hasMeshReferences())
        return;

    m_manifoldArray = btManifoldArray();

    for(world::animation::Bone& bone : m_bones)
    {
        glm::vec3 box = GhostVolumeCollisionCoefficient * bone.mesh->m_boundingBox.getExtent();
        bone.shape = std::make_shared<btBoxShape>(util::convert(box));
        bone.shape->setMargin(COLLISION_MARGIN_DEFAULT);
        bone.mesh->m_radius = std::min(std::min(box.x, box.y), box.z);

        bone.ghostObject = std::make_shared<btPairCachingGhostObject>();

        bone.ghostObject->setIgnoreCollisionCheck(bone.bt_body.get(), true);

        glm::mat4 gltr = entity.m_transform * bone.globalTransform;
        gltr[3] = glm::vec4(glm::mat3(gltr) * bone.mesh->m_center, 1.0f);

        bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(gltr));
        bone.ghostObject->setCollisionFlags(bone.ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE |
                                            btCollisionObject::CF_CHARACTER_OBJECT);
        bone.ghostObject->setUserPointer(&entity);
        bone.ghostObject->setCollisionShape(bone.shape.get());
        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);

        bone.last_collisions.clear();
    }

    m_hasGhosts = true;
}

void Skeleton::updateCurrentCollisions(const Entity& entity, const glm::mat4& transform)
{
    if(!hasGhosts())
        return;

    for(world::animation::Bone& bone : m_bones)
    {
        auto v = bone.mesh->m_center;

        bone.last_collisions.clear();
        auto tr = transform * bone.globalTransform;
        auto orig_tr = bone.ghostObject->getWorldTransform();
        bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr));
        bone.ghostObject->getWorldTransform().setOrigin(util::convert(glm::vec3(tr * glm::vec4(v, 1.0f))));

        btBroadphasePairArray& pairArray = bone.ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
        btVector3 aabb_min, aabb_max;

        bone.ghostObject->getCollisionShape()->getAabb(bone.ghostObject->getWorldTransform(), aabb_min, aabb_max);
        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->getBroadphase()->setAabb(bone.ghostObject->getBroadphaseHandle(), aabb_min, aabb_max,
                                                                                       m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher());
        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(
            bone.ghostObject->getOverlappingPairCache(), m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatchInfo(), m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher());

        int num_pairs = bone.ghostObject->getOverlappingPairCache()->getNumOverlappingPairs();
        for(int j = 0; j < num_pairs; j++)
        {
            m_manifoldArray.clear();
            btBroadphasePair* collisionPair = &pairArray[j];

            if(!collisionPair)
            {
                continue;
            }

            if(collisionPair->m_algorithm)
            {
                collisionPair->m_algorithm->getAllContactManifolds(m_manifoldArray);
            }

            for(int k = 0; k < m_manifoldArray.size(); k++)
            {
                btPersistentManifold* manifold = m_manifoldArray[k];
                for(int c = 0; c < manifold->getNumContacts(); c++)
                {
                    if(manifold->getContactPoint(c).getDistance() < 0.0)
                    {
                        bone.last_collisions.emplace_back(const_cast<btCollisionObject*>(m_manifoldArray[k]->getBody0()));
                        if(&entity == static_cast<Entity*>(bone.last_collisions.back()->getUserPointer()))
                        {
                            bone.last_collisions.back() = const_cast<btCollisionObject*>(m_manifoldArray[k]->getBody1());
                        }
                        break;
                    }
                }
            }
        }
        bone.ghostObject->setWorldTransform(orig_tr);
    }
}

bool Skeleton::createRagdoll(const RagdollSetup& setup)
{
    bool result = true;
    for(size_t i = 0; i < setup.body_setup.size(); i++)
    {
        if(i >= m_bones.size() || !m_bones[i].bt_body)
        {
            result = false;
            continue; // If body is absent, return false and bypass this body setup.
        }

        btVector3 inertia(0.0, 0.0, 0.0);
        btScalar mass = setup.body_setup[i].mass;

        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->removeRigidBody(m_bones[i].bt_body.get());

        m_bones[i].bt_body->getCollisionShape()->calculateLocalInertia(mass, inertia);
        m_bones[i].bt_body->setMassProps(mass, inertia);

        m_bones[i].bt_body->updateInertiaTensor();
        m_bones[i].bt_body->clearForces();

        m_bones[i].bt_body->setLinearFactor(btVector3(1.0, 1.0, 1.0));
        m_bones[i].bt_body->setAngularFactor(btVector3(1.0, 1.0, 1.0));

        m_bones[i].bt_body->setDamping(setup.body_setup[i].damping[0], setup.body_setup[i].damping[1]);
        m_bones[i].bt_body->setRestitution(setup.body_setup[i].restitution);
        m_bones[i].bt_body->setFriction(setup.body_setup[i].friction);

        m_bones[i].bt_body->setSleepingThresholds(RagdollDefaultSleepingThreshold, RagdollDefaultSleepingThreshold);

        if(!m_bones[i].parent)
        {
            glm::float_t r = m_bones[i].mesh->m_boundingBox.getMinimumExtent();
            m_bones[i].bt_body->setCcdMotionThreshold(0.8f * r);
            m_bones[i].bt_body->setCcdSweptSphereRadius(r);
        }
    }
    return result;
}

void Skeleton::initCollisions(const glm::vec3& speed)
{
    for(animation::Bone& bone : m_bones)
    {
        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->addRigidBody(bone.bt_body.get());
        bone.bt_body->activate();
        bone.bt_body->setLinearVelocity(util::convert(speed));
        if(bone.ghostObject)
        {
            m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->removeCollisionObject(bone.ghostObject.get());
            m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_NONE, COLLISION_NONE);
        }
    }
}

void Skeleton::updateRigidBody(const glm::mat4& transform)
{
    for(animation::Bone& bone : m_bones)
    {
        if(bone.bt_body)
        {
            bone.bt_body->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(transform * bone.globalTransform));
        }
    }
}

btCollisionObject* Skeleton::getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t& curr_flag)
{
    curr_flag = 0x00;
    if(!hasGhosts())
        return nullptr;

    for(animation::Bone& bone : m_bones)
    {
        if(bone.body_part & parts_flags)
        {
            if(!bone.last_collisions.empty())
            {
                curr_flag = bone.body_part;
                auto res = bone.last_collisions.back();
                bone.last_collisions.pop_back();
                return res;
            }
        }
    }

    return nullptr;
}

void Skeleton::genRigidBody(Entity& entity)
{
    if(!m_model || entity.getCollisionType() == world::CollisionType::None)
        return;

    for(animation::Bone& bone : m_bones)
    {
        btCollisionShape* cshape = nullptr;
        switch(entity.getCollisionShape())
        {
            case world::CollisionShape::Sphere:
                cshape = core::BT_CSfromSphere(bone.mesh->m_radius);
                break;

            case world::CollisionShape::TriMeshConvex:
                cshape = core::BT_CSfromMesh(bone.mesh, true, true, false);
                break;

            case world::CollisionShape::TriMesh:
                cshape = core::BT_CSfromMesh(bone.mesh, true, true, true);
                break;

            case world::CollisionShape::Box:
            default:
                cshape = core::BT_CSfromBBox(bone.mesh->m_boundingBox, true, true);
                break;
        };

        if(!cshape)
            continue;

        btVector3 localInertia(0, 0, 0);
        if(entity.getCollisionShape() != world::CollisionShape::TriMesh)
            cshape->calculateLocalInertia(0.0, localInertia);

        btTransform startTransform;
        startTransform.setFromOpenGLMatrix(glm::value_ptr(entity.m_transform * bone.globalTransform));
        bone.bt_body = std::make_shared<btRigidBody>(0, new btDefaultMotionState(startTransform), cshape, localInertia);

        switch(entity.getCollisionType())
        {
            case world::CollisionType::Kinematic:
                bone.bt_body->setCollisionFlags(bone.bt_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                break;

            case world::CollisionType::Ghost:
                bone.bt_body->setCollisionFlags(bone.bt_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
                break;

            case world::CollisionType::Actor:
            case world::CollisionType::Vehicle:
                bone.bt_body->setCollisionFlags(bone.bt_body->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
                break;

            case world::CollisionType::Static:
            default:
                bone.bt_body->setCollisionFlags(bone.bt_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
                break;
        }

        m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->addRigidBody(bone.bt_body.get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        bone.bt_body->setUserPointer(&entity);
    }
}

void Skeleton::enableCollision()
{
    for(const world::animation::Bone& bone : m_bones)
    {
        if(bone.bt_body && !bone.bt_body->isInWorld())
            m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->addRigidBody(bone.bt_body.get());
    }
}

void Skeleton::disableCollision()
{
    for(const world::animation::Bone& bone : m_bones)
    {
        if(bone.bt_body && bone.bt_body->isInWorld())
            m_entity->getWorld()->m_engine->m_bullet.dynamicsWorld->removeRigidBody(bone.bt_body.get());
    }
}

const Animation& Skeleton::getCurrentAnimationRef() const
{
    return m_model->getAnimation(m_currentTarget.animation);
}
}
}
