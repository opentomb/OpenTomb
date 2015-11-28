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

#include <array>
#include <memory>
#include <vector>
#include <iostream>

namespace world
{
struct Character;
struct SkeletalModel;
struct Entity;
struct RDSetup;
enum class CollisionShape;
enum class CollisionType;

namespace core
{
struct BaseMesh;
} // namespace core

namespace animation
{

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
enum class AnimTextureType
{
    Forward,
    Backward,
    Reverse
};

/*
 *  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
 */
struct TexFrame
{
    glm::float_t mat[4];
    glm::float_t move[2];
    uint16_t tex_ind;
};

struct AnimSeq
{
    bool uvrotate = false;   // UVRotate mode flag.
    bool frame_lock = false; // Single frame mode. Needed for TR4-5 compatible UVRotate.

    bool blend;              // Blend flag.  Reserved for future use!
    glm::float_t blend_rate; // Blend rate.  Reserved for future use!
    glm::float_t blend_time; // Blend value. Reserved for future use!

    AnimTextureType anim_type = AnimTextureType::Forward;
    bool reverse_direction = false;    // Used only with type 2 to identify current animation direction.
    util::Duration frame_time = util::Duration::zero(); // Time passed since last frame update.
    uint16_t current_frame = 0;    // Current frame for this sequence.
    util::Duration frame_duration = util::MilliSeconds(50); // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    glm::float_t uvrotate_speed;   // Speed of UVRotation, in seconds.
    glm::float_t uvrotate_max;     // Reference value used to restart rotation.
    glm::float_t current_uvrotate; // Current coordinate window position.

    std::vector<TexFrame> frames;
    std::vector<uint32_t> frame_list; // Offset into anim textures frame list.
};

struct AnimationState
{
    uint16_t animation = 0;
    uint16_t frame = 0;
    LaraState state = LaraState::WalkForward;
};

/*
 * animation switching control structure
 */
struct AnimDispatch
{
    AnimationState next;  //!< "switch to" animation
    uint16_t frame_low;  //!< low border of state change condition
    uint16_t frame_high; //!< high border of state change condition
};

struct StateChange
{
    LaraState id;
    std::vector<AnimDispatch> anim_dispatch;
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
    glm::vec3 position = {0,0,0};
    core::BoundingBox boundingBox;
};

/**
 * A sequence of keyframes.
 */
struct Animation
{
    uint32_t id;
    int32_t speed_x; // Forward-backward speed
    int32_t accel_x; // Forward-backward accel
    int32_t speed_y; // Left-right speed
    int32_t accel_y; // Left-right accel
    uint32_t anim_command;
    uint32_t num_anim_commands;
    LaraState state_id;

    boost::container::flat_map<LaraState, StateChange> stateChanges;

    Animation* next_anim = nullptr; // Next default animation
    int next_frame;                 // Next default frame

    std::vector<AnimCommand> finalAnimCommands; // cmds for end-of-anim

    const StateChange* findStateChangeByAnim(int nextAnimId) const noexcept
    {
        if(nextAnimId < 0)
            return nullptr;

        for(const auto& stateChange : stateChanges)
        {
            for(const AnimDispatch& dispatch : stateChange.second.anim_dispatch)
            {
                if(dispatch.next.animation == nextAnimId)
                {
                    return &stateChange.second;
                }
            }
        }

        return nullptr;
    }

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
        const size_t frameIndex = frame/m_stretchFactor;
        BOOST_ASSERT(frameIndex < m_keyFrames.size());
        const SkeletonKeyFrame& first = m_keyFrames[frameIndex];
        const SkeletonKeyFrame& second = frameIndex+1 >= m_keyFrames.size()
                                       ? m_keyFrames.back()
                                       : m_keyFrames[frameIndex+1];

        if(first.boneKeyFrames.size() != second.boneKeyFrames.size())
            std::cerr << first.boneKeyFrames.size() << "," << second.boneKeyFrames.size() << "\n";

        BOOST_ASSERT(first.boneKeyFrames.size() == second.boneKeyFrames.size());

        const size_t subOffset = frame - frameIndex*m_stretchFactor; // offset between keyframes
        if( subOffset == 0 )
            return first; // no need to interpolate

        const glm::float_t lerp = static_cast<glm::float_t>(subOffset) / static_cast<glm::float_t>(m_stretchFactor);

        SkeletonKeyFrame result;
        result.position = glm::mix(first.position, second.position, lerp);
        result.boundingBox.max = glm::mix(first.boundingBox.max, second.boundingBox.max, lerp);
        result.boundingBox.min = glm::mix(first.boundingBox.min, second.boundingBox.min, lerp);

        result.boneKeyFrames.resize( first.boneKeyFrames.size() );

        for(size_t k = 0; k < first.boneKeyFrames.size(); k++)
        {
            result.boneKeyFrames[k].offset = glm::mix(first.boneKeyFrames[k].offset, second.boneKeyFrames[k].offset, lerp);
            result.boneKeyFrames[k].qrotate = glm::slerp(first.boneKeyFrames[k].qrotate, second.boneKeyFrames[k].qrotate, lerp);
        }

        return result;
    }

    SkeletonKeyFrame& rawKeyFrame(size_t idx)
    {
        if( idx >= m_keyFrames.size() )
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

/**
 * @brief A single bone in a @c Skeleton
 */
struct Bone
{
    Bone* parent;
    uint16_t index;
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

    ~Bone();
};

class Skeleton
{
    std::vector<Bone> m_bones{};
    glm::vec3 m_position = {0, 0, 0};
    core::BoundingBox m_boundingBox{};

    bool m_hasSkin = false; //!< whether any skinned meshes need rendering

    SkeletalModel* m_model = nullptr;

    AnimationState m_previousAnimation;
    AnimationState m_currentAnimation;

    AnimationMode m_mode = AnimationMode::NormalControl;

    btManifoldArray m_manifoldArray;

    bool m_hasGhosts = false;

    util::Duration m_animationTime = util::Duration::zero();

  public:
    void (*onFrame)(Character* ent, AnimUpdate state) = nullptr;

    const Animation& getCurrentAnimationFrame() const;
    AnimUpdate stepAnimation(util::Duration time, Entity* cmdEntity = nullptr);
    void setAnimation(int animation, int frame = 0);

    uint16_t getCurrentAnimation() const noexcept
    {
        return m_currentAnimation.animation;
    }
    void setCurrentAnimation(uint16_t value) noexcept
    {
        m_currentAnimation.animation = value;
    }

    uint16_t getPreviousAnimation() const noexcept
    {
        return m_previousAnimation.animation;
    }
    void setPreviousAnimation(uint16_t value) noexcept
    {
        m_previousAnimation.animation = value;
    }

    uint16_t getCurrentFrame() const noexcept
    {
        return m_currentAnimation.frame;
    }
    void setCurrentFrame(uint16_t value) noexcept
    {
        m_currentAnimation.frame = value;
    }

    uint16_t getPreviousFrame() const noexcept
    {
        return m_previousAnimation.frame;
    }
    void setPreviousFrame(uint16_t value) noexcept
    {
        m_previousAnimation.frame = value;
    }

    const SkeletalModel* getModel() const noexcept
    {
        return m_model;
    }
    SkeletalModel* model() const noexcept
    {
        return m_model;
    }
    void setModel(SkeletalModel* model) noexcept
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

    void fromModel(SkeletalModel* m_model);

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

    void copyMeshBinding(const SkeletalModel* model, bool resetMeshSlot = false);

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

    void createGhosts(Entity* entity, const glm::mat4& transform);

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

    void updateCurrentCollisions(const Entity* entity, const glm::mat4& transform);
    bool createRagdoll(const RDSetup& setup);
    void initCollisions(const glm::vec3& speed);
    void updateRigidBody(const glm::mat4& transform);
    btCollisionObject* getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t* curr_flag);
    void genRigidBody(Entity* entity, CollisionShape collisionShape, CollisionType collisionType, const glm::mat4& transform);
    void enableCollision();
    void disableCollision();
};

} // namespace animation
} // namespace world
