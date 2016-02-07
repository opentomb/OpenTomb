#pragma once

#include "bone.h"
#include "transition.h"
#include "world/skeletalmodel.h"

#include <BulletCollision/btBulletCollisionCommon.h>

namespace world
{
struct RagdollSetup;
}

namespace world
{
namespace animation
{
class Skeleton
{
private:
    Entity* m_entity;

    std::vector<Bone> m_bones{};
    glm::vec3 m_position = { 0, 0, 0 };
    core::BoundingBox m_boundingBox{};

    bool m_hasSkin = false; //!< whether any skinned meshes need rendering

    std::shared_ptr<SkeletalModel> m_model = nullptr;

    AnimationState m_previousState;
    AnimationState m_currentState;

    AnimationMode m_mode = AnimationMode::NormalControl;

    btManifoldArray m_manifoldArray;

    bool m_hasGhosts = false;

    util::Duration m_animationTime = util::Duration::zero();

public:
    explicit Skeleton(Entity* entity)
        : m_entity(entity)
    {
    }

    Entity* getEntity() const
    {
        return m_entity;
    }

    void (*onFrame)(Character& ent, AnimUpdate state) = nullptr;

    const Animation& getCurrentAnimation() const;
    AnimUpdate stepAnimation(util::Duration time, Entity* cmdEntity = nullptr);
    void setAnimation(AnimationId animation, int frame = 0);

    AnimationId getCurrentAnimationId() const noexcept
    {
        return m_currentState.animation;
    }
    void setCurrentAnimationId(AnimationId value) noexcept
    {
        m_currentState.animation = value;
    }

    AnimationId getPreviousAnimationId() const noexcept
    {
        return m_previousState.animation;
    }
    void setPreviousAnimationId(AnimationId value) noexcept
    {
        m_previousState.animation = value;
    }

    size_t getCurrentFrame() const noexcept
    {
        return m_currentState.frame;
    }
    void setCurrentFrame(size_t value) noexcept
    {
        m_currentState.frame = value;
    }

    size_t getPreviousFrame() const noexcept
    {
        return m_previousState.frame;
    }
    void setPreviousFrame(size_t value) noexcept
    {
        m_previousState.frame = value;
    }

    const std::shared_ptr<SkeletalModel>& getModel() const noexcept
    {
        return m_model;
    }
    void setModel(const std::shared_ptr<SkeletalModel>& model) noexcept
    {
        m_model = model;
    }

    LaraState getCurrentState() const noexcept
    {
        return m_currentState.state;
    }
    LaraState getPreviousState() const noexcept
    {
        return m_previousState.state;
    }

    bool hasGhosts() const noexcept
    {
        return m_hasGhosts;
    }

    void fromModel(const std::shared_ptr<SkeletalModel>& model);

    /**
    * That function updates item animation and rebuilds skeletal matrices;
    */
    void itemFrame(util::Duration time);

    /**
    * @brief Update bone transformations from key frame interpolation.
    */
    void updatePose();

    const core::BoundingBox& getBoundingBox() const noexcept
    {
        return m_boundingBox;
    }

    const glm::mat4& getRootTransform() const
    {
        BOOST_ASSERT(!m_bones.empty());
        return m_bones.front().globalTransform;
    }

    size_t getBoneCount() const noexcept
    {
        return m_bones.size();
    }

    const std::vector<Bone>& getBones() const noexcept
    {
        return m_bones;
    }

    std::vector<Bone>& bones() noexcept
    {
        return m_bones;
    }

    Bone& bone(size_t i)
    {
        BOOST_ASSERT(i < m_bones.size());
        return m_bones[i];
    }

    void setBodyPartFlag(size_t boneId, uint32_t flag)
    {
        if(boneId >= m_bones.size())
            return;

        m_bones[boneId].body_part = flag;
    }

    void copyMeshBinding(const std::shared_ptr<SkeletalModel>& model, bool resetMeshSlot = false);

    void setAnimationMode(AnimationMode mode) noexcept
    {
        m_mode = mode;
    }
    AnimationMode getAnimationMode() const noexcept
    {
        return m_mode;
    }

    void setCurrentState(LaraState state) noexcept
    {
        m_currentState.state = state;
    }

    void setPreviousState(LaraState state) noexcept
    {
        m_previousState.state = state;
    }

    const btManifoldArray& getManifoldArray() const noexcept
    {
        return m_manifoldArray;
    }

    btManifoldArray& manifoldArray() noexcept
    {
        return m_manifoldArray;
    }

    void updateTransform(const glm::mat4& entityTransform);

    void updateBoundingBox();

    void createGhosts(Entity& entity);

    void cleanCollisionBodyParts(uint32_t parts_flags)
    {
        for(Bone& bone : m_bones)
        {
            if(bone.body_part & parts_flags)
            {
                bone.last_collisions.clear();
            }
        }
    }

    void cleanCollisionAllBodyParts()
    {
        for(Bone& bone : m_bones)
        {
            bone.last_collisions.clear();
        }
    }

    void updateCurrentCollisions(const Entity& entity, const glm::mat4& transform);
    bool createRagdoll(const RagdollSetup& setup);
    void initCollisions(const glm::vec3& speed);
    void updateRigidBody(const glm::mat4& transform);
    btCollisionObject* getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t& curr_flag);
    void genRigidBody(Entity& entity);
    void enableCollision();
    void disableCollision();
};
}
}
