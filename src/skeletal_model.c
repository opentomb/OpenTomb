
#include <stdlib.h>

#include "core/system.h"
#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "mesh.h"
#include "skeletal_model.h"


void SSBoneFrame_InitSSAnim(struct ss_animation_s *ss_anim, uint32_t anim_type_id);

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
            animation_frame_p anim = model->animations;
            for(uint16_t i = 0; i < model->animation_count; i++, anim++)
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
                    free(anim->frames);
                    anim->frames = NULL;
                }
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
    uint16_t parents[model->mesh_count];

    parents[0] = 0;
    model->mesh_tree[0].parent = 0;                                             // root
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
            if(stack + 1 < (int16_t)model->mesh_count)
            {
                stack++;
                parents[stack] = model->mesh_tree[i].parent;
            }
        }
    }
}


void SkeletalModel_InterpolateFrames(skeletal_model_p model)
{
    uint16_t new_frames_count;
    animation_frame_p anim = model->animations;
    bone_frame_p bf, new_bone_frames;
    float lerp, t;

    for(uint16_t i = 0; i < model->animation_count; i++, anim++)
    {
        if(anim->frames_count > 1 && anim->original_frame_rate > 1)             // we can't interpolate one frame or rate < 2!
        {
            new_frames_count = (uint16_t)anim->original_frame_rate * (anim->frames_count - 1) + 1;
            bf = new_bone_frames = (bone_frame_p)malloc(new_frames_count * sizeof(bone_frame_t));

            /*
             * the first frame does not changes
             */
            bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            bf->bone_tag_count = model->mesh_count;
            vec3_set_zero(bf->pos);
            vec3_copy(bf->centre, anim->frames[0].centre);
            vec3_copy(bf->pos, anim->frames[0].pos);
            vec3_copy(bf->bb_max, anim->frames[0].bb_max);
            vec3_copy(bf->bb_min, anim->frames[0].bb_min);
            for(uint16_t k = 0; k < model->mesh_count; k++)
            {
                vec3_copy(bf->bone_tags[k].offset, anim->frames[0].bone_tags[k].offset);
                vec4_copy(bf->bone_tags[k].qrotate, anim->frames[0].bone_tags[k].qrotate);
            }
            bf++;

            for(uint16_t j = 1; j < anim->frames_count; j++)
            {
                for(uint16_t lerp_index = 1; lerp_index <= anim->original_frame_rate; lerp_index++)
                {
                    vec3_set_zero(bf->pos);
                    lerp = ((float)lerp_index) / (float)anim->original_frame_rate;
                    t = 1.0f - lerp;

                    bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
                    bf->bone_tag_count = model->mesh_count;

                    bf->centre[0] = t * anim->frames[j-1].centre[0] + lerp * anim->frames[j].centre[0];
                    bf->centre[1] = t * anim->frames[j-1].centre[1] + lerp * anim->frames[j].centre[1];
                    bf->centre[2] = t * anim->frames[j-1].centre[2] + lerp * anim->frames[j].centre[2];

                    bf->pos[0] = t * anim->frames[j-1].pos[0] + lerp * anim->frames[j].pos[0];
                    bf->pos[1] = t * anim->frames[j-1].pos[1] + lerp * anim->frames[j].pos[1];
                    bf->pos[2] = t * anim->frames[j-1].pos[2] + lerp * anim->frames[j].pos[2];

                    bf->bb_max[0] = t * anim->frames[j-1].bb_max[0] + lerp * anim->frames[j].bb_max[0];
                    bf->bb_max[1] = t * anim->frames[j-1].bb_max[1] + lerp * anim->frames[j].bb_max[1];
                    bf->bb_max[2] = t * anim->frames[j-1].bb_max[2] + lerp * anim->frames[j].bb_max[2];

                    bf->bb_min[0] = t * anim->frames[j-1].bb_min[0] + lerp * anim->frames[j].bb_min[0];
                    bf->bb_min[1] = t * anim->frames[j-1].bb_min[1] + lerp * anim->frames[j].bb_min[1];
                    bf->bb_min[2] = t * anim->frames[j-1].bb_min[2] + lerp * anim->frames[j].bb_min[2];

                    for(uint16_t k = 0; k < model->mesh_count; k++)
                    {
                        bf->bone_tags[k].offset[0] = t * anim->frames[j-1].bone_tags[k].offset[0] + lerp * anim->frames[j].bone_tags[k].offset[0];
                        bf->bone_tags[k].offset[1] = t * anim->frames[j-1].bone_tags[k].offset[1] + lerp * anim->frames[j].bone_tags[k].offset[1];
                        bf->bone_tags[k].offset[2] = t * anim->frames[j-1].bone_tags[k].offset[2] + lerp * anim->frames[j].bone_tags[k].offset[2];
                        
                        vec4_slerp(bf->bone_tags[k].qrotate, anim->frames[j-1].bone_tags[k].qrotate, anim->frames[j].bone_tags[k].qrotate, lerp);
                    }
                    bf++;
                }
            }

            /*
             * swap old and new animation bone brames
             * free old bone frames;
             */
            for(uint16_t j = 0; j < anim->frames_count; j++)
            {
                if(anim->frames[j].bone_tag_count)
                {
                    anim->frames[j].bone_tag_count = 0;
                    free(anim->frames[j].bone_tags);
                    anim->frames[j].bone_tags = NULL;
                }
            }
            free(anim->frames);
            anim->frames = new_bone_frames;
            anim->frames_count = new_frames_count;
        }
    }
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


void SkeletalModel_FillSkinnedMeshMap(skeletal_model_p model)
{
    uint32_t *ch;
    uint32_t founded_index = 0xFFFFFFFF;
    float tv[3];
    vertex_p v, founded_vertex;
    base_mesh_p mesh_base, mesh_skin;
    mesh_tree_tag_p tree_tag, prev_tree_tag;

    tree_tag = model->mesh_tree;
    for(uint16_t i = 0; i < model->mesh_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            continue;
        }
        mesh_base = tree_tag->mesh_base;
        mesh_skin = tree_tag->mesh_skin;
        ch = mesh_skin->skin_map = (uint32_t*)malloc(mesh_skin->vertex_count * sizeof(uint32_t));
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
            else
            {
                vec3_add(tv, v->position, tree_tag->offset);
                prev_tree_tag = model->mesh_tree + tree_tag->parent;
                founded_index = BaseMesh_FindVertexIndex(prev_tree_tag->mesh_base, tv);
                if(founded_index != 0xFFFFFFFF)
                {
                    founded_vertex = prev_tree_tag->mesh_base->vertices + founded_index;
                    *ch = founded_index;
                    vec3_sub(v->position, founded_vertex->position, tree_tag->offset);
                    vec3_copy(v->normal, founded_vertex->normal);
                }
            }
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


void SkeletalModel_CopyMeshesToSkinned(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        if(i != src[i].parent)
        {
            dst[i].mesh_skin = src[i].mesh_base;
        }
    }
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
            bf->bone_tags[i].index = i;
            bf->bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
            bf->bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
            bf->bone_tags[i].mesh_slot = NULL;
            bf->bone_tags[i].alt_anim = NULL;
            bf->bone_tags[i].body_part = model->mesh_tree[i].body_part;

            vec3_copy(bf->bone_tags[i].offset, model->mesh_tree[i].offset);
            vec4_set_zero(bf->bone_tags[i].qrotate);
            Mat4_E_macro(bf->bone_tags[i].transform);
            Mat4_E_macro(bf->bone_tags[i].full_transform);

            if(i > 0)
            {
                bf->bone_tags[i].parent = bf->bone_tags + model->mesh_tree[i].parent;
            }
        }
    }
}


void SSBoneFrame_InitSSAnim(struct ss_animation_s *ss_anim, uint32_t anim_type_id)
{
    ss_anim->anim_ext_flags = 0x00;
    ss_anim->anim_frame_flags = 0x00;
    ss_anim->type = anim_type_id;
    ss_anim->enabled = 1;
    ss_anim->model = NULL;
    ss_anim->onFrame = NULL;
    ss_anim->onEndFrame = NULL;
    ss_anim->targeting_bone = 0x00;
    ss_anim->targeting_flags = 0x0000;
    vec3_set_zero(ss_anim->target);
    vec4_set_zero_angle(ss_anim->current_mod);
    ss_anim->bone_direction[0] = 0.0f;
    ss_anim->bone_direction[1] = 1.0f;
    ss_anim->bone_direction[2] = 0.0f;
    ss_anim->targeting_limit[0] = 0.0f;
    ss_anim->targeting_limit[1] = 1.0f;
    ss_anim->targeting_limit[2] = 0.0f;
    ss_anim->targeting_limit[3] =-1.0f;
    vec3_set_one(ss_anim->targeting_axis_mod);

    ss_anim->frame_time = 0.0f;
    ss_anim->next_state = 0;
    ss_anim->lerp = 0.0;
    ss_anim->current_animation = 0;
    ss_anim->current_frame = 0;
    ss_anim->next_animation = 0;
    ss_anim->next_frame = 0;
    ss_anim->period = 1.0f / 30.0f;

    ss_anim->next = NULL;
    ss_anim->prev = NULL;
}


void SSBoneFrame_Clear(ss_bone_frame_p bf)
{
    if(bf && bf->bone_tag_count)
    {
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


void SSBoneFrame_Update(struct ss_bone_frame_s *bf, float time)
{
    float t = 1.0f - bf->animations.lerp;
    ss_bone_tag_p btag = bf->bone_tags;
    bone_tag_p src_btag, next_btag;
    skeletal_model_p model = bf->animations.model;
    animation_frame_p curr_anim = model->animations + bf->animations.current_animation;
    animation_frame_p next_anim = model->animations + bf->animations.next_animation;
    bone_frame_p curr_bf = curr_anim->frames + bf->animations.current_frame;
    bone_frame_p next_bf = next_anim->frames + bf->animations.next_frame;
    
    vec3_interpolate_macro(bf->bb_max, curr_bf->bb_max, next_bf->bb_max, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->bb_min, curr_bf->bb_min, next_bf->bb_min, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->centre, curr_bf->centre, next_bf->centre, bf->animations.lerp, t);
    vec3_interpolate_macro(bf->pos, curr_bf->pos, next_bf->pos, bf->animations.lerp, t);
    
    next_btag = next_bf->bone_tags;
    src_btag = curr_bf->bone_tags;
    for(uint16_t k = 0; k < curr_bf->bone_tag_count; k++, btag++, src_btag++, next_btag++)
    {
        vec3_interpolate_macro(btag->offset, src_btag->offset, next_btag->offset, bf->animations.lerp, t);
        vec3_copy(btag->transform + 12, btag->offset);
        btag->transform[15] = 1.0f;
        if(k == 0)
        {
            vec3_add(btag->transform + 12, btag->transform + 12, bf->pos);
            vec4_slerp(btag->qrotate, src_btag->qrotate, next_btag->qrotate, bf->animations.lerp);
        }
        else
        {
            bone_tag_p ov_src_btag = src_btag;
            bone_tag_p ov_next_btag = next_btag;
            float ov_lerp = bf->animations.lerp;
            if(btag->alt_anim && btag->alt_anim->model && btag->alt_anim->enabled && (btag->alt_anim->model->mesh_tree[k].replace_anim != 0))
            {
                curr_anim = btag->alt_anim->model->animations + btag->alt_anim->current_animation;
                next_anim = btag->alt_anim->model->animations + btag->alt_anim->next_animation;
                bone_frame_p ov_curr_bf = curr_anim->frames + btag->alt_anim->current_frame;
                bone_frame_p ov_next_bf = next_anim->frames + btag->alt_anim->next_frame;
                ov_lerp = btag->alt_anim->lerp;
                ov_src_btag = ov_curr_bf->bone_tags + k;
                ov_next_btag = ov_next_bf->bone_tags + k;
            }
            vec4_slerp(btag->qrotate, ov_src_btag->qrotate, ov_next_btag->qrotate, ov_lerp);
        }
        Mat4_set_qrotation(btag->transform, btag->qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    btag = bf->bone_tags;
    Mat4_Copy(btag->full_transform, btag->transform);
    Mat4_Copy(btag->orig_transform, btag->transform);
    btag++;
    for(uint16_t k = 1; k < curr_bf->bone_tag_count; k++, btag++)
    {
        Mat4_Mat4_mul(btag->full_transform, btag->parent->full_transform, btag->transform);
        Mat4_Copy(btag->orig_transform, btag->full_transform);
    }

    for(ss_animation_p ss_anim = &bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        SSBoneFrame_TargetBoneToSlerp(bf, ss_anim, time);
    }
}


void SSBoneFrame_RotateBone(struct ss_bone_frame_s *bf, const float q_rotate[4], int bone)
{
    float tr[16], q[4];
    ss_bone_tag_p b_tag = b_tag = bf->bone_tags + bone;
    
    vec4_copy(q, q_rotate);
    Mat4_E(tr);
    Mat4_RotateQuaternion(tr, q);
    vec4_copy(q, b_tag->transform + 12);
    Mat4_Mat4_mul(b_tag->transform, tr, b_tag->transform);
    vec4_copy(b_tag->transform + 12, q);
    for(uint16_t i = bone; i < bf->bone_tag_count; i++)
    {
        ss_bone_tag_p btag = bf->bone_tags + i;
        if(btag->parent)
        {
            Mat4_Mat4_mul(btag->full_transform, btag->parent->full_transform, btag->transform);
        }
        else
        {
            Mat4_Copy(btag->full_transform, btag->transform);
        }
    }
}


int  SSBoneFrame_CheckTargetBoneLimit(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim)
{
    ss_bone_tag_p b_tag = b_tag = bf->bone_tags + ss_anim->targeting_bone;
    float target_dir[3], target_local[3], limit_dir[3], t;

    Mat4_vec3_mul_inv(target_local, bf->transform, ss_anim->target);
    if(b_tag->parent)
    {
        Mat4_vec3_mul_inv(target_local, b_tag->parent->full_transform, target_local);
    }
    vec3_sub(target_dir, target_local, b_tag->transform + 12);
    vec3_norm(target_dir, t);
    vec3_copy(limit_dir, ss_anim->targeting_limit);

    if((ss_anim->targeting_limit[3] == -1.0f) ||
       (vec3_dot(limit_dir, target_dir) > ss_anim->targeting_limit[3]))
    {
        return 1;
    }

    return 0;
}


void SSBoneFrame_TargetBoneToSlerp(struct ss_bone_frame_s *bf, struct ss_animation_s *ss_anim, float time)
{
    if(ss_anim->anim_ext_flags & ANIM_EXT_TARGET_TO)
    {
        ss_bone_tag_p b_tag = bf->bone_tags + ss_anim->targeting_bone;
        float clamped_q[4], q[4], target_dir[3], target_local[3], bone_dir[3];

        Mat4_vec3_mul_inv(target_local, bf->transform, ss_anim->target);
        if(b_tag->parent)
        {
            Mat4_vec3_mul_inv(target_local, b_tag->parent->full_transform, target_local);
        }
        vec3_sub(target_dir, target_local, b_tag->transform + 12);
        if(ss_anim->targeting_flags & ANIM_TARGET_OWERRIDE_ANIM)
        {
            Mat4_vec3_rot_macro(bone_dir, b_tag->transform, ss_anim->bone_direction);
        }
        else
        {
            vec3_copy(bone_dir, ss_anim->bone_direction);
        }

        vec4_GetQuaternionRotation(q, bone_dir, target_dir);
        if(ss_anim->targeting_flags & ANIM_TARGET_USE_AXIS_MOD)
        {
            q[0] *= ss_anim->targeting_axis_mod[0];
            q[1] *= ss_anim->targeting_axis_mod[1];
            q[2] *= ss_anim->targeting_axis_mod[2];
            q[3] = 1.0f - vec3_sqabs(q);
            q[3] = sqrtf(q[3]);
        }
        if(q[3] < ss_anim->targeting_limit[3])
        {
            vec4_clampw(q, ss_anim->targeting_limit[3]);
        }
        vec4_slerp_to(clamped_q, ss_anim->current_mod, q, time * M_PI / 1.3f);
        vec4_copy(ss_anim->current_mod, clamped_q);
        SSBoneFrame_RotateBone(bf, ss_anim->current_mod, ss_anim->targeting_bone);
    }
    else if(ss_anim->current_mod[3] < 1.0f)
    {       
        if(ss_anim->current_mod[3] < 0.99f)
        {
            float zero_ang[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            float clamped_q[4];
            vec4_slerp_to(clamped_q, ss_anim->current_mod, zero_ang, time * M_PI / 1.3f);
            vec4_copy(ss_anim->current_mod, clamped_q);
            SSBoneFrame_RotateBone(bf, ss_anim->current_mod, ss_anim->targeting_bone);
        }
        else
        {
            vec4_set_zero_angle(ss_anim->current_mod);
        }
    }
}


void SSBoneFrame_SetTrget(struct ss_animation_s *ss_anim, uint16_t targeted_bone, const float target_pos[3], const float bone_dir[3])
{
    ss_anim->targeting_bone = targeted_bone;
    vec3_copy(ss_anim->target, target_pos);
    vec3_copy(ss_anim->bone_direction, bone_dir);
}


void SSBoneFrame_SetTargetingAxisMod(struct ss_animation_s *ss_anim, const float mod[3])
{
    if(mod)
    {
        vec3_copy(ss_anim->targeting_axis_mod, mod);
        ss_anim->targeting_flags |= ANIM_TARGET_USE_AXIS_MOD;
    }
    else
    {
        vec3_set_one(ss_anim->targeting_axis_mod);
        ss_anim->targeting_flags &= ~ANIM_TARGET_USE_AXIS_MOD;
    }
}


void SSBoneFrame_SetTargetingLimit(struct ss_animation_s *ss_anim, const float limit[4])
{
    if(limit)
    {
        vec4_copy(ss_anim->targeting_limit, limit);
    }
    else
    {
        ss_anim->targeting_limit[3] = -1.0f;
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


void SSBoneFrame_DisableOverrideAnim(struct ss_bone_frame_s *bf, uint16_t anim_type)
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


/*
 *******************************************************************************
 */
struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim)
{
    if(state_change_anim >= 0)
    {
        state_change_p ret = anim->state_change;
        for(uint16_t i = 0; i < anim->state_change_count; i++, ret++)
        {
            for(uint16_t j = 0; j < ret->anim_dispatch_count; j++)
            {
                if(ret->anim_dispatch[j].next_anim == state_change_anim)
                {
                    return ret;
                }
            }
        }
    }

    return NULL;
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


int Anim_GetAnimDispatchCase(struct ss_bone_frame_s *bf, uint32_t id)
{
    animation_frame_p anim = bf->animations.model->animations + bf->animations.current_animation;
    state_change_p stc = anim->state_change;

    for(uint16_t i = 0; i < anim->state_change_count; i++, stc++)
    {
        if(stc->id == id)
        {
            anim_dispatch_p disp = stc->anim_dispatch;
            for(uint16_t j = 0; j < stc->anim_dispatch_count; j++, disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (bf->animations.current_frame >= disp->frame_low) && (bf->animations.current_frame <= disp->frame_high))
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
        frame %= anim->frames_count;
        frame = (frame >= 0) ? (frame) : (anim->frames_count - 1 + frame);
        ss_anim->period = 1.0f / 30.0f;

        ss_anim->changing_curr = 0x03;
        ss_anim->changing_next = 0x03;
        
        ss_anim->current_state = anim->state_id;
        ss_anim->next_state = anim->state_id;

        ss_anim->next_animation = animation;
        ss_anim->next_frame = frame;
        ss_anim->current_animation = animation;
        ss_anim->current_frame = frame;

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
    animation_frame_p next_anim = ss_anim->model->animations + ss_anim->next_animation;
    state_change_p stc = Anim_FindStateChangeByID(next_anim, ss_anim->next_state);
    
    ss_anim->frame_time = (ss_anim->frame_time >= 0.0f) ? (ss_anim->frame_time) : (0.0f);
    ss_anim->frame_time += time;
    new_frame = ss_anim->frame_time / ss_anim->period;
    dt = ss_anim->frame_time - (float)new_frame * ss_anim->period;
    ss_anim->lerp = dt / ss_anim->period;
    
    ss_anim->changing_next = 0x00;
    ss_anim->changing_curr = 0x00;
    
    /*
     * Flag has a highest priority
     */
    if((new_frame + 1 >= next_anim->frames_count) && (ss_anim->anim_frame_flags == ANIM_LOOP_LAST_FRAME))
    {
        ss_anim->next_frame = next_anim->frames_count - 1;
        ss_anim->current_frame = ss_anim->next_frame;
        ss_anim->current_animation = ss_anim->next_animation;
        ss_anim->lerp = 0.0f;
        ss_anim->frame_time = (float)ss_anim->next_frame * ss_anim->period;
        return 0x00;
    }
    else if(ss_anim->anim_frame_flags == ANIM_FRAME_LOCK)
    {
        ss_anim->current_frame = 0;
        ss_anim->current_frame = ss_anim->next_frame;
        ss_anim->current_animation = ss_anim->next_animation;
        ss_anim->lerp = 0.0f;
        ss_anim->frame_time = 0.0f;
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
            if((disp->frame_high >= disp->frame_low) && ((new_frame >= disp->frame_low) && (new_frame <= disp->frame_high)))
            {
                ss_anim->current_animation = ss_anim->next_animation;
                ss_anim->current_frame = ss_anim->next_frame;
                ss_anim->next_animation = disp->next_anim;
                ss_anim->next_frame = disp->next_frame;
                ss_anim->frame_time = (float)ss_anim->next_frame * ss_anim->period + dt;
                ss_anim->current_state = ss_anim->model->animations[ss_anim->next_animation].state_id;
                ss_anim->next_state = ss_anim->current_state;
                ss_anim->changing_curr = ss_anim->changing_next;
                ss_anim->changing_next = 0x03;
                return 0x03;
            }
        }
    }
    
    /*
     * Check next anim if frame >= frames_count
     */
    if(new_frame + 1 > next_anim->frames_count)
    {
        ss_anim->current_animation = ss_anim->next_animation;
        ss_anim->current_frame = ss_anim->next_frame;
        ss_anim->next_frame = next_anim->next_frame;
        ss_anim->next_animation  = next_anim->next_anim->id;
        ss_anim->frame_time = (float)ss_anim->next_frame * ss_anim->period + dt;
        ss_anim->current_state = ss_anim->model->animations[ss_anim->next_animation].state_id;
        ss_anim->next_state = ss_anim->current_state;
        ss_anim->changing_curr = ss_anim->changing_next;
        ss_anim->changing_next = 0x02;
        return 0x02;
    }
    
    if(ss_anim->next_frame != new_frame)
    {
        ss_anim->current_animation = ss_anim->next_animation;
        ss_anim->current_frame = ss_anim->next_frame;
        ss_anim->next_frame = new_frame;
        ss_anim->changing_curr = ss_anim->changing_next;
        ss_anim->changing_next = 0x01;
        return 0x01;
    }
    
    return 0x00;
}
