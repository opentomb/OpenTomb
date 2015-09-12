
#include <stdlib.h>

#include "core/system.h"
#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "mesh.h"
#include "skeletal_model.h"


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


void TreeTag_GenParentsIndexes(skeletal_model_p model)
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


void SSBoneFrame_CreateFromModel(ss_bone_frame_p bf, skeletal_model_p model)
{
    vec3_set_zero(bf->bb_min);
    vec3_set_zero(bf->bb_max);
    vec3_set_zero(bf->centre);
    vec3_set_zero(bf->pos);
    bf->animations.anim_flags = 0x0000;
    bf->animations.frame_time = 0.0;
    bf->animations.period = 1.0 / 30.0;
    bf->animations.next_state = 0;
    bf->animations.lerp = 0.0;
    bf->animations.current_animation = 0;
    bf->animations.current_frame = 0;
    bf->animations.next_animation = 0;
    bf->animations.next_frame = 0;

    bf->animations.next = NULL;
    bf->animations.onFrame = NULL;
    bf->animations.model = model;
    bf->bone_tag_count = model->mesh_count;
    bf->bone_tags = (ss_bone_tag_p)malloc(bf->bone_tag_count * sizeof(ss_bone_tag_t));

    bf->bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i = 0; i < bf->bone_tag_count; i++)
    {
        bf->bone_tags[i].index = i;
        bf->bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bf->bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        bf->bone_tags[i].mesh_slot = NULL;
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

    dst->command = src->command;
    vec3_copy(dst->move, src->move);

    for(uint16_t i = 0; i < dst->bone_tag_count; i++)
    {
        vec4_copy(dst->bone_tags[i].qrotate, src->bone_tags[i].qrotate);
        vec3_copy(dst->bone_tags[i].offset, src->bone_tags[i].offset);
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
            vec3_set_zero(bf->move);
            bf->command = 0x00;
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
                    vec3_set_zero(bf->move);
                    bf->command = 0x00;
                    lerp = ((float)lerp_index) / (float)anim->original_frame_rate;
                    t = 1.0 - lerp;

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


void SkeletonModel_FillTransparency(skeletal_model_p model)
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


mesh_tree_tag_p SkeletonClone(mesh_tree_tag_p src, int tags_count)
{
    mesh_tree_tag_p ret = (mesh_tree_tag_p)malloc(tags_count * sizeof(mesh_tree_tag_t));

    for(int i = 0; i < tags_count; i++)
    {
        ret[i].mesh_base = src[i].mesh_base;
        ret[i].mesh_skin = src[i].mesh_skin;
        ret[i].flag = src[i].flag;
        vec3_copy(ret[i].offset, src[i].offset);
        ret[i].replace_anim = src[i].replace_anim;
        ret[i].replace_mesh = src[i].replace_mesh;
    }
    return ret;
}

void SkeletonCopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_skin = src[i].mesh_base;
    }
}

vertex_p FindVertexInMesh(base_mesh_p mesh, float v[3])
{
    vertex_p mv = mesh->vertices;
    for(uint32_t i = 0; i < mesh->vertex_count; i++, mv++)
    {
        if(vec3_dist_sq(v, mv->position) < 4.0)
        {
            return mv;
        }
    }

    return NULL;
}

void FillSkinnedMeshMap(skeletal_model_p model)
{
    int8_t *ch;
    float tv[3];
    vertex_p v, rv;
    base_mesh_p mesh_base, mesh_skin;
    mesh_tree_tag_p tree_tag, prev_tree_tag;

    tree_tag = model->mesh_tree;
    for(uint16_t i = 0; i < model->mesh_count; i++, tree_tag++)
    {
        mesh_base = tree_tag->mesh_base;
        mesh_skin = tree_tag->mesh_skin;

        if(!mesh_skin)
        {
            return;
        }

        ch = mesh_skin->skin_map = (int8_t*)malloc(mesh_skin->vertex_count * sizeof(int8_t));
        v = mesh_skin->vertices;
        for(uint32_t k = 0; k < mesh_skin->vertex_count; k++, v++, ch++)
        {
            rv = FindVertexInMesh(mesh_base, v->position);
            if(rv != NULL)
            {
                *ch = 1;
                vec3_copy(v->position, rv->position);
                vec3_copy(v->normal, rv->normal);
            }
            else
            {
                *ch = 0;
                vec3_add(tv, v->position, tree_tag->offset);
                prev_tree_tag = model->mesh_tree;
                for(uint16_t mesh_index = 0; mesh_index < model->mesh_count; mesh_index++, prev_tree_tag++)
                {
                    rv = FindVertexInMesh(prev_tree_tag->mesh_base, tv);
                    if(rv != NULL)
                    {
                        *ch = 2;
                        vec3_sub(v->position, rv->position, tree_tag->offset);
                        vec3_copy(v->normal, rv->normal);
                        break;
                    }
                }
            }
        }
    }
}


void Anim_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, float etr[16])
{
    float cmd_tr[3], tr[3], t;
    ss_bone_tag_p btag = bf->bone_tags;
    bone_tag_p src_btag, next_btag;
    skeletal_model_p model = bf->animations.model;
    bone_frame_p curr_bf, next_bf;

    next_bf = model->animations[bf->animations.next_animation].frames + bf->animations.next_frame;
    curr_bf = model->animations[bf->animations.current_animation].frames + bf->animations.current_frame;

    t = 1.0 - bf->animations.lerp;
    if(etr && (curr_bf->command & ANIM_CMD_MOVE))
    {
        Mat4_vec3_rot_macro(tr, etr, curr_bf->move);
        vec3_mul_scalar(cmd_tr, tr, bf->animations.lerp);
    }
    else
    {
        vec3_set_zero(tr);
        vec3_set_zero(cmd_tr);
    }

    vec3_interpolate_macro(bf->bb_max, curr_bf->bb_max, next_bf->bb_max, bf->animations.lerp, t);
    vec3_add(bf->bb_max, bf->bb_max, cmd_tr);
    vec3_interpolate_macro(bf->bb_min, curr_bf->bb_min, next_bf->bb_min, bf->animations.lerp, t);
    vec3_add(bf->bb_min, bf->bb_min, cmd_tr);
    vec3_interpolate_macro(bf->centre, curr_bf->centre, next_bf->centre, bf->animations.lerp, t);
    vec3_add(bf->centre, bf->centre, cmd_tr);

    vec3_interpolate_macro(bf->pos, curr_bf->pos, next_bf->pos, bf->animations.lerp, t);
    vec3_add(bf->pos, bf->pos, cmd_tr);
    next_btag = next_bf->bone_tags;
    src_btag = curr_bf->bone_tags;
    for(uint16_t k = 0; k < curr_bf->bone_tag_count; k++, btag++, src_btag++, next_btag++)
    {
        vec3_interpolate_macro(btag->offset, src_btag->offset, next_btag->offset, bf->animations.lerp, t);
        vec3_copy(btag->transform+12, btag->offset);
        btag->transform[15] = 1.0;
        if(k == 0)
        {
            vec3_add(btag->transform+12, btag->transform+12, bf->pos);
            vec4_slerp(btag->qrotate, src_btag->qrotate, next_btag->qrotate, bf->animations.lerp);
        }
        else
        {
            bone_tag_p ov_src_btag = src_btag;
            bone_tag_p ov_next_btag = next_btag;
            float ov_lerp = bf->animations.lerp;
            for(ss_animation_p ov_anim = bf->animations.next; ov_anim; ov_anim = ov_anim->next)
            {
                if((ov_anim->model != NULL) && (ov_anim->model->mesh_tree[k].replace_anim != 0))
                {
                    bone_frame_p ov_curr_bf = ov_anim->model->animations[ov_anim->current_animation].frames + ov_anim->current_frame;
                    bone_frame_p ov_next_bf = ov_anim->model->animations[ov_anim->next_animation].frames + ov_anim->next_frame;
                    ov_src_btag = ov_curr_bf->bone_tags + k;
                    ov_next_btag = ov_next_bf->bone_tags + k;
                    ov_lerp = ov_anim->lerp;
                    break;
                }
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
    btag++;
    for(uint16_t k = 1; k < curr_bf->bone_tag_count; k++, btag++)
    {
        Mat4_Mat4_mul(btag->full_transform, btag->parent->full_transform, btag->transform);
    }
}


void Anim_SetAnimation(struct ss_bone_frame_s *bf, int animation, int frame)
{
    animation_frame_p anim = &bf->animations.model->animations[animation];
    bf->animations.lerp = 0.0;
    frame %= anim->frames_count;
    frame = (frame >= 0)?(frame):(anim->frames_count - 1 + frame);
    bf->animations.period = 1.0 / 30.0;

    bf->animations.last_state = anim->state_id;
    bf->animations.next_state = anim->state_id;
    
    bf->animations.current_animation = animation;
    bf->animations.current_frame = frame;
    bf->animations.next_animation = animation;
    bf->animations.next_frame = frame;

    bf->animations.frame_time = (float)frame * bf->animations.period;
    //long int t = (bf->animations.frame_time) / bf->animations.period;
    //float dt = bf->animations.frame_time - (float)t * bf->animations.period;
    bf->animations.frame_time = (float)frame * bf->animations.period;// + dt;
}


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


/*
 * Next frame and next anim calculation function.
 */
void Anim_GetNextFrame(struct ss_bone_frame_s *bf, float time, struct state_change_s *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags)
{
    animation_frame_p curr_anim = bf->animations.model->animations + bf->animations.current_animation;

    *frame = (bf->animations.frame_time + time) / bf->animations.period;
    *frame = (*frame >= 0.0)?(*frame):(0.0);                                    // paranoid checking
    *anim  = bf->animations.current_animation;

    /*
     * Flag has a highest priority
     */
    if(anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(*frame >= curr_anim->frames_count - 1)
        {
            *frame = curr_anim->frames_count - 1;
            *anim  = bf->animations.current_animation;                          // paranoid dublicate
        }
        return;
    }
    else if(anim_flags == ANIM_LOCK)
    {
        *frame = 0;
        *anim  = bf->animations.current_animation;
        return;
    }

    /*
     * State change check
     */
    if(stc != NULL)
    {
        anim_dispatch_p disp = stc->anim_dispatch;
        for(uint16_t i = 0; i < stc->anim_dispatch_count; i++, disp++)
        {
            if((disp->frame_high >= disp->frame_low) && ((*frame >= disp->frame_low) && (*frame <= disp->frame_high) /*|| (bf->animations.current_frame < disp->frame_low) && (*frame > disp->frame_high)*/))
            {
                *anim  = disp->next_anim;
                *frame = disp->next_frame;
                return;                                                         // anim was changed
            }
        }
    }

    /*
     * Check next anim if frame >= frames_count
     */
    if(*frame >= curr_anim->frames_count)
    {
        if(curr_anim->next_anim)
        {
            *frame = curr_anim->next_frame;
            *anim  = curr_anim->next_anim->id;
            return;
        }

        *frame %= curr_anim->frames_count;
        *anim   = bf->animations.current_animation;                             // paranoid dublicate
        return;
    }
}
