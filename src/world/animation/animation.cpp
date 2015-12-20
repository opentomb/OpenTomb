#include "animation.h"

#include "engine/engine.h"
#include "world/core/basemesh.h"
#include "world/core/util.h"
#include "world/entity.h"
#include "world/ragdoll.h"
#include "world/skeletalmodel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>

#include <stack>

constexpr float GhostVolumeCollisionCoefficient = 0.4f;

namespace world
{
namespace animation
{

void Skeleton::fromModel(SkeletalModel* model)
{
    m_hasSkin = false;
    m_boundingBox.min = {0, 0, 0};
    m_boundingBox.max = {0, 0, 0};
    m_position = {0, 0, 0};

    m_model = model;

    m_bones.resize(model->meshes.size());

    std::stack<Bone*> parents;
    parents.push(nullptr);
    m_bones[0].parent = nullptr; // root
    for(size_t i = 0; i < m_bones.size(); i++)
    {
        m_bones[i].index = i;
        m_bones[i].mesh = model->meshes[i].mesh_base;
        m_bones[i].mesh_skin = model->meshes[i].mesh_skin;
        if(m_bones[i].mesh_skin)
            m_hasSkin = true;
        m_bones[i].mesh_slot = nullptr;
        m_bones[i].body_part = model->meshes[i].body_part;

        m_bones[i].offset = model->meshes[i].offset;
        m_bones[i].qrotate = {0, 0, 0, 0};
        m_bones[i].transform = glm::mat4(1.0f);
        m_bones[i].full_transform = glm::mat4(1.0f);

        if(i == 0)
            continue;

        m_bones[i].parent = &m_bones[i - 1];
        if(model->meshes[i].flag & 0x01) // POP
        {
            if(!parents.empty())
            {
                m_bones[i].parent = parents.top();
                parents.pop();
            }
        }
        if(model->meshes[i].flag & 0x02) // PUSH
        {
            if(parents.size() + 1 < model->meshes.size())
            {
                parents.push(m_bones[i].parent);
            }
        }
    }
}

void Skeleton::itemFrame(util::Duration time)
{
    stepAnimation(time, nullptr);
    updatePose();
}

void Skeleton::updatePose()
{
    BOOST_ASSERT( m_currentAnimation.animation < m_model->animations.size() );
    BOOST_ASSERT( m_currentAnimation.frame < m_model->animations[m_currentAnimation.animation].getFrameDuration() );
    const animation::SkeletonKeyFrame keyFrame = m_model->animations[m_currentAnimation.animation].getInterpolatedFrame(m_currentAnimation.frame);

    m_boundingBox = keyFrame.boundingBox;
    m_position = keyFrame.position;

    BOOST_ASSERT(keyFrame.boneKeyFrames.size() <= m_bones.size());

    for(size_t k = 0; k < keyFrame.boneKeyFrames.size(); k++)
    {
        m_bones[k].offset = keyFrame.boneKeyFrames[k].offset;
        m_bones[k].transform = glm::translate(glm::mat4(1.0f), m_bones[k].offset);
        if(k == 0)
        {
            m_bones[k].transform = glm::translate(m_bones[k].transform, m_position);
        }

        m_bones[k].qrotate = keyFrame.boneKeyFrames[k].qrotate;
        m_bones[k].transform *= glm::mat4_cast(m_bones[k].qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    m_bones[0].full_transform = m_bones[0].transform;
    for(size_t k = 1; k < keyFrame.boneKeyFrames.size(); k++)
    {
        BOOST_ASSERT(m_bones[k].parent != nullptr);
        m_bones[k].full_transform = m_bones[k].parent->full_transform * m_bones[k].transform;
    }
}

void Skeleton::copyMeshBinding(const SkeletalModel* model, bool resetMeshSlot)
{
    size_t meshes_to_copy = std::min(m_bones.size(), m_model->meshes.size());

    for(size_t i = 0; i < meshes_to_copy; i++)
    {
        m_bones[i].mesh = model->meshes[i].mesh_base;
        m_bones[i].mesh_skin = model->meshes[i].mesh_skin;
        if(resetMeshSlot)
            m_bones[i].mesh_slot = nullptr;
    }
}

void Skeleton::setAnimation(int animation, int frame)
{
    // FIXME: is anim < 0 actually happening?
    animation = (animation < 0) ? (0) : (animation);

    Animation* anim = &m_model->animations[animation];

    if(frame < 0)
        frame = anim->getFrameDuration() - 1 - ((-frame - 1) % anim->getFrameDuration());
    else
        frame %= anim->getFrameDuration();

    m_currentAnimation.animation = animation;
    m_currentAnimation.frame = frame;
    m_currentAnimation.state = anim->state_id;
    m_previousAnimation.state = anim->state_id;
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

        m_previousAnimation.animation = m_currentAnimation.animation;
        m_previousAnimation.frame = m_currentAnimation.frame;
        m_animationTime += AnimationFrameTime;
        if(m_currentAnimation.frame > 0)
        {
            m_currentAnimation.frame--;
            return AnimUpdate::NewFrame;
        }
        else
        {
            m_currentAnimation.frame = getCurrentAnimationFrame().getFrameDuration() - 1;
            return AnimUpdate::NewAnim;
        }
    }

    m_animationTime += time;
    if(m_animationTime < AnimationFrameTime)
    {
        return AnimUpdate::None;
    }

    m_previousAnimation.animation = m_currentAnimation.animation;
    m_previousAnimation.frame = m_currentAnimation.frame;
    m_animationTime -= AnimationFrameTime;

    uint16_t frame_id = m_currentAnimation.frame + 1;

    // check anim flags:
    if(m_mode == AnimationMode::LoopLastFrame)
    {
        if(frame_id >= getCurrentAnimationFrame().getFrameDuration())
        {
            m_currentAnimation.frame = getCurrentAnimationFrame().getFrameDuration() - 1;
            return AnimUpdate::NewFrame; // period time has passed so it's a new frame, or should this be none?
        }
    }
    else if(m_mode == AnimationMode::Locked)
    {
        m_currentAnimation.frame = 0;
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
            m_currentAnimation.frame = frame_id;
            return AnimUpdate::NewFrame;
        }
    }

    // check state change:
    AnimUpdate stepResult = AnimUpdate::NewFrame;
    auto anim_id = m_currentAnimation.animation;
    if(m_currentAnimation.state != m_previousAnimation.state)
    {
        if(m_model->findStateChange(m_currentAnimation.state, anim_id, frame_id))
        {
            m_previousAnimation.state = m_model->animations[anim_id].state_id;
            m_currentAnimation.state = m_previousAnimation.state;
            stepResult = AnimUpdate::NewAnim;
        }
    }

    // check end of animation:
    if(frame_id >= m_model->animations[anim_id].getFrameDuration())
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : m_model->animations[anim_id].finalAnimCommands) // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }

        if(m_model->animations[anim_id].next_anim)
        {
            frame_id = m_model->animations[anim_id].next_frame;
            anim_id = m_model->animations[anim_id].next_anim->id;

            // some overlay anims may have invalid nextAnim/nextFrame values:
            if(anim_id < m_model->animations.size() && frame_id < m_model->animations[anim_id].getFrameDuration())
            {
                m_previousAnimation.state = m_model->animations[anim_id].state_id;
                m_currentAnimation.state = m_previousAnimation.state;
            }
            else
            {
                // invalid values:
                anim_id = m_currentAnimation.animation;
                frame_id = 0;
            }
        }
        else
        {
            frame_id = 0;
        }
        stepResult = AnimUpdate::NewAnim;
    }

    m_currentAnimation.animation = anim_id;
    m_currentAnimation.frame = frame_id;

    if(cmdEntity)
    {
        for(AnimCommand acmd : m_model->animations[anim_id].animCommands(frame_id))
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }

    return stepResult;
}

const Animation& Skeleton::getCurrentAnimationFrame() const
{
    return m_model->animations[m_currentAnimation.animation];
}

void Skeleton::updateTransform(const glm::mat4& entityTransform)
{
    const glm::mat4 inverseTransform = glm::inverse(entityTransform);
    for(Bone& bone : m_bones)
    {
        glm::mat4 tr;
        bone.bt_body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(tr));
        bone.full_transform = inverseTransform * tr;
    }

    // that cycle is necessary only for skinning models;
    for(Bone& bone : m_bones)
    {
        if(bone.parent != nullptr)
        {
            bone.transform = glm::inverse(bone.parent->full_transform) * bone.full_transform;
        }
        else
        {
            bone.transform = bone.full_transform;
        }
    }
}

void Skeleton::updateBoundingBox()
{
    m_boundingBox = m_bones[0].mesh->boundingBox;
    for(const Bone& bone : m_bones)
    {
        m_boundingBox.adjust(glm::vec3(bone.full_transform[3]), bone.mesh->boundingBox.getOuterDiameter() * 0.5f);
    }
}

void Skeleton::createGhosts(Entity& entity)
{
    if(!m_model || m_model->meshes.empty())
        return;

    m_manifoldArray = btManifoldArray();

    for(world::animation::Bone& bone : m_bones)
    {
        glm::vec3 box = GhostVolumeCollisionCoefficient * bone.mesh->boundingBox.getDiameter();
        bone.shape = std::make_shared<btBoxShape>(util::convert(box));
        bone.shape->setMargin(COLLISION_MARGIN_DEFAULT);
        bone.mesh->m_radius = std::min(std::min(box.x, box.y), box.z);

        bone.ghostObject = std::make_shared<btPairCachingGhostObject>();

        bone.ghostObject->setIgnoreCollisionCheck(bone.bt_body.get(), true);

        glm::mat4 gltr = entity.m_transform * bone.full_transform;
        gltr[3] = glm::vec4(glm::mat3(gltr) * bone.mesh->m_center, 1.0f);

        bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(gltr));
        bone.ghostObject->setCollisionFlags(bone.ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE |
                                            btCollisionObject::CF_CHARACTER_OBJECT);
        bone.ghostObject->setUserPointer(&entity);
        bone.ghostObject->setCollisionShape(bone.shape.get());
        engine::bt_engine_dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);

        bone.last_collisions.clear();
    }

    m_hasGhosts = true;
}

Bone::~Bone()
{
    if(ghostObject)
    {
        ghostObject->setUserPointer(nullptr);
        engine::bt_engine_dynamicsWorld->removeCollisionObject(ghostObject.get());
    }

    if(bt_body)
    {
        bt_body->setUserPointer(nullptr);

        delete bt_body->getMotionState();
        bt_body->setMotionState(nullptr);

        bt_body->setCollisionShape(nullptr);

        engine::bt_engine_dynamicsWorld->removeRigidBody(bt_body.get());
    }
}

void Skeleton::updateCurrentCollisions(const Entity& entity, const glm::mat4& transform)
{
    if(!hasGhosts())
        return;

    for(world::animation::Bone& bone : m_bones)
    {
        auto v = bone.mesh->m_center;

        bone.last_collisions.clear();
        auto tr = transform * bone.full_transform;
        auto orig_tr = bone.ghostObject->getWorldTransform();
        bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(tr));
        bone.ghostObject->getWorldTransform().setOrigin(util::convert(glm::vec3(tr * glm::vec4(v, 1.0f))));

        btBroadphasePairArray& pairArray = bone.ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
        btVector3 aabb_min, aabb_max;

        bone.ghostObject->getCollisionShape()->getAabb(bone.ghostObject->getWorldTransform(), aabb_min, aabb_max);
        engine::bt_engine_dynamicsWorld->getBroadphase()->setAabb(bone.ghostObject->getBroadphaseHandle(), aabb_min, aabb_max,
                                                                  engine::bt_engine_dynamicsWorld->getDispatcher());
        engine::bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(
            bone.ghostObject->getOverlappingPairCache(), engine::bt_engine_dynamicsWorld->getDispatchInfo(), engine::bt_engine_dynamicsWorld->getDispatcher());

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

bool Skeleton::createRagdoll(const RDSetup& setup)
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

        engine::bt_engine_dynamicsWorld->removeRigidBody(m_bones[i].bt_body.get());

        m_bones[i].bt_body->getCollisionShape()->calculateLocalInertia(mass, inertia);
        m_bones[i].bt_body->setMassProps(mass, inertia);

        m_bones[i].bt_body->updateInertiaTensor();
        m_bones[i].bt_body->clearForces();

        m_bones[i].bt_body->setLinearFactor(btVector3(1.0, 1.0, 1.0));
        m_bones[i].bt_body->setAngularFactor(btVector3(1.0, 1.0, 1.0));

        m_bones[i].bt_body->setDamping(setup.body_setup[i].damping[0], setup.body_setup[i].damping[1]);
        m_bones[i].bt_body->setRestitution(setup.body_setup[i].restitution);
        m_bones[i].bt_body->setFriction(setup.body_setup[i].friction);

        m_bones[i].bt_body->setSleepingThresholds(RD_DEFAULT_SLEEPING_THRESHOLD, RD_DEFAULT_SLEEPING_THRESHOLD);

        if(!m_bones[i].parent)
        {
            glm::float_t r = m_bones[i].mesh->boundingBox.getInnerDiameter();
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
        engine::bt_engine_dynamicsWorld->addRigidBody(bone.bt_body.get());
        bone.bt_body->activate();
        bone.bt_body->setLinearVelocity(util::convert(speed));
        if(bone.ghostObject)
        {
            engine::bt_engine_dynamicsWorld->removeCollisionObject(bone.ghostObject.get());
            engine::bt_engine_dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_NONE, COLLISION_NONE);
        }
    }
}

void Skeleton::updateRigidBody(const glm::mat4& transform)
{
    for(animation::Bone& bone : m_bones)
    {
        if(bone.bt_body)
        {
            bone.bt_body->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(transform * bone.full_transform));
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
                cshape = core::BT_CSfromBBox(bone.mesh->boundingBox, true, true);
                break;
        };

        if(!cshape)
            continue;

        btVector3 localInertia(0, 0, 0);
        if(entity.getCollisionShape() != world::CollisionShape::TriMesh)
            cshape->calculateLocalInertia(0.0, localInertia);

        btTransform startTransform;
        startTransform.setFromOpenGLMatrix(glm::value_ptr(entity.m_transform * bone.full_transform));
        bone.bt_body = std::make_shared<btRigidBody>(0.0, new btDefaultMotionState(startTransform), cshape, localInertia);

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

        engine::bt_engine_dynamicsWorld->addRigidBody(bone.bt_body.get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        bone.bt_body->setUserPointer(&entity);
    }
}

void Skeleton::enableCollision()
{
    for(const world::animation::Bone& bone : m_bones)
    {
        if(bone.bt_body && !bone.bt_body->isInWorld())
            engine::bt_engine_dynamicsWorld->addRigidBody(bone.bt_body.get());
    }
}

void Skeleton::disableCollision()
{
    for(const world::animation::Bone& bone : m_bones)
    {
        if(bone.bt_body && bone.bt_body->isInWorld())
            engine::bt_engine_dynamicsWorld->removeRigidBody(bone.bt_body.get());
    }
}

} // namespace animation
} // namespace world
