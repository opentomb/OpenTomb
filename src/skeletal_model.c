
#include <stdlib.h>
#include <memory.h>

#include "core/system.h"
#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "mesh.h"
#include "skeletal_model.h"


void SSBoneFrame_InitSSAnim(struct ss_animation_s *ss_anim, uint32_t anim_type_id);
void Anim_Clear(struct animation_frame_s *anim);


void SkeletalModel_Clear(skeletal_model_p model)
{
    if(model != NULL)
    {
        if(model->mesh_tree)
        {
            model->mesh_count = 0;
            free(model->mesh_tree);
            model->mesh_tree = NULL;
        }

        if(model->collision_map)
        {
            free(model->collision_map);
            model->collision_map = NULL;
        }

        if(model->animation_count)
        {
            for(uint16_t i = 0; i < model->animation_count; i++)
            {
                Anim_Clear(model->animations + i);
            }
            model->animation_count= 0;
            free(model->animations);
            model->animations = NULL;
        }
    }
}


void SkeletalModel_GenParentsIndexes(skeletal_model_p model)
{
	int stack = 0;
#ifdef _WIN32
	uint16_t* parents = malloc(sizeof(model->mesh_count));
#elif __linux__
	uint16_t parents[model->mesh_count];
#endif

    parents[0] = 0;
    model->mesh_tree[0].parent = 0;      // root

    for(uint16_t i = 1; i < model->mesh_count; i++)
    {
        model->mesh_tree[i].parent = i - 1;

        if(model->mesh_tree[i].flag & 0x01)                                     // POP
        {
            if(stack > 0)
            {
                model->mesh_tree[i].parent = parents[stack];
                stack--;
            }
        }
        if(model->mesh_tree[i].flag & 0x02)                                     // PUSH
        {
            if(stack + 1 < (uint16_t) model->mesh_count)
            {
                stack++;
                parents[stack] = model->mesh_tree[i].parent;
            }
        }
    }
    
    for(uint16_t i = 1; i + 1 < model->mesh_count; i++)
    {
        for(uint16_t j = 1; j + i < model->mesh_count; j++)
        {
            uint16_t m1 = model->collision_map[j];
            uint16_t m2 = model->collision_map[j + 1];

            if(model->mesh_tree[m1].parent > model->mesh_tree[m2].parent)
            {
                uint16_t t = model->collision_map[j];
                model->collision_map[j] = model->collision_map[j + 1];
                model->collision_map[j + 1] = t;
            }
        }
    }

#ifdef _WIN32
	free(parents);
#endif
}


void SkeletalModel_FillTransparency(skeletal_model_p model)
{
    model->transparency_flags = MESH_FULL_OPAQUE;
    for(uint16_t i = 0; i < model->mesh_count; i++)
    {
        if(model->mesh_tree[i].mesh_base->transparency_polygons != NULL)
        {
            model->transparency_flags = MESH_HAS_TRANSPARENCY;
            return;
        }
    }
}


void SkeletalModel_CopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}


void SkeletalModel_CopyAnims(skeletal_model_p dst, skeletal_model_p src)
{
    animation_frame_p new_anims = (animation_frame_p)calloc(src->animation_count, sizeof(animation_frame_t));
    animation_frame_p dst_a = new_anims;
    animation_frame_p src_a = src->animations;
    
    for(uint16_t i = 0; i < src->animation_count; ++i, ++dst_a, ++src_a)
    {
        animation_command_p *last_cmd = &dst_a->commands;
        animation_effect_p *last_effect = &dst_a->effects;
        
        for(animation_command_p cmd = src_a->commands; cmd; cmd = cmd->next)
        {
            *last_cmd = (animation_command_p)malloc(sizeof(animation_command_t));
            **last_cmd = *cmd;
            (*last_cmd)->next = NULL;
            last_cmd = &((*last_cmd)->next);
        }

        for(animation_effect_p effect = src_a->effects; effect; effect = effect->next)
        {
            *last_effect = (animation_effect_p)malloc(sizeof(animation_effect_t));
            **last_effect = *effect;
            (*last_effect)->next = NULL;
            last_effect = &((*last_effect)->next);
        }

        dst_a->frames_count = src_a->frames_count;
        dst_a->frames = (bone_frame_p)calloc(src_a->frames_count, sizeof(bone_frame_t));
        for(uint16_t i = 0; i < src_a->frames_count; ++i)
        {
            size_t sz = src_a->frames[i].bone_tag_count * sizeof(bone_tag_t);
            dst_a->frames[i] = src_a->frames[i];
            dst_a->frames[i].bone_tags = (bone_tag_p)malloc(sz);
            memcpy(dst_a->frames[i].bone_tags, src_a->frames[i].bone_tags, sz);
        }
        
        dst_a->state_change_count = src_a->state_change_count;
        dst_a->state_change = (state_change_p)calloc(src_a->state_change_count, sizeof(state_change_t));
        for(uint16_t i = 0; i < src_a->state_change_count; ++i)
        {
            size_t sz = src_a->state_change[i].anim_dispatch_count * sizeof(anim_dispatch_t);
            dst_a->state_change[i] = src_a->state_change[i];
            dst_a->state_change[i].anim_dispatch = (anim_dispatch_p)malloc(sz);
            memcpy(dst_a->state_change[i].anim_dispatch, src_a->state_change[i].anim_dispatch, sz);
        }
        
        dst_a->id = src_a->id;
        dst_a->state_id = src_a->state_id;
        dst_a->max_frame = src_a->max_frame;
        dst_a->frames_count = src_a->frames_count;
        dst_a->state_change_count = src_a->state_change_count;

        dst_a->speed_x = src_a->speed_x;
        dst_a->accel_x = src_a->accel_x;
        dst_a->speed_y = src_a->speed_y;
        dst_a->accel_y = src_a->accel_y;

        dst_a->next_anim = new_anims + src_a->next_anim->id;
        dst_a->next_frame = src_a->next_frame;
    }
    
    for(uint16_t i = 0; i < dst->animation_count; ++i)
    {
        Anim_Clear(dst->animations + i);
    }
    free(dst->animations);
    dst->animations = new_anims;
    dst->animation_count = src->animation_count;
}


void BoneFrame_Copy(bone_frame_p dst, bone_frame_p src)
{
    if(dst->bone_tag_count < src->bone_tag_count)
    {
        dst->bone_tags = (bone_tag_p)realloc(dst->bone_tags, src->bone_tag_count * sizeof(bone_tag_t));
    }
    dst->bone_tag_count = src->bone_tag_count;
    vec3_copy(dst->pos, src->pos);
    vec3_copy(dst->centre, src->centre);
    vec3_copy(dst->bb_max, src->bb_max);
    vec3_copy(dst->bb_min, src->bb_min);

    for(uint16_t i = 0; i < dst->bone_tag_count; i++)
    {
        vec4_copy(dst->bone_tags[i].qrotate, src->bone_tags[i].qrotate);
        vec3_copy(dst->bone_tags[i].offset, src->bone_tags[i].offset);
    }
}


void SSBoneFrame_CreateFromModel(ss_bone_frame_p bf, skeletal_model_p model)
{
    vec3_set_zero(bf->bb_min);
    vec3_set_zero(bf->bb_max);
    vec3_set_zero(bf->centre);
    vec3_set_zero(bf->pos);
    bf->transform = NULL;
    bf->flags = 0x0000;
    bf->bone_tag_count = 0;
    bf->bone_tags = NULL;
    
    SSBoneFrame_InitSSAnim(&bf->animations, ANIM_TYPE_BASE);
    bf->animations.model = model;
    if(model)
    {
        bf->bone_tag_count = model->mesh_count;
        bf->bone_tags = (ss_bone_tag_p)malloc(bf->bone_tag_count * sizeof(ss_bone_tag_t));
        bf->bone_tags[0].parent = NULL;                                         // root
        for(uint16_t i = 0; i < bf->bone_tag_count; i++)
        {
            ss_bone_tag_p b_tag = bf->bone_tags + i;
            b_tag->index = i;
            b_tag->is_hidden = 0x00;
            b_tag->is_targeted = 0x00;
            b_tag->is_axis_modded = 0x00;
            b_tag->mesh_base = model->mesh_tree[i].mesh_base;
            b_tag->mesh_replace = NULL;
            b_tag->mesh_skin = NULL;
            b_tag->mesh_slot = NULL;
            b_tag->skin_map = NULL;
            b_tag->alt_anim = NULL;
            b_tag->body_part = model->mesh_tree[i].body_part;

            vec3_copy(b_tag->offset, model->mesh_tree[i].offset);
            vec4_set_zero(b_tag->qrotate);
            Mat4_E_macro(b_tag->local_transform);
            Mat4_E_macro(b_tag->current_transform);

            vec3_set_zero(b_tag->mod.target_pos);
            vec4_set_zero_angle(b_tag->mod.current_q);
            b_tag->mod.bone_local_direction[0] = 0.0f;
            b_tag->mod.bone_local_direction[1] = 1.0f;
            b_tag->mod.bone_local_direction[2] = 0.0f;
            b_tag->mod.limit[0] = 0.0f;
            b_tag->mod.limit[1] = 1.0f;
            b_tag->mod.limit[2] = 0.0f;
            b_tag->mod.limit[3] =-1.0f;
            b_tag->mod.current_slerp = 1.0f;
            vec3_set_one(b_tag->mod.axis_mod);
            
            if(i > 0)
            {
                b_tag->parent = bf->bone_tags + model->mesh_tree[i].parent;
            }
        }
    }
}


void SSBoneFrame_InitSSAnim(struct ss_animation_s *ss_anim, uint32_t anim_type_id)
{
    ss_anim->anim_ext_flags = 0x00;
    ss_anim->anim_frame_flags = 0x00;
    ss_anim->type = anim_type_id;
    ss_anim->enabled = 0x01;
    ss_anim->do_jump_anim = 0x00;
    ss_anim->heavy_state = 0x00;
    ss_anim->model = NULL;
    ss_anim->onFrame = NULL;
    ss_anim->onEndFrame = NULL;

    ss_anim->frame_time = 0.0f;
    ss_anim->target_state = -1;
    ss_anim->lerp = 0.0;
    ss_anim->prev_animation = 0;
    ss_anim->prev_frame = 0;
    ss_anim->current_animation = 0;
    ss_anim->current_frame = 0;
    ss_anim->period = 1.0f / 30.0f;

    ss_anim->next = NULL;
    ss_anim->prev = NULL;
}


void SSBoneFrame_Clear(ss_bone_frame_p bf)
{
    if(bf && bf->bone_tag_count)
    {
        for(uint16_t i = 0; i < bf->bone_tag_count; i++)
        {
            if(bf->bone_tags[i].skin_map)
            {
                free(bf->bone_tags[i].skin_map);
            }
        }
        
        free(bf->bone_tags);
        bf->bone_tag_count = 0;
        bf->bone_tags = NULL;
    }

    for(ss_animation_p ss_anim = bf->animations.next; ss_anim;)
    {
        ss_animation_p ss_anim_next = ss_anim->next;
        ss_anim->next = NULL;
        free(ss_anim);
        ss_anim = ss_anim_next;
    }
    bf->animations.next = NULL;
}


void SSBoneFrame_Copy(struct ss_bone_frame_s *dst, struct ss_bone_frame_s *src)
{
    if(dst->bone_tag_count == src->bone_tag_count)
    {
        ss_animation_p src_a = &src->animations;
        
        vec3_copy(dst->pos, src->pos);
        vec3_copy(dst->centre, src->centre);
        vec3_copy(dst->bb_max, src->bb_max);
        vec3_copy(dst->bb_min, src->bb_min);
        
        for(; src_a; src_a = src_a->next)
        {
            ss_animation_p dst_a = SSBoneFrame_GetOverrideAnim(dst, src_a->type);
            if(!dst_a)
            {
                dst_a = SSBoneFrame_AddOverrideAnim(dst, src_a->model, src_a->type);
            }
            dst_a->enabled = src_a->enabled;
            dst_a->model = src_a->model;
            dst_a->prev_animation = src_a->prev_animation;
            dst_a->prev_frame = src_a->prev_frame;
            dst_a->current_animation = src_a->current_animation;
            dst_a->current_frame = src_a->current_frame;
            dst_a->lerp = src_a->lerp;
            dst_a->frame_time = src_a->frame_time;
            dst_a->target_state = src_a->target_state;
        }
        
        for(uint16_t i = 0; i < src->bone_tag_count; ++i)
        {
            dst->bone_tags[i].mod = src->bone_tags[i].mod;
            dst->bone_tags[i].alt_anim = src->bone_tags[i].alt_anim;
        }
    }
}


void SSBoneFrame_Update(struct ss_bone_frame_s *bf, float time)
{
    float t = 1.0f - bf->animations.lerp;
    ss_bone_tag_p btag = bf->bone_tags;
    bone_tag_p src_btag, next_btag;
    skeletal_model_p model = bf->animations.model;
    animation_frame_p curr_anim = model->animations + bf->animations.prev_animation;
    animation_frame_p next_anim = model->animations + bf->animations.current_animation;
    bone_frame_p curr_bf = curr_anim->frames + bf->animations.prev_frame;
    bone_frame_p next_bf = next_anim->frames + bf->animations.current_frame;
    
    vec3_interpolate_macro(bf->bb_max, curr_bf->bb_max, next_bf->bb_max, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->bb_min, curr_bf->bb_min, next_bf->bb_min, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->centre, curr_bf->centre, next_bf->centre, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->pos, curr_bf->pos, next_bf->pos, bf->animations.lerp, t);
    
    next_btag = next_bf->bone_tags;
    src_btag = curr_bf->bone_tags;
    for(uint16_t k = 0; k < curr_bf->bone_tag_count; k++, btag++, src_btag++, next_btag++)
    {
        vec3_interpolate_macro(btag->offset, src_btag->offset, next_btag->offset, bf->animations.lerp, t);
        vec3_copy(btag->local_transform + 12, btag->offset);
        btag->local_transform[15] = 1.0f;
        if(k == 0)
        {
            vec3_add(btag->local_transform + 12, btag->local_transform + 12, bf->pos);
            vec4_slerp(btag->qrotate, src_btag->qrotate, next_btag->qrotate, bf->animations.lerp);
        }
        else
        {
            bone_tag_p ov_src_btag = src_btag;
            bone_tag_p ov_next_btag = next_btag;
            float ov_lerp = bf->animations.lerp;
            if(btag->alt_anim && btag->alt_anim->model && btag->alt_anim->enabled && (btag->alt_anim->model->mesh_tree[k].replace_anim != 0))
            {
                curr_anim = btag->alt_anim->model->animations + btag->alt_anim->prev_animation;
                next_anim = btag->alt_anim->model->animations + btag->alt_anim->current_animation;
                bone_frame_p ov_curr_bf = curr_anim->frames + btag->alt_anim->prev_frame;
                bone_frame_p ov_next_bf = next_anim->frames + btag->alt_anim->current_frame;
                ov_lerp = btag->alt_anim->lerp;
                ov_src_btag = ov_curr_bf->bone_tags + k;
                ov_next_btag = ov_next_bf->bone_tags + k;
            }
            vec4_slerp(btag->qrotate, ov_src_btag->qrotate, ov_next_btag->qrotate, ov_lerp);
        }
        Mat4_set_qrotation(btag->local_transform, btag->qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    btag = bf->bone_tags;
    Mat4_Copy(btag->current_transform, btag->local_transform);
    btag++;
    for(uint16_t k = 1; k < curr_bf->bone_tag_count; k++, btag++)
    {
        Mat4_Mat4_mul(btag->current_transform, btag->parent->current_transform, btag->local_transform);
        SSBoneFrame_TargetBoneToSlerp(bf, btag, time);
    }
}


void SSBoneFrame_RotateBone(struct ss_bone_frame_s *bf, const float q_rotate[4], int bone)
{
    ss_bone_tag_p b_tag = b_tag = bf->bone_tags + bone;

    Mat4_RotateRByQuaternion(b_tag->local_transform, q_rotate);
    for(uint16_t i = bone; i < bf->bone_tag_count; i++)
    {
        ss_bone_tag_p btag = bf->bone_tags + i;
        if(btag->parent)
        {
            Mat4_Mat4_mul(btag->current_transform, btag->parent->current_transform, btag->local_transform);
            if(btag->parent->index < bone)
            {
                break;
            }
        }
        else
        {
            Mat4_Copy(btag->current_transform, btag->local_transform);
        }
    }
}


int  SSBoneFrame_CheckTargetBoneLimit(struct ss_bone_frame_s *bf, struct ss_bone_tag_s *b_tag, float target[3])
{
    float target_dir[3], target_local[3], limit_dir[3], t;

    Mat4_vec3_mul_inv(target_local, bf->transform->M4x4, target);
    if(b_tag->parent)
    {
        Mat4_vec3_mul_inv(target_local, b_tag->parent->current_transform, target_local);
    }
    vec3_sub(target_dir, target_local, b_tag->local_transform + 12);
    vec3_norm(target_dir, t);
    vec3_copy(limit_dir, b_tag->mod.limit);

    if((b_tag->mod.limit[3] == -1.0f) ||
       (vec3_dot(limit_dir, target_dir) > b_tag->mod.limit[3]))
    {
        return 1;
    }

    return 0;
}


void SSBoneFrame_TargetBoneToSlerp(struct ss_bone_frame_s *bf, struct ss_bone_tag_s *b_tag, float time)
{
    b_tag->mod.current_slerp = 1.0f;
    if(b_tag->is_targeted)
    {
        float clamped_q[4], q[4], target_dir[3], target_local[3], bone_dir[3];

        Mat4_vec3_mul_inv(target_local, bf->transform->M4x4, b_tag->mod.target_pos);
        if(b_tag->parent)
        {
            Mat4_vec3_mul_inv(target_local, b_tag->parent->current_transform, target_local);
        }
        vec3_sub(target_dir, target_local, b_tag->local_transform + 12);
        vec3_copy(bone_dir, b_tag->mod.bone_local_direction);

        vec4_GetQuaternionRotation(q, bone_dir, target_dir);
        if(q[3] < b_tag->mod.limit[3])
        {
            vec4_clampw(q, b_tag->mod.limit[3]);
        }
        if(b_tag->is_axis_modded)
        {
            q[0] *= b_tag->mod.axis_mod[0];
            q[1] *= b_tag->mod.axis_mod[1];
            q[2] *= b_tag->mod.axis_mod[2];
            q[3] = 1.0f - vec3_sqabs(q);
            q[3] = sqrtf(q[3]);
        }
        b_tag->mod.current_slerp = vec4_slerp_to(clamped_q, b_tag->mod.current_q, q, time * M_PI / 1.3f);
        vec4_copy(b_tag->mod.current_q, clamped_q);
        SSBoneFrame_RotateBone(bf, b_tag->mod.current_q, b_tag->index);
    }
    else if(b_tag->mod.current_q[3] < 1.0f)
    {       
        if(b_tag->mod.current_q[3] < 0.99f)
        {
            float zero_ang[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            float clamped_q[4];
            vec4_slerp_to(clamped_q, b_tag->mod.current_q, zero_ang, time * M_PI / 1.3f);
            vec4_copy(b_tag->mod.current_q, clamped_q);
            SSBoneFrame_RotateBone(bf, b_tag->mod.current_q, b_tag->index);
        }
        else
        {
            vec4_set_zero_angle(b_tag->mod.current_q);
        }
    }
}


void SSBoneFrame_SetTarget(struct ss_bone_tag_s *b_tag, const float target_pos[3], const float bone_dir[3])
{
    b_tag->is_targeted = 0x01;
    vec3_copy(b_tag->mod.target_pos, target_pos);
    vec3_copy(b_tag->mod.bone_local_direction, bone_dir);
}


void SSBoneFrame_SetTargetingAxisMod(struct ss_bone_tag_s *b_tag, const float mod[3])
{
    if(mod)
    {
        vec3_copy(b_tag->mod.axis_mod, mod);
        b_tag->is_axis_modded = 0x01;
    }
    else
    {
        vec3_set_one(b_tag->mod.axis_mod);
        b_tag->is_axis_modded = 0x00;
    }
}


void SSBoneFrame_SetTargetingLimit(struct ss_bone_tag_s *b_tag, const float limit[4])
{
    if(limit)
    {
        vec4_copy(b_tag->mod.limit, limit);
    }
    else
    {
        b_tag->mod.limit[0] = 0.0f;
        b_tag->mod.limit[1] = 0.0f;
        b_tag->mod.limit[2] = 0.0f;
        b_tag->mod.limit[3] = -1.0f;
    }
}


struct ss_animation_s *SSBoneFrame_AddOverrideAnim(struct ss_bone_frame_s *bf, struct skeletal_model_s *sm, uint16_t anim_type_id)
{
    if(!sm || (sm->mesh_count == bf->bone_tag_count))
    {
        ss_animation_p ss_anim = (ss_animation_p)malloc(sizeof(ss_animation_t));
        SSBoneFrame_InitSSAnim(ss_anim, anim_type_id);
        ss_anim->model = sm;

        ss_anim->next = bf->animations.next;
        if(bf->animations.next)
        {
            bf->animations.next->prev = ss_anim;
        }
        bf->animations.next = ss_anim;
        
        return ss_anim;
    }

    return NULL;
}


struct ss_animation_s *SSBoneFrame_GetOverrideAnim(struct ss_bone_frame_s *bf, uint16_t anim_type)
{
    for(ss_animation_p p = &bf->animations; p; p = p->next)
    {
        if(p->type == anim_type)
        {
            return p;
        }
    }
    return NULL;
}


void SSBoneFrame_EnableOverrideAnimByType(struct ss_bone_frame_s *bf, uint16_t anim_type)
{
    for(ss_animation_p ss_anim = &bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        if(ss_anim->type == anim_type)
        {
            ss_anim->enabled = 1;
            for(uint16_t i = 0; i < bf->bone_tag_count; i++)
            {
                mesh_tree_tag_p mtag = ss_anim->model->mesh_tree + i;
                if(mtag->replace_anim != 0)
                {
                    bf->bone_tags[i].alt_anim = ss_anim;
                }
            }
            break;
        }
    }
}


void SSBoneFrame_EnableOverrideAnim(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim)
{
    ss_anim->enabled = 1;
    for(uint16_t i = 0; i < bf->bone_tag_count; i++)
    {
        mesh_tree_tag_p mtag = ss_anim->model->mesh_tree + i;
        if(mtag->replace_anim != 0)
        {
            bf->bone_tags[i].alt_anim = ss_anim;
        }
    }
}


void SSBoneFrame_DisableOverrideAnimByType(struct ss_bone_frame_s *bf, uint16_t anim_type)
{
    for(ss_animation_p ss_anim = &bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        if(ss_anim->type == anim_type)
        {
            ss_anim->enabled = 0;
            break;
        }
    }
    
    for(uint16_t i = 0; i < bf->bone_tag_count; i++)
    {
        if(bf->bone_tags[i].alt_anim && bf->bone_tags[i].alt_anim->type == anim_type)
        {
            bf->bone_tags[i].alt_anim = NULL;
        }
    }
}


void SSBoneFrame_DisableOverrideAnim(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim)
{
    ss_anim->enabled = 0;
    for(uint16_t i = 0; i < bf->bone_tag_count; i++)
    {
        if(bf->bone_tags[i].alt_anim && bf->bone_tags[i].alt_anim->type == ss_anim->type)
        {
            bf->bone_tags[i].alt_anim = NULL;
        }
    }
}


/*
 *******************************************************************************
 */
void Anim_Clear(struct animation_frame_s *anim)
{
    if(anim->state_change_count)
    {
        for(uint16_t j = 0; j < anim->state_change_count; j++)
        {
            anim->state_change[j].anim_dispatch_count = 0;
            free(anim->state_change[j].anim_dispatch);
            anim->state_change[j].anim_dispatch = NULL;
            anim->state_change[j].id = 0;
        }
        anim->state_change_count = 0;
        free(anim->state_change);
        anim->state_change = NULL;
    }

    if(anim->frames_count)
    {
        for(uint16_t j = 0; j < anim->frames_count; j++)
        {
            if(anim->frames[j].bone_tag_count)
            {
                anim->frames[j].bone_tag_count = 0;
                free(anim->frames[j].bone_tags);
                anim->frames[j].bone_tags = NULL;
            }
        }
        anim->frames_count = 0;
        anim->max_frame = 0;
        free(anim->frames);
        anim->frames = NULL;
    }

    while(anim->commands)
    {
        animation_command_p next_command = anim->commands->next;
        free(anim->commands);
        anim->commands = next_command;
    }

    while(anim->effects)
    {
        animation_effect_p next_effect = anim->effects->next;
        free(anim->effects);
        anim->effects = next_effect;
    }
}


void Anim_AddCommand(struct animation_frame_s *anim, const animation_command_p command)
{
    animation_command_p *ptr = &anim->commands;
    for(; *ptr; ptr = &((*ptr)->next));
    *ptr = (animation_command_p)malloc(sizeof(animation_command_t));
    **ptr = *command;
    (*ptr)->next = NULL;
}


void Anim_AddEffect(struct animation_frame_s *anim, const animation_effect_p effect)
{
    animation_effect_p *ptr = &anim->effects;
    for(; *ptr; ptr = &((*ptr)->next));
    *ptr = (animation_effect_p)malloc(sizeof(animation_effect_t));
    **ptr = *effect;
    (*ptr)->next = NULL;
}


struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, uint32_t id)
{
    state_change_p ret = anim->state_change;
    for(uint16_t i = 0; i < anim->state_change_count; i++, ret++)
    {
        if(ret->id == id)
        {
            return ret;
        }
    }

    return NULL;
}


int Anim_GetAnimDispatchCase(struct ss_animation_s *ss_anim, uint32_t id)
{
    animation_frame_p anim = ss_anim->model->animations + ss_anim->prev_animation;
    state_change_p stc = anim->state_change;

    for(uint16_t i = 0; i < anim->state_change_count; i++, stc++)
    {
        if(stc->id == id)
        {
            anim_dispatch_p disp = stc->anim_dispatch;
            for(uint16_t j = 0; j < stc->anim_dispatch_count; j++, disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (ss_anim->prev_frame >= disp->frame_low) && (ss_anim->prev_frame <= disp->frame_high))
                {
                    return (int)j;
                }
            }
        }
    }

    return -1;
}


void Anim_SetAnimation(struct ss_animation_s *ss_anim, int animation, int frame)
{
    if(ss_anim && ss_anim->model && (animation < ss_anim->model->animation_count))
    {
        animation_frame_p anim = &ss_anim->model->animations[animation];
        ss_anim->lerp = 0.0;
        frame %= anim->max_frame;
        frame = (frame >= 0) ? (frame) : (anim->max_frame + frame);
        ss_anim->period = 1.0f / 30.0f;

        ss_anim->frame_changing_state = 0x04;

        ss_anim->target_state = -1;

        ss_anim->current_animation = animation;
        ss_anim->current_frame = frame;
        ss_anim->prev_animation = animation;
        ss_anim->prev_frame = frame;

        ss_anim->frame_time = (float)frame * ss_anim->period;
    }
}

/*
 * Next frame and next anim calculation function.
 */
int  Anim_SetNextFrame(struct ss_animation_s *ss_anim, float time)
{
    float dt;
    int32_t new_frame;
    animation_frame_p next_anim = ss_anim->model->animations + ss_anim->current_animation;
    state_change_p stc = (ss_anim->target_state >= 0) ? (Anim_FindStateChangeByID(next_anim, ss_anim->target_state)) : (NULL);
    
    if(ss_anim->heavy_state && (next_anim->state_id == ss_anim->target_state) && (next_anim->next_anim->state_id == ss_anim->target_state))
    {
        ss_anim->heavy_state = 0x00;
    }
    
    ss_anim->frame_time = (ss_anim->frame_time >= 0.0f) ? (ss_anim->frame_time) : (0.0f);
    ss_anim->frame_time += time;
    new_frame = ss_anim->frame_time / ss_anim->period;
    if((ss_anim->current_animation == ss_anim->prev_animation) && 
       (ss_anim->current_frame == ss_anim->prev_frame) && 
       (ss_anim->current_frame == new_frame))
    {
        ++new_frame;
        ss_anim->frame_time += ss_anim->period;
    }
    
    dt = ss_anim->frame_time - (float)new_frame * ss_anim->period;
    ss_anim->lerp = dt / ss_anim->period;
    
    ss_anim->frame_changing_state = 0x00;
    
    /*
     * Flag has a highest priority
     */
    if(ss_anim->anim_frame_flags & ANIM_FRAME_LOCK)
    {
        ss_anim->prev_frame = 0;
        ss_anim->prev_frame = ss_anim->current_frame;
        ss_anim->prev_animation = ss_anim->current_animation;
        ss_anim->lerp = 0.0f;
        ss_anim->frame_time = 0.0f;
        return 0x00;
    }
    else if((new_frame + 1 >= next_anim->max_frame) && (ss_anim->anim_frame_flags == ANIM_LOOP_LAST_FRAME))
    {
        ss_anim->current_frame = next_anim->max_frame - 1;
        ss_anim->prev_frame = ss_anim->current_frame;
        ss_anim->prev_animation = ss_anim->current_animation;
        ss_anim->lerp = 0.0f;
        ss_anim->frame_time = (float)ss_anim->current_frame * ss_anim->period;
        return 0x00;
    }
    
    /*
     * State change check
     */
    if(stc)
    {
        anim_dispatch_p disp = stc->anim_dispatch;
        for(uint16_t i = 0; i < stc->anim_dispatch_count; i++, disp++)
        {
            if((next_anim->max_frame == 1) || 
               (new_frame >= disp->frame_low) && (new_frame <= disp->frame_high) || 
               (ss_anim->current_frame <= disp->frame_high) && (new_frame >= disp->frame_high))
            {
                ss_anim->prev_animation = ss_anim->current_animation;
                ss_anim->prev_frame = ss_anim->current_frame;
                ss_anim->current_animation = disp->next_anim;
                ss_anim->current_frame = disp->next_frame;
                ss_anim->frame_time = (float)ss_anim->current_frame * ss_anim->period + dt;
                ss_anim->target_state = ss_anim->heavy_state ? ss_anim->target_state : -1;
                ss_anim->frame_changing_state = 0x03;
                return 0x03;
            }
        }
    }
    
    /*
     * Check next anim if frame >= max_frame
     */
    if(new_frame >= next_anim->max_frame)
    {
        ss_anim->prev_animation = ss_anim->current_animation;
        ss_anim->prev_frame = ss_anim->current_frame;
        ss_anim->current_frame = next_anim->next_frame;
        ss_anim->current_animation = next_anim->next_anim->id;
        ss_anim->frame_time = (float)ss_anim->current_frame * ss_anim->period + dt;
        ss_anim->target_state = ss_anim->heavy_state ? ss_anim->target_state : -1;
        ss_anim->frame_changing_state = 0x02;
        return 0x02;
    }
    
    if(ss_anim->current_frame != new_frame)
    {
        ss_anim->prev_animation = ss_anim->current_animation;
        ss_anim->prev_frame = ss_anim->current_frame;
        ss_anim->current_frame = new_frame;
        ss_anim->frame_changing_state = 0x01;
        return 0x01;
    }
    
    return 0x00;
}


int  Anim_IncTime(struct ss_animation_s *ss_anim, float time)
{
    ss_anim->frame_time += time;
    if(ss_anim->frame_time <= 0.0f)
    {
        ss_anim->frame_time = 0.0f;
        ss_anim->prev_frame = 0;
        ss_anim->current_frame = 0;
        ss_anim->lerp = 0.0f;
        ss_anim->frame_changing_state = 0x02;
        return 2;
    }

    animation_frame_p curr_anim = ss_anim->model->animations + ss_anim->current_animation;
    int16_t prev_frame = ss_anim->prev_frame;
    ss_anim->prev_frame = ss_anim->frame_time / ss_anim->period;
    ss_anim->current_frame = ss_anim->prev_frame + 1;
    if(ss_anim->current_frame >= curr_anim->max_frame)
    {
        ss_anim->frame_time = ss_anim->period * (float)(curr_anim->max_frame - 1);
        ss_anim->frame_time = (ss_anim->frame_time < 0.0f) ? (0.0f) : (ss_anim->frame_time);
        ss_anim->prev_frame = curr_anim->max_frame - 1;
        ss_anim->current_frame = curr_anim->max_frame - 1;
        ss_anim->lerp = 1.0f;
        ss_anim->frame_changing_state = 0x2;
        return 1;
    }

    float dt = ss_anim->frame_time - (float)ss_anim->prev_frame * ss_anim->period;
    ss_anim->lerp = dt / ss_anim->period;
    ss_anim->frame_changing_state = (prev_frame == ss_anim->prev_frame) ? (0x00) : (0x01);
    return 0;
}


void SSBoneFrame_FillSkinnedMeshMap(ss_bone_frame_p bf)
{
    uint32_t *ch;
    uint32_t founded_index = 0xFFFFFFFF;
    float tv[3];
    vertex_p v, founded_vertex;
    base_mesh_p mesh_base, mesh_skin;
    ss_bone_tag_p tree_tag = bf->bone_tags;

    for(uint16_t i = 0; i < bf->bone_tag_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            continue;
        }
        if(tree_tag->skin_map)
        {
            free(tree_tag->skin_map);
            tree_tag->skin_map = NULL;
        }
        mesh_base = tree_tag->mesh_base;
        mesh_skin = tree_tag->mesh_skin;
        ch = tree_tag->skin_map = (uint32_t*)malloc(mesh_skin->vertex_count * sizeof(uint32_t));
        v = mesh_skin->vertices;
        for(uint32_t k = 0; k < mesh_skin->vertex_count; k++, v++, ch++)
        {
            *ch = 0xFFFFFFFF;
            founded_index = BaseMesh_FindVertexIndex(mesh_base, v->position);
            if(founded_index != 0xFFFFFFFF)
            {
                founded_vertex = mesh_base->vertices + founded_index;
                vec3_copy(v->position, founded_vertex->position);
                vec3_copy(v->normal, founded_vertex->normal);
            }
            else if(tree_tag->parent)
            {
                vec3_add(tv, v->position, tree_tag->offset);
                founded_index = BaseMesh_FindVertexIndex(tree_tag->parent->mesh_base, tv);
                if(founded_index != 0xFFFFFFFF)
                {
                    founded_vertex = tree_tag->parent->mesh_base->vertices + founded_index;
                    *ch = founded_index;
                    vec3_sub(v->position, founded_vertex->position, tree_tag->offset);
                    vec3_copy(v->normal, founded_vertex->normal);
                }
            }
        }
    }
}