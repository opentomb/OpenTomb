#pragma once

#include "world/core/boundingbox.h"

#include <btBulletCollisionCommon.h>

#include <array>
#include <vector>
#include <memory>


namespace world
{
struct Character;

namespace core
{
struct SkeletalModel;
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

/*
 * base frame of animated skeletal model
 */
struct BoneFrame
{
    uint16_t            command;                                                // & 0x01 - move need, &0x02 - 180 rotate need
    std::vector<BoneTag> bone_tags;                                              // bones data
    btVector3 position;                                                 // position (base offset)
    core::BoundingBox boundingBox;
    btVector3 centre;                                              // bounding box centre
    btVector3 move;                                                // move command data
    btScalar            v_Vertical;                                             // jump command data
    btScalar            v_Horizontal;                                           // jump command data
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

struct SSAnimation
{
    int16_t                     last_state = 0;
    int16_t                     next_state = 0;
    int16_t                     last_animation = 0;
    int16_t                     current_animation = 0;                              //
    int16_t                     next_animation = 0;                                 //
    //! @todo Many comparisons with unsigned, so check if it can be made unsigned.
    int16_t                     current_frame = 0;                                  //
    int16_t                     next_frame = 0;                                     //

    uint16_t                    anim_flags = 0;                                     // additional animation control param

    btScalar                    period = 1.0f / 30;                                 // one frame change period
    btScalar                    frame_time = 0;                                     // current time
    btScalar                    lerp = 0;

    void (*onFrame)(Character* ent, SSAnimation *ss_anim, AnimUpdate state);

    core::SkeletalModel    *model = nullptr;                                          // pointer to the base model
    SSAnimation      *next = nullptr;
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
    btVector3 centre;                                      // bounding box centre

    SSAnimation       animations;                                     // animations list

    bool hasSkin;                                       // whether any skinned meshes need rendering

    void fromModel(core::SkeletalModel* model);
};

void BoneFrame_Copy(BoneFrame* dst, BoneFrame* src);

} // namespace animation
} // namespace world
