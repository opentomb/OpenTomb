#pragma once

#include "world/core/boundingbox.h"

#include <btBulletCollisionCommon.h>

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
    btVector3 position;
    std::array<float, 4> color;
    btVector3 normal;
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
    btScalar    mat[4];
    btScalar    move[2];
    uint16_t    tex_ind;
};

struct AnimSeq
{
    bool        uvrotate;               // UVRotate mode flag.
    bool        frame_lock;             // Single frame mode. Needed for TR4-5 compatible UVRotate.

    bool        blend;                  // Blend flag.  Reserved for future use!
    btScalar    blend_rate;             // Blend rate.  Reserved for future use!
    btScalar    blend_time;             // Blend value. Reserved for future use!

    AnimTextureType anim_type;          // 0 = normal, 1 = back, 2 = reverse.
    bool        reverse_direction;      // Used only with type 2 to identify current animation direction.
    btScalar    frame_time;             // Time passed since last frame update.
    uint16_t    current_frame;          // Current frame for this sequence.
    btScalar    frame_rate;             // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    btScalar    uvrotate_speed;         // Speed of UVRotation, in seconds.
    btScalar    uvrotate_max;           // Reference value used to restart rotation.
    btScalar    current_uvrotate;       // Current coordinate window position.

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
    uint32_t                    id;
    std::vector<AnimDispatch> anim_dispatch;
};

struct BoneTag
{
    btVector3 offset;                                            // bone vector
    btQuaternion qrotate;                                           // rotation quaternion
};

#define ANIM_CMD_MOVE               0x01
#define ANIM_CMD_CHANGE_DIRECTION   0x02
#define ANIM_CMD_JUMP               0x04

/*
 * base frame of animated skeletal model
 */
struct BoneFrame
{
    std::vector<BoneTag> bone_tags;                 // bones data
    btVector3            position;                       // position (base offset)
    core::BoundingBox    boundingBox;
    btVector3            center;                    // bounding box centre

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
    uint16_t                    state_id;
    std::vector<BoneFrame> frames;                 // Frame data

    std::vector<StateChange> stateChanges;           // Animation statechanges data

    AnimationFrame   *next_anim;              // Next default animation
    int                         next_frame;             // Next default frame

    std::vector<AnimCommand> animCommands; // cmds for end-of-anim

    StateChange* findStateChangeByAnim(int state_change_anim)
    {
        if(state_change_anim < 0)
            return nullptr;

        for(StateChange& stateChange : stateChanges)
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

    StateChange* findStateChangeByID(uint32_t id)
    {
        for(StateChange& stateChange : stateChanges)
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
    int16_t                     last_state = 0;
    int16_t                     next_state = 0;
    int16_t                     current_animation = 0;                              //
                                                                                    //! @todo Many comparisons with unsigned, so check if it can be made unsigned.
    int16_t                     current_frame = 0;                                  //

    SSAnimationMode mode = SSAnimationMode::NormalControl;

    btScalar                    period = 1.0f / 30.0f;                              // one frame change period
    btScalar                    frame_time = 0;                                     // time in current frame

                                                                                    // lerp:
    btScalar                    lerp = 0;
    int16_t                     lerp_last_animation = 0;
    int16_t                     lerp_last_frame = 0;

    void(*onFrame)(Character* ent, SSAnimation *ss_anim, AnimUpdate state);

    SkeletalModel    *model = nullptr;                                          // pointer to the base model
    SSAnimation      *next = nullptr;

    void setAnimation(int animation, int frame = 0, int another_model = -1);
    bool findStateChange(uint32_t stateid, uint16_t& animid_out, uint16_t& frameid_inout);
    AnimUpdate stepAnimation(btScalar time, Entity *cmdEntity = nullptr);
};

struct SSBoneTag
{
    SSBoneTag   *parent;
    uint16_t                index;
    std::shared_ptr<core::BaseMesh> mesh_base;                                          // base mesh - pointer to the first mesh in array
    std::shared_ptr<core::BaseMesh> mesh_skin;                                          // base skinned mesh for ТР4+
    std::shared_ptr<core::BaseMesh> mesh_slot;
    btVector3 offset;                                          // model position offset

    btQuaternion qrotate;                                         // quaternion rotation
    btTransform transform;    // 4x4 OpenGL matrix for stack usage
    btTransform full_transform;    // 4x4 OpenGL matrix for global usage

    uint32_t                body_part;                                          // flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...
};

/*
 * base frame of animated skeletal model
 */
struct SSBoneFrame
{
    std::vector<SSBoneTag> bone_tags;                                      // array of bones
    btVector3 position;                                         // position (base offset)
    core::BoundingBox boundingBox;
    btVector3 center;                                      // bounding box center

    SSAnimation       animations;                                     // animations list

    bool hasSkin;                                       // whether any skinned meshes need rendering

    void fromModel(SkeletalModel* model);

    /**
     * That function updates item animation and rebuilds skeletal matrices;
     * @param bf - extended bone frame of the item;
     */
    void itemFrame(btScalar time);
};

void BoneFrame_Copy(BoneFrame* dst, BoneFrame* src);

} // namespace animation
} // namespace world
