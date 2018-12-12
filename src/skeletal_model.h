
#ifndef SKELETAL_MODEL_H
#define SKELETAL_MODEL_H

#ifdef	__cplusplus
extern "C" {
#endif

#define ANIM_CMD_MOVE               0x01
#define ANIM_CMD_CHANGE_DIRECTION   0x02
#define ANIM_CMD_JUMP               0x04

/*
 * Animation control flags
 */
#define ANIM_NORMAL_CONTROL             (0)
#define ANIM_LOOP_LAST_FRAME            (1)
#define ANIM_FRAME_LOCK                 (2)
#define ANIM_FRAME_REVERSE              (4)

// changing info:
#define SS_CHANGING_NONE        (0x00)      // 0x00 - no changes; 
#define SS_CHANGING_FRAME       (0x01)      // 0x01 - next frame, same anim;
#define SS_CHANGING_END_ANIM    (0x02)      // 0x02 - next frame, next anim (anim ended, may be loop);
#define SS_CHANGING_BY_STATE    (0x03)      // 0x03 - new frame, new anim (by state change info);
#define SS_CHANGING_HEAVY       (0x04)      // 0x04 - rough change by set animation;

    
#define ANIM_TYPE_BASE                  (0x0000)
#define ANIM_TYPE_WEAPON_LH             (0x0002)
#define ANIM_TYPE_WEAPON_RH             (0x0003)
#define ANIM_TYPE_WEAPON_TH             (0x0004)
#define ANIM_TYPE_MISK_1                (0x0100)
#define ANIM_TYPE_MISK_2                (0x0101)
#define ANIM_TYPE_MISK_3                (0x0102)
#define ANIM_TYPE_MISK_4                (0x0103)

#include <stdint.h>

#include "core/base_types.h"
    
struct base_mesh_s;

/*
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */

/*
 * SMOOTHED ANIMATIONS STRUCTURES
 * stack matrices are needed for skinned mesh transformations.
 */
typedef struct ss_bone_tag_s
{
    struct ss_bone_tag_s   *parent;
    uint32_t                body_part;                                          // flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...
    uint16_t                index;
    uint16_t                is_hidden : 1;
    uint16_t                is_targeted : 1;
    uint16_t                is_axis_modded : 1;
    struct
    {
        float               bone_local_direction[3];
        float               target_pos[3];
        float               limit[4];                                           // x, y, z, cos(alpha_limit)
        float               current_q[4];
        float               axis_mod[3];
        float               current_slerp;
    }                       mod;
    struct base_mesh_s     *mesh_base;                                          // base mesh - pointer to the first mesh in array
    struct base_mesh_s     *mesh_replace;
    struct base_mesh_s     *mesh_skin;                                          // base skinned mesh for ТР4+
    struct base_mesh_s     *mesh_slot;
    struct ss_animation_s  *alt_anim;
    uint32_t               *skin_map;                                           // vertices map for skin mesh
    float                   offset[3];                                          // model position offset

    float                   qrotate[4];                                         // quaternion rotation
    
#ifdef __GNUC__
    float                   local_transform[16] __attribute__((packed, aligned(16)));       // 4x4 OpenGL matrix for stack usage
    float                   current_transform[16]  __attribute__((packed, aligned(16)));    // 4x4 OpenGL matrix for global usage
#else
    float                   local_transform[16];       // 4x4 OpenGL matrix for stack usage
    float                   current_transform[16];    // 4x4 OpenGL matrix for global usage
#endif
}ss_bone_tag_t, *ss_bone_tag_p;

typedef struct bone_tag_s
{
    float               offset[3];                                              // bone vector
    float               qrotate[4];                                             // rotation quaternion
}bone_tag_t, *bone_tag_p;

typedef struct bone_frame_s
{
    uint16_t            bone_tag_count;                                         // number of bones
    uint16_t            unused;                                                
    struct bone_tag_s  *bone_tags;                                              // bones data
    float               pos[3];                                                 // position (base offset)
    float               bb_min[3];                                              // bounding box min coordinates
    float               bb_max[3];                                              // bounding box max coordinates
    float               centre[3];                                              // bounding box centre
}bone_frame_t, *bone_frame_p ;

typedef struct ss_animation_s
{
    uint16_t                    type;
    uint16_t                    enabled : 1;
    uint16_t                    heavy_state : 1;
    uint16_t                    frame_changing_state : 14;
    int16_t                     target_state;
    int16_t                     prev_animation;
    int16_t                     prev_frame;
    int16_t                     current_animation;
    int16_t                     current_frame;
    
    struct bone_frame_s         current_bf;
    struct bone_frame_s         prev_bf;
    
    uint16_t                    anim_frame_flags;                               // base animation control flags
    uint16_t                    anim_ext_flags;                                 // additional animation control flags

    float                       period;                                         // one frame change period
    float                       frame_time;                                     // current time
    float                       lerp;

    int                       (*onFrame)(struct entity_s *ent, struct ss_animation_s *ss_anim, float time);
    void                      (*onEndFrame)(struct entity_s *ent, struct ss_animation_s *ss_anim);
    struct skeletal_model_s    *model;                                          // pointer to the base model
    struct ss_animation_s      *next;
    struct ss_animation_s      *prev;
}ss_animation_t, *ss_animation_p;

/*
 * base frame of animated skeletal model
 */
typedef struct ss_bone_frame_s
{
    uint16_t                    bone_tag_count;                                 // number of bones
    uint16_t                    flags;    
    struct ss_bone_tag_s       *bone_tags;                                      // array of bones
    float                       pos[3];                                         // position (base offset)
    float                       bb_min[3];                                      // bounding box min coordinates
    float                       bb_max[3];                                      // bounding box max coordinates
    float                       centre[3];                                      // bounding box centre
    struct engine_transform_s  *transform;

    struct ss_animation_s       animations;                                     // animations list
}ss_bone_frame_t, *ss_bone_frame_p;

/*
 * mesh tree base element structure
 */
typedef struct mesh_tree_tag_s
{
    struct base_mesh_s         *mesh_base;                                      // base mesh - pointer to the first mesh in array
    float                       offset[3];                                      // model position offset
    uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = POP + PUSH
    uint16_t                    parent;                                         // parent index, can not be invalid
    uint32_t                    body_part;
    uint16_t                    replace_mesh;                                   // flag for shoot / guns animations (0x00, 0x01)
    uint16_t                    replace_anim;
}mesh_tree_tag_t, *mesh_tree_tag_p;

/*
 * animation switching control structure
 */
typedef struct anim_dispatch_s
{
    uint16_t    next_anim;                                                      // "switch to" animation
    uint16_t    next_frame;                                                     // "switch to" frame
    uint16_t    frame_low;                                                      // low border of state change condition
    uint16_t    frame_high;                                                     // high border of state change condition
}anim_dispatch_t, *anim_dispatch_p;

typedef struct state_change_s
{
    uint32_t                    id;
    uint16_t                    anim_dispatch_count;
    struct anim_dispatch_s     *anim_dispatch;
}state_change_t, *state_change_p;

typedef struct animation_command_s
{
    uint16_t                    id;
    uint16_t                    extra;
    int16_t                     frame;
    int16_t                     effect;
    float                       data[3];
    struct animation_command_s *next;
}animation_command_t, *animation_command_p;

/*
 * one animation frame structure
 */
typedef struct animation_frame_s
{
    uint32_t                    id;
    uint16_t                    state_id;
    uint16_t                    max_frame;
    uint16_t                    frames_count;           // Number of frames
    uint16_t                    state_change_count;     // Number of animation statechanges
    struct bone_frame_s        *frames;                 // Frame data
    struct state_change_s      *state_change;           // Animation statechanges data
    struct animation_command_s *commands;
    
    float                       speed_x;                // Forward-backward speed
    float                       accel_x;                // Forward-backward accel
    float                       speed_y;                // Left-right speed
    float                       accel_y;                // Left-right accel

    struct animation_frame_s   *next_anim;              // Next default animation
    int32_t                     next_frame;             // Next default frame
}animation_frame_t, *animation_frame_p;

/*
 * skeletal model with animations data.
 */

typedef struct skeletal_model_s
{
    uint32_t                    id;                                             // ID
    uint8_t                     transparency_flags;                             // transparancy flags; 0 - opaque; 1 - alpha test; other - blending mode
    uint8_t                     hide;                                           // do not render
    float                       bbox_min[3];                                    // bbox info
    float                       bbox_max[3];
    float                       centre[3];                                      // the centre of model

    uint16_t                    animation_count;                                // number of animations
    struct animation_frame_s   *animations;                                     // animations data

    uint16_t                    mesh_count;                                     // number of model meshes
    struct mesh_tree_tag_s     *mesh_tree;                                      // base mesh tree.
    uint16_t                   *collision_map;
}skeletal_model_t, *skeletal_model_p;


void SkeletalModel_Clear(skeletal_model_p model);
void SkeletalModel_GenParentsIndexes(skeletal_model_p model);

void SkeletalModel_FillTransparency(skeletal_model_p model);
void SkeletalModel_CopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count);
void SkeletalModel_CopyAnims(skeletal_model_p dst, skeletal_model_p src);
void BoneFrame_Copy(bone_frame_p dst, const bone_frame_p src);

void SSBoneFrame_CreateFromModel(ss_bone_frame_p bf, skeletal_model_p model);
void SSBoneFrame_Clear(ss_bone_frame_p bf);
void SSBoneFrame_Copy(struct ss_bone_frame_s *dst, struct ss_bone_frame_s *src);
void SSBoneFrame_UpdateMoveCommand(struct ss_animation_s *ss_anim, float move[3]);
void SSBoneFrame_UpdateChangeDirCommand(struct ss_bone_frame_s *bf);
void SSBoneFrame_Update(struct ss_bone_frame_s *bf, float time);
void SSBoneFrame_RotateBone(struct ss_bone_frame_s *bf, const float q_rotate[4], int bone);
int  SSBoneFrame_CheckTargetBoneLimit(struct ss_bone_frame_s *bf, struct ss_bone_tag_s *b_tag, float target[3]);
void SSBoneFrame_TargetBoneToSlerp(struct ss_bone_frame_s *bf, struct ss_bone_tag_s *b_tag, float time);
void SSBoneFrame_SetTarget(struct ss_bone_tag_s *b_tag, const float target_pos[3], const float bone_dir[3]);
void SSBoneFrame_SetTargetingAxisMod(struct ss_bone_tag_s *b_tag, const float mod[3]);
void SSBoneFrame_SetTargetingLimit(struct ss_bone_tag_s *b_tag, const float limit[4]);
struct ss_animation_s *SSBoneFrame_AddOverrideAnim(struct ss_bone_frame_s *bf, struct skeletal_model_s *sm, uint16_t anim_type_id);
struct ss_animation_s *SSBoneFrame_GetOverrideAnim(struct ss_bone_frame_s *bf, uint16_t anim_type);
void SSBoneFrame_EnableOverrideAnimByType(struct ss_bone_frame_s *bf, uint16_t anim_type);
void SSBoneFrame_EnableOverrideAnim(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim);
void SSBoneFrame_DisableOverrideAnimByType(struct ss_bone_frame_s *bf, uint16_t anim_type);
void SSBoneFrame_DisableOverrideAnim(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim);
void SSBoneFrame_FillSkinnedMeshMap(ss_bone_frame_p model);

void Anim_AddCommand(struct animation_frame_s *anim, const animation_command_p command);
struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, uint32_t id);
int  Anim_GetAnimDispatchCase(struct ss_animation_s *ss_anim, uint32_t id);
void Anim_SetAnimation(struct ss_animation_s *ss_anim, int animation, int frame);

int  Anim_SetNextFrame(struct ss_animation_s *ss_anim, float time);
int  Anim_IncTime(struct ss_animation_s *ss_anim, float time);
inline uint16_t Anim_GetCurrentState(struct ss_animation_s *ss_anim)
{
    return ss_anim->model->animations[ss_anim->current_animation].state_id;
}

#ifdef	__cplusplus
}
#endif

#endif //SKELETAL_MODEL_H
