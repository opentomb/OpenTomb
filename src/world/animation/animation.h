#pragma once

#include "animcommands.h"
#include "util/helpers.h"
#include "world/core/boundingbox.h"
#include "world/statecontroller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <boost/assert.hpp>

#include <array>
#include <list>
#include <memory>
#include <vector>

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
constexpr float FrameRate = 30;
constexpr util::Duration FrameTime = util::fromSeconds(1.0/FrameRate);

// This is the global game logic refresh interval (physics timestep)
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.
// This should be a multiple of animation::FrameRate (1/30,60,90,120,...)
constexpr float GameLogicFrameRate = 2*FrameRate;
constexpr util::Duration GameLogicFrameTime = util::fromSeconds(1.0/GameLogicFrameRate);

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
    glm::float_t    mat[4];
    glm::float_t    move[2];
    uint16_t    tex_ind;
};

struct AnimSeq
{
    bool        uvrotate;               // UVRotate mode flag.
    bool        frame_lock;             // Single frame mode. Needed for TR4-5 compatible UVRotate.

    bool        blend;                  // Blend flag.  Reserved for future use!
    glm::float_t    blend_rate;             // Blend rate.  Reserved for future use!
    glm::float_t    blend_time;             // Blend value. Reserved for future use!

    AnimTextureType anim_type;
    bool        reverse_direction;      // Used only with type 2 to identify current animation direction.
    util::Duration frame_time;             // Time passed since last frame update.
    uint16_t    current_frame;          // Current frame for this sequence.
    util::Duration frame_rate;             // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    glm::float_t    uvrotate_speed;         // Speed of UVRotation, in seconds.
    glm::float_t    uvrotate_max;           // Reference value used to restart rotation.
    glm::float_t    current_uvrotate;       // Current coordinate window position.

    std::vector<TexFrame> frames;
    std::vector<uint32_t> frame_list;   // Offset into anim textures frame list.
};

/*
 * animation switching control structure
 */
struct AnimDispatch
{
    uint16_t    next_anim;  //!< "switch to" animation
    uint16_t    next_frame; //!< "switch to" frame
    uint16_t    frame_low;  //!< low border of state change condition
    uint16_t    frame_high; //!< high border of state change condition
};

struct StateChange
{
    LaraState                 id;
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
    glm::vec3            position;
    core::BoundingBox    boundingBox;

    std::vector<AnimCommand> animCommands;          // cmds for end-of-anim
};

struct AnimationFrame
{
    uint32_t                    id;
    uint8_t                     original_frame_rate;
    int32_t                     speed_x;                // Forward-backward speed
    int32_t                     accel_x;                // Forward-backward accel
    int32_t                     speed_y;                // Left-right speed
    int32_t                     accel_y;                // Left-right accel
    uint32_t                    anim_command;
    uint32_t                    num_anim_commands;
    LaraState                   state_id;
    std::vector<SkeletonKeyFrame> keyFrames;

    std::vector<StateChange> stateChanges;           // Animation statechanges data

    AnimationFrame   *next_anim;              // Next default animation
    int                         next_frame;             // Next default frame

    std::vector<AnimCommand> animCommands; // cmds for end-of-anim

    const StateChange* findStateChangeByAnim(int state_change_anim) const noexcept
    {
        if(state_change_anim < 0)
            return nullptr;

        for(const StateChange& stateChange : stateChanges)
        {
            for(const AnimDispatch& dispatch : stateChange.anim_dispatch)
            {
                if(dispatch.next_anim == state_change_anim)
                {
                    return &stateChange;
                }
            }
        }

        return nullptr;
    }

    const StateChange* findStateChangeByID(LaraState id) const noexcept
    {
        for(const StateChange& stateChange : stateChanges)
        {
            if(stateChange.id == id)
            {
                return &stateChange;
            }
        }

        return nullptr;
    }
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
    Bone *parent;
    uint16_t index;
    std::shared_ptr<core::BaseMesh> mesh; //!< The mesh this bone deforms
    std::shared_ptr<core::BaseMesh> mesh_skin;
    std::shared_ptr<core::BaseMesh> mesh_slot; //!< Optional additional mesh
    glm::vec3 offset;

    glm::quat qrotate;
    glm::mat4 transform; //!< Local transformation matrix
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
    glm::vec3 m_position = {0,0,0};
    core::BoundingBox m_boundingBox{};

    bool m_hasSkin = false; //!< whether any skinned meshes need rendering

    SkeletalModel* m_model = nullptr;
    util::Duration m_frameTime{0}; //!< time in current frame

    glm::float_t m_frameLerp = 0; //!< Bias for mixing previous and current key frames

    int16_t m_previousAnimation = 0;
    uint16_t m_currentAnimation = 0;

    int16_t m_previousFrame = 0;
    int16_t m_currentFrame = 0; //! @todo Many comparisons with unsigned, so check if it can be made unsigned.

    LaraState m_previousState = LaraState::WalkForward;
    LaraState m_nextState = LaraState::WalkForward;

    AnimationMode m_mode = AnimationMode::NormalControl;

    btManifoldArray m_manifoldArray;

    bool m_hasGhosts = false;

public:
    void (*onFrame)(Character* ent, AnimUpdate state) = nullptr;

    const AnimationFrame& getCurrentAnimationFrame() const;
    AnimUpdate stepAnimation(util::Duration time, Entity *cmdEntity = nullptr);
    void setAnimation(int animation, int frame = 0);


    int16_t getCurrentAnimation() const noexcept
    {
        return m_currentAnimation;
    }
    void setCurrentAnimation(uint16_t value) noexcept
    {
        m_currentAnimation = value;
    }

    int16_t getLerpLastAnimation() const noexcept
    {
        return m_previousAnimation;
    }
    void setLerpLastAnimation(int16_t value) noexcept
    {
        m_previousAnimation = value;
    }

    int16_t getCurrentFrame() const noexcept
    {
        return m_currentFrame;
    }
    void setCurrentFrame(int16_t value) noexcept
    {
        m_currentFrame = value;
    }

    int16_t getLerpLastFrame() const noexcept
    {
        return m_previousFrame;
    }
    void setLerpLastFrame(int16_t value) noexcept
    {
        m_previousFrame = value;
    }

    util::Duration getFrameTime() const noexcept
    {
        return m_frameTime;
    }
    void setFrameTime(util::Duration time) noexcept
    {
        m_frameTime = time;
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

    LaraState getNextState() const noexcept
    {
        return m_nextState;
    }
    LaraState getLastState() const noexcept
    {
        return m_previousState;
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
        BOOST_ASSERT( !m_bones.empty() );
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

    void setNextState(LaraState state) noexcept
    {
        m_nextState = state;
    }

    void setLastState(LaraState state) noexcept
    {
        m_previousState = state;
    }

    void setLerp(glm::float_t lerp)
    {
        m_frameLerp = lerp;
    }
    glm::float_t getLerp() const noexcept
    {
        return m_frameLerp;
    }

    const btManifoldArray& getManifoldArray() const noexcept
    {
        return m_manifoldArray;
    }

    btManifoldArray& manifoldArray() noexcept
    {
        return m_manifoldArray;
    }

    void updateTransform(const glm::mat4 &entityTransform);

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
    btCollisionObject* getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t *curr_flag);
    void genRigidBody(Entity* entity, CollisionShape collisionShape, CollisionType collisionType, const glm::mat4& transform);
    void enableCollision();
    void disableCollision();
};

} // namespace animation
} // namespace world
