#pragma once

#include "world/core/boundingbox.h"
#include "world/statecontroller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>
#include <memory>
#include <vector>

namespace world
{
struct Character;
struct SkeletalModel;
struct Entity;

namespace core
{
struct BaseMesh;
} // namespace core

namespace animation
{

//! Default fixed TR framerate needed for animation calculation
constexpr float FrameRate = 30;

// This is the global game logic refresh interval (physics timestep)
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.
// This should be a multiple of animation::FrameRate (1/30,60,90,120,...)
constexpr float GameLogicFrameRate = 2*FrameRate;

enum class AnimUpdate
{
    None,
    NewFrame,
    NewAnim
};

struct AnimCommand
{
    int cmdId;
    int param[3];
};

/*
 * Animated version of vertex. Does not contain texture coordinate, because that is in a different VBO.
 */
struct AnimatedVertex
{
    glm::vec3 position;
    std::array<float, 4> color;
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

    AnimTextureType anim_type;          // 0 = normal, 1 = back, 2 = reverse.
    bool        reverse_direction;      // Used only with type 2 to identify current animation direction.
    glm::float_t    frame_time;             // Time passed since last frame update.
    uint16_t    current_frame;          // Current frame for this sequence.
    glm::float_t    frame_rate;             // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

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
    uint16_t    next_anim;                                                      // "switch to" animation
    uint16_t    next_frame;                                                     // "switch to" frame
    uint16_t    frame_low;                                                      // low border of state change condition
    uint16_t    frame_high;                                                     // high border of state change condition
};

struct StateChange
{
    LaraState                 id;
    std::vector<AnimDispatch> anim_dispatch;
};

struct BoneTag
{
    glm::vec3 offset;                                            // bone vector
    glm::quat qrotate;                                           // rotation quaternion
};

/*
 * base frame of animated skeletal model
 */
struct BoneFrame
{
    std::vector<BoneTag> bone_tags;                 // bones data
    glm::vec3            position;                       // position (base offset)
    core::BoundingBox    boundingBox;
    glm::vec3            center;                    // bounding box centre

    std::vector<AnimCommand> animCommands;          // cmds for end-of-anim
};

/*
 * one animation frame structure
 */
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
    std::vector<BoneFrame> frames;                 // Frame data

    std::vector<StateChange> stateChanges;           // Animation statechanges data

    AnimationFrame   *next_anim;              // Next default animation
    int                         next_frame;             // Next default frame

    std::vector<AnimCommand> animCommands; // cmds for end-of-anim

    const StateChange* findStateChangeByAnim(int state_change_anim) const
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

    const StateChange* findStateChangeByID(LaraState id) const
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

enum class SSAnimationMode
{
    NormalControl,
    LoopLastFrame,
    WeaponCompat,
    Locked
};

struct SSAnimation
{
    LaraState                   last_state = LaraState::WALK_FORWARD;
    LaraState                   next_state = LaraState::WALK_FORWARD;
    int16_t                     current_animation = 0;                              //
                                                                                    //! @todo Many comparisons with unsigned, so check if it can be made unsigned.
    int16_t                     current_frame = 0;                                  //

    SSAnimationMode mode = SSAnimationMode::NormalControl;

    glm::float_t                frame_time = 0;                                     // time in current frame

                                                                                    // lerp:
    glm::float_t                lerp = 0;
    int16_t                     lerp_last_animation = 0;
    int16_t                     lerp_last_frame = 0;

    void (*onFrame)(Character* ent, SSAnimation *ss_anim, AnimUpdate state) = nullptr;

    SkeletalModel    *model = nullptr;                                          // pointer to the base model
    SSAnimation      *next = nullptr;

    void setAnimation(int animation, int frame = 0, int another_model = -1);
    bool findStateChange(LaraState stateid, uint16_t& animid_out, uint16_t& frameid_inout);
    AnimUpdate stepAnimation(glm::float_t time, Entity *cmdEntity = nullptr);

    const AnimationFrame& getCurrentAnimationFrame() const;
};

struct SSBoneTag
{
    SSBoneTag   *parent;
    uint16_t                index;
    std::shared_ptr<core::BaseMesh> mesh_base;                                          // base mesh - pointer to the first mesh in array
    std::shared_ptr<core::BaseMesh> mesh_skin;                                          // base skinned mesh for ТР4+
    std::shared_ptr<core::BaseMesh> mesh_slot;
    glm::vec3 offset;                                          // model position offset

    glm::quat qrotate;                                         // quaternion rotation
    glm::mat4 transform;    // 4x4 OpenGL matrix for stack usage
    glm::mat4 full_transform;    // 4x4 OpenGL matrix for global usage

    uint32_t                body_part;                                          // flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...
};

/*
 * base frame of animated skeletal model
 */
struct SSBoneFrame
{
    std::vector<SSBoneTag> bone_tags;                                      // array of bones
    glm::vec3 position = {0,0,0};                                         // position (base offset)
    core::BoundingBox boundingBox;
    glm::vec3 center = {0,0,0};                                      // bounding box center

    SSAnimation       animations;                                     // animations list

    bool hasSkin;                                       // whether any skinned meshes need rendering

    void fromModel(SkeletalModel* model);

    /**
     * That function updates item animation and rebuilds skeletal matrices;
     * @param bf - extended bone frame of the item;
     */
    void itemFrame(glm::float_t time);

    void updateCurrentBoneFrame();
};

} // namespace animation
} // namespace world
