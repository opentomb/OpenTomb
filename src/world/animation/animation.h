#pragma once

#include "animcommands.h"
#include "util/helpers.h"
#include "world/core/boundingbox.h"
#include "world/statecontroller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <boost/container/flat_map.hpp>

#include <memory>
#include <vector>

namespace world
{
class Character;
class SkeletalModel;
class Entity;
struct RagdollSetup;
class World;
enum class CollisionShape;
enum class CollisionType;

namespace core
{
struct BaseMesh;
} // namespace core

namespace animation
{
using AnimationId = uint32_t;

//! Default fixed TR framerate needed for animation calculation
constexpr float AnimationFrameRate = 30;
constexpr util::Duration AnimationFrameTime = util::fromSeconds(1.0f / AnimationFrameRate);

// This is the global game logic refresh interval (physics timestep)
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.
// This should be a multiple of animation::FrameRate (1/30,60,90,120,...)
constexpr float GameLogicFrameRate = 2 * AnimationFrameRate;
constexpr util::Duration GameLogicFrameTime = util::fromSeconds(1.0f / GameLogicFrameRate);

enum class AnimUpdate
{
    None,
    NewFrame,
    NewAnim
};

struct AnimCommand
{
    AnimCommandOpcode cmdId;
    int param[3];
};

/*
 * Animated version of vertex. Does not contain texture coordinate, because that is in a different VBO.
 */
struct AnimatedVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};

// Animated texture types
enum class TextureAnimationType
{
    Forward,
    Backward,
    Reverse
};

/*
 *  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
 */
struct TextureAnimationKeyFrame
{
    glm::mat2 coordinateTransform;
    glm::vec2 move;
    size_t textureIndex;
};

struct TextureAnimationSequence
{
    bool uvrotate = false;   //!< UVRotate mode flag.
    glm::float_t uvrotateSpeed;   // Speed of UVRotation, in seconds.
    glm::float_t uvrotateMax;     // Reference value used to restart rotation.

    bool frame_lock = false; //!< Single frame mode. Needed for TR4-5 compatible UVRotate.

    TextureAnimationType textureType = TextureAnimationType::Forward;
    bool reverse = false;    // Used only with type 2 to identify current animation direction.
    util::Duration frameTime = util::Duration::zero(); // Time passed since last frame update.
    size_t currentFrame = 0;    // Current frame for this sequence.
    util::Duration timePerFrame = util::MilliSeconds(50); // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    std::vector<TextureAnimationKeyFrame> keyFrames;
    std::vector<size_t> textureIndices; // Offset into anim textures frame list.
};

struct AnimationState
{
    AnimationId animation = 0;
    size_t frame = 0;
    LaraState state = LaraState::WalkForward;
};

/*
 * animation switching control structure
 */
struct AnimationDispatch
{
    AnimationState next;  //!< "switch to" animation
    size_t start;  //!< low border of state change condition
    size_t end; //!< high border of state change condition
};

struct StateChange
{
    LaraState id;
    std::vector<AnimationDispatch> dispatches;
};

/**
 * Defines position and rotation in the parent's coordinate system
 */
struct BoneKeyFrame
{
    glm::vec3 offset;
    glm::quat qrotate;
};

/**
 * Collection of @c BoneKeyFrame and @c AnimCommand
 */
struct SkeletonKeyFrame
{
    std::vector<BoneKeyFrame> boneKeyFrames;
    glm::vec3 position = { 0,0,0 };
    core::BoundingBox boundingBox;
};

/**
 * A sequence of keyframes.
 */
struct Animation
{
    AnimationId id;
    int32_t speed_x; // Forward-backward speed
    int32_t accel_x; // Forward-backward accel
    int32_t speed_y; // Left-right speed
    int32_t accel_y; // Left-right accel
    size_t animationCommand;
    size_t animationCommandCount;
    LaraState state_id;

    boost::container::flat_map<LaraState, StateChange> stateChanges;

    Animation* next_anim = nullptr; // Next default animation
    size_t next_frame;                 // Next default frame

    std::vector<AnimCommand> finalAnimCommands; // cmds for end-of-anim

    const StateChange* findStateChangeByID(LaraState id) const noexcept
    {
        auto it = stateChanges.find(id);
        if(it == stateChanges.end())
            return nullptr;

        return &it->second;
    }

    StateChange* findStateChangeByID(LaraState id) noexcept
    {
        auto it = stateChanges.find(id);
        if(it == stateChanges.end())
            return nullptr;

        return &it->second;
    }

    const BoneKeyFrame& getInitialBoneKeyFrame() const
    {
        return m_keyFrames.front().boneKeyFrames.front();
    }

    SkeletonKeyFrame getInterpolatedFrame(size_t frame) const
    {
        BOOST_ASSERT(frame < m_duration);
        const size_t frameIndex = frame / m_stretchFactor;
        BOOST_ASSERT(frameIndex < m_keyFrames.size());
        const SkeletonKeyFrame& first = m_keyFrames[frameIndex];
        const SkeletonKeyFrame& second = frameIndex + 1 >= m_keyFrames.size()
            ? m_keyFrames.back()
            : m_keyFrames[frameIndex + 1];

        BOOST_ASSERT(first.boneKeyFrames.size() == second.boneKeyFrames.size());

        const size_t subOffset = frame - frameIndex*m_stretchFactor; // offset between keyframes
        if(subOffset == 0)
            return first; // no need to interpolate

        const glm::float_t lerp = static_cast<glm::float_t>(subOffset) / static_cast<glm::float_t>(m_stretchFactor);

        SkeletonKeyFrame result;
        result.position = glm::mix(first.position, second.position, lerp);
        result.boundingBox.max = glm::mix(first.boundingBox.max, second.boundingBox.max, lerp);
        result.boundingBox.min = glm::mix(first.boundingBox.min, second.boundingBox.min, lerp);

        result.boneKeyFrames.resize(first.boneKeyFrames.size());

        for(size_t k = 0; k < first.boneKeyFrames.size(); k++)
        {
            result.boneKeyFrames[k].offset = glm::mix(first.boneKeyFrames[k].offset, second.boneKeyFrames[k].offset, lerp);
            result.boneKeyFrames[k].qrotate = glm::slerp(first.boneKeyFrames[k].qrotate, second.boneKeyFrames[k].qrotate, lerp);
        }

        return result;
    }

    SkeletonKeyFrame& rawKeyFrame(size_t idx)
    {
        if(idx >= m_keyFrames.size())
            throw std::out_of_range("Keyframe index out of bounds");
        return m_keyFrames[idx];
    }

    size_t getFrameDuration() const
    {
        return m_duration;
    }

    void setDuration(size_t frames, size_t keyFrames, uint8_t stretchFactor)
    {
        BOOST_ASSERT(stretchFactor > 0);
        BOOST_ASSERT(frames > 0);
        m_keyFrames.resize(keyFrames);
        m_duration = frames;
        m_stretchFactor = stretchFactor;
    }

    size_t getKeyFrameCount() const noexcept
    {
        return m_keyFrames.size();
    }

    uint8_t getStretchFactor() const
    {
        return m_stretchFactor;
    }

    std::vector<AnimCommand>& animCommands(int frame)
    {
        return m_animCommands[frame];
    }

private:
    std::vector<SkeletonKeyFrame> m_keyFrames;
    uint8_t m_stretchFactor = 1; //!< Time scale (>1 means slowdown)
    std::map<int, std::vector<AnimCommand>> m_animCommands; //!< Maps from real frame index to commands
    size_t m_duration = 1; //!< Real frame duration
};

enum class AnimationMode
{
    NormalControl,
    LoopLastFrame,
    WeaponCompat,
    Locked
};

using BoneId = uint32_t;

class Skeleton;

/**
 * @brief A single bone in a @c Skeleton
 */
struct Bone
{
    Skeleton* m_skeleton;

    Bone* parent;
    BoneId index;
    std::shared_ptr<core::BaseMesh> mesh; //!< The mesh this bone deforms
    std::shared_ptr<core::BaseMesh> mesh_skin;
    std::shared_ptr<core::BaseMesh> mesh_slot; //!< Optional additional mesh
    glm::vec3 offset;

    glm::quat qrotate;
    glm::mat4 transform;      //!< Local transformation matrix
    glm::mat4 full_transform; //!< Global transformation matrix

    uint32_t body_part; //!< flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...

    std::shared_ptr<btRigidBody> bt_body;
    std::shared_ptr<btPairCachingGhostObject> ghostObject; // like Bullet character controller for penetration resolving.
    std::shared_ptr<btCollisionShape> shape;
    std::vector<btCollisionObject*> last_collisions;

    explicit Bone(Skeleton* skeleton)
        : m_skeleton(skeleton)
    {
    }

    ~Bone();
};

class Skeleton
{
private:
    World* m_world;

    std::vector<Bone> m_bones{};
    glm::vec3 m_position = { 0, 0, 0 };
    core::BoundingBox m_boundingBox{};

    bool m_hasSkin = false; //!< whether any skinned meshes need rendering

    std::shared_ptr<SkeletalModel> m_model = nullptr;

    AnimationState m_previousAnimation;
    AnimationState m_currentAnimation;

    AnimationMode m_mode = AnimationMode::NormalControl;

    btManifoldArray m_manifoldArray;

    bool m_hasGhosts = false;

    util::Duration m_animationTime = util::Duration::zero();

public:
    explicit Skeleton(World* world)
        : m_world(world)
    {
    }

    World* getWorld() const
    {
        return m_world;
    }

    void(*onFrame)(Character& ent, AnimUpdate state) = nullptr;

    const Animation& getCurrentAnimationFrame() const;
    AnimUpdate stepAnimation(util::Duration time, Entity* cmdEntity = nullptr);
    void setAnimation(AnimationId animation, int frame = 0);

    AnimationId getCurrentAnimation() const noexcept
    {
        return m_currentAnimation.animation;
    }
    void setCurrentAnimation(AnimationId value) noexcept
    {
        m_currentAnimation.animation = value;
    }

    AnimationId getPreviousAnimation() const noexcept
    {
        return m_previousAnimation.animation;
    }
    void setPreviousAnimation(AnimationId value) noexcept
    {
        m_previousAnimation.animation = value;
    }

    size_t getCurrentFrame() const noexcept
    {
        return m_currentAnimation.frame;
    }
    void setCurrentFrame(size_t value) noexcept
    {
        m_currentAnimation.frame = value;
    }

    size_t getPreviousFrame() const noexcept
    {
        return m_previousAnimation.frame;
    }
    void setPreviousFrame(size_t value) noexcept
    {
        m_previousAnimation.frame = value;
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
        return m_currentAnimation.state;
    }
    LaraState getPreviousState() const noexcept
    {
        return m_previousAnimation.state;
    }

    bool hasGhosts() const noexcept
    {
        return m_hasGhosts;
    }

    void fromModel(const std::shared_ptr<SkeletalModel>& m_model);

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
        return m_bones.front().full_transform;
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
        m_currentAnimation.state = state;
    }

    void setPreviousState(LaraState state) noexcept
    {
        m_previousAnimation.state = state;
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
} // namespace animation
} // namespace world
