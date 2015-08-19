#include <stdlib.h>
#include <math.h>

#include "core/console.h"
#include "core/vmath.h"
#include "core/obb.h"
#include "audio.h"
#include "mesh.h"
#include "entity.h"
#include "render.h"
#include "camera.h"
#include "world.h"
#include "engine.h"
#include "engine_physics.h"
#include "script.h"
#include "gui.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "gameflow.h"
#include "engine_string.h"


entity_p Entity_Create()
{
    entity_p ret = (entity_p)calloc(1, sizeof(entity_t));

    ret->move_type = MOVE_ON_FLOOR;
    Mat4_E(ret->transform);
    ret->state_flags = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    ret->type_flags = ENTITY_TYPE_GENERIC;
    ret->callback_flags = 0x00000000;               // no callbacks by default

    ret->OCB = 0;
    ret->trigger_layout = 0x00;
    ret->timer = 0.0;

    ret->self = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->self->next = NULL;
    ret->self->object = ret;
    ret->self->object_type = OBJECT_ENTITY;
    ret->self->room = NULL;
    ret->self->collision_type = COLLISION_TYPE_KINEMATIC;
    ret->self->collision_shape = COLLISION_SHAPE_TRIMESH;
    ret->obb = OBB_Create();
    ret->obb->transform = ret->transform;

    ret->no_fix_all = 0x00;
    ret->no_fix_skeletal_parts = 0x00000000;
    ret->physics = Physics_CreatePhysicsData(ret->self);

    ret->character = NULL;
    ret->current_sector = NULL;

    ret->bf = (ss_bone_frame_p)malloc(sizeof(ss_bone_frame_t));
    ret->bf->animations.model = NULL;
    ret->bf->animations.onFrame = NULL;
    ret->bf->animations.frame_time = 0.0;
    ret->bf->animations.last_state = 0;
    ret->bf->animations.next_state = 0;
    ret->bf->animations.lerp = 0.0;
    ret->bf->animations.current_animation = 0;
    ret->bf->animations.current_frame = 0;
    ret->bf->animations.next_animation = 0;
    ret->bf->animations.next_frame = 0;
    ret->bf->animations.next = NULL;
    ret->bf->bone_tag_count = 0;
    ret->bf->bone_tags = 0;
    vec3_set_zero(ret->bf->bb_max);
    vec3_set_zero(ret->bf->bb_min);
    vec3_set_zero(ret->bf->centre);
    vec3_set_zero(ret->bf->pos);
    vec3_set_zero(ret->angles);
    vec4_set_zero(ret->speed);
    vec3_set_one(ret->scaling);

    ret->speed_mult = DEFAULT_CHARACTER_SPEED_MULT;
    ret->current_speed = 0.0;

    ret->activation_offset[0] = 0.0;
    ret->activation_offset[1] = 256.0;
    ret->activation_offset[2] = 0.0;
    ret->activation_offset[3] = 128.0;

    return ret;
}


void Entity_Clear(entity_p entity)
{
    if(entity)
    {
        if((entity->self->room != NULL) && (entity != engine_world.Character))
        {
            Room_RemoveEntity(entity->self->room, entity);
        }

        if(entity->obb)
        {
            OBB_Clear(entity->obb);
            free(entity->obb);
            entity->obb = NULL;
        }

        Ragdoll_Delete(entity->physics);
        entity->type_flags &= ~ENTITY_TYPE_DYNAMIC;

        if(entity->character)
        {
            Character_Clean(entity);
        }

        Physics_DeletePhysicsData(entity->physics);
        entity->physics = NULL;

        if(entity->self)
        {
            free(entity->self);
            entity->self = NULL;
        }

        if(entity->bf)
        {
            if(entity->bf->bone_tag_count)
            {
                free(entity->bf->bone_tags);
                entity->bf->bone_tags = NULL;
                entity->bf->bone_tag_count = 0;
            }

            for(ss_animation_p ss_anim=entity->bf->animations.next;ss_anim!=NULL;)
            {
                ss_animation_p ss_anim_next = ss_anim->next;
                ss_anim->next = NULL;
                free(ss_anim);
                ss_anim = ss_anim_next;
            }
            entity->bf->animations.next = NULL;
            free(entity->bf);
            entity->bf = NULL;
        }
    }
}


void Entity_Enable(entity_p ent)
{
    if(!(ent->state_flags & ENTITY_STATE_ENABLED))
    {
        Entity_EnableCollision(ent);
        ent->state_flags |= ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    }
}


void Entity_Disable(entity_p ent)
{
    if(ent->state_flags & ENTITY_STATE_ENABLED)
    {
        Entity_DisableCollision(ent);
        ent->state_flags = 0x0000;
    }
}


void Entity_EnableCollision(entity_p ent)
{
    if(Physics_IsBodyesInited(ent->physics))
    {
        ent->self->collision_type |= 0x0001;
        Physics_EnableCollision(ent->physics);
    }
    else
    {
        ent->self->collision_type = COLLISION_TYPE_KINEMATIC;
        Physics_GenRigidBody(ent->physics, ent->bf, ent->transform);
    }
}


void Entity_DisableCollision(entity_p ent)
{
    if(Physics_IsBodyesInited(ent->physics))
    {
        ent->self->collision_type &= ~0x0001;
        Physics_DisableCollision(ent->physics);
    }
}

void Entity_UpdateRoomPos(entity_p ent)
{
    float pos[3];
    room_p new_room;
    room_sector_p new_sector;

    if(ent->character)
    {
        Mat4_vec3_mul(pos, ent->transform, ent->bf->bone_tags->full_transform+12);
        pos[0] = ent->transform[12+0];
        pos[1] = ent->transform[12+1];
    }
    else
    {
        float v[3];
        vec3_add(v, ent->bf->bb_min, ent->bf->bb_max);
        v[0] /= 2.0;
        v[1] /= 2.0;
        v[2] /= 2.0;
        Mat4_vec3_mul_macro(pos, ent->transform, v);
    }
    new_room = Room_FindPosCogerrence(pos, ent->self->room);
    if(new_room)
    {
        new_sector = Room_GetSectorXYZ(new_room, pos);
        if(new_room != new_sector->owner_room)
        {
            new_room = new_sector->owner_room;
        }

        if(!ent->character && (ent->self->room != new_room))
        {
            if((ent->self->room != NULL) && !Room_IsOverlapped(ent->self->room, new_room))
            {
                if(ent->self->room)
                {
                    Room_RemoveEntity(ent->self->room, ent);
                }
                if(new_room)
                {
                    Room_AddEntity(new_room, ent);
                }
            }
        }

        ent->self->room = new_room;
        ent->last_sector = ent->current_sector;

        if(ent->current_sector != new_sector)
        {
            ent->trigger_layout &= (uint8_t)(~ENTITY_TLAYOUT_SSTATUS); // Reset sector status.
            ent->current_sector = new_sector;
        }
    }
}


void Entity_UpdateTransform(entity_p entity)
{
    float R[4], Rt[4], temp[4];
    float sin_t2, cos_t2, t;
    float *up_dir = entity->transform + 8;                                   // OZ
    float *view_dir = entity->transform + 4;                                 // OY
    float *right_dir = entity->transform + 0;                                // OX
    int i;

    if(entity->character != NULL)
    {
        Entity_GhostUpdate(entity);
    }
    i = entity->angles[0] / 360.0;
    i = (entity->angles[0] < 0.0)?(i-1):(i);
    entity->angles[0] -= 360.0 * i;

    i = entity->angles[1] / 360.0;
    i = (entity->angles[1] < 0.0)?(i-1):(i);
    entity->angles[1] -= 360.0 * i;

    i = entity->angles[2] / 360.0;
    i = (entity->angles[2] < 0.0)?(i-1):(i);
    entity->angles[2] -= 360.0 * i;

    t = entity->angles[0] * M_PI / 180.0;
    sin_t2 = sin(t);
    cos_t2 = cos(t);

    /*
     * LEFT - RIGHT INIT
     */

    view_dir[0] =-sin_t2;                                                       // OY - view
    view_dir[1] = cos_t2;
    view_dir[2] = 0.0;
    view_dir[3] = 0.0;

    right_dir[0] = cos_t2;                                                      // OX - right
    right_dir[1] = sin_t2;
    right_dir[2] = 0.0;
    right_dir[3] = 0.0;

    up_dir[0] = 0.0;                                                            // OZ - up
    up_dir[1] = 0.0;
    up_dir[2] = 1.0;
    up_dir[3] = 0.0;

    if(entity->angles[1] != 0.0)
    {
        t = entity->angles[1] * M_PI / 360.0;                                   // UP - DOWN
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        R[3] = cos_t2;
        R[0] = right_dir[0] * sin_t2;
        R[1] = right_dir[1] * sin_t2;
        R[2] = right_dir[2] * sin_t2;
        vec4_sop(Rt, R);

        vec4_mul(temp, R, up_dir);
        vec4_mul(up_dir, temp, Rt);
        vec4_mul(temp, R, view_dir);
        vec4_mul(view_dir, temp, Rt);
    }

    if(entity->angles[2] != 0.0)
    {
        t = entity->angles[2] * M_PI / 360.0;                                   // ROLL
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        R[3] = cos_t2;
        R[0] = view_dir[0] * sin_t2;
        R[1] = view_dir[1] * sin_t2;
        R[2] = view_dir[2] * sin_t2;
        vec4_sop(Rt, R);

        vec4_mul(temp, R, right_dir);
        vec4_mul(right_dir, temp, Rt);
        vec4_mul(temp, R, up_dir);
        vec4_mul(up_dir, temp, Rt);
    }

    view_dir[3] = 0.0;
    right_dir[3] = 0.0;
    up_dir[3] = 0.0;

    if(entity->character != NULL)
    {
        Entity_FixPenetrations(entity, NULL);
    }
}


void Entity_UpdateCurrentSpeed(entity_p entity, int zeroVz)
{
    float t  = entity->current_speed * entity->speed_mult;
    float vz = (zeroVz)?(0.0):(entity->speed[2]);

    if(entity->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(entity->speed, entity->transform+4, t);
    }
    else if(entity->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(entity->speed, entity->transform+4,-t);
    }
    else if(entity->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(entity->speed, entity->transform+0,-t);
    }
    else if(entity->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(entity->speed, entity->transform+0, t);
    }
    else
    {
        vec3_set_zero(entity->speed);
    }

    entity->speed[2] = vz;
}


void Entity_AddOverrideAnim(struct entity_s *ent, int model_id)
{
    skeletal_model_p sm = World_GetModelByID(&engine_world, model_id);

    if((sm != NULL) && (sm->mesh_count == ent->bf->bone_tag_count))
    {
        ss_animation_p ss_anim = (ss_animation_p)malloc(sizeof(ss_animation_t));

        ss_anim->model = sm;
        ss_anim->onFrame = NULL;
        ss_anim->next = ent->bf->animations.next;
        ent->bf->animations.next = ss_anim;

        ss_anim->frame_time = 0.0;
        ss_anim->next_state = 0;
        ss_anim->lerp = 0.0;
        ss_anim->current_animation = 0;
        ss_anim->current_frame = 0;
        ss_anim->next_animation = 0;
        ss_anim->next_frame = 0;
        ss_anim->period = 1.0 / 30.0;;
    }
}


void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, float etr[16])
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
    for(uint16_t k=0;k<curr_bf->bone_tag_count;k++,btag++,src_btag++,next_btag++)
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
            for(ss_animation_p ov_anim=bf->animations.next;ov_anim!=NULL;ov_anim = ov_anim->next)
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
    for(uint16_t k=1;k<curr_bf->bone_tag_count;k++,btag++)
    {
        Mat4_Mat4_mul(btag->full_transform, btag->parent->full_transform, btag->transform);
    }
}


void Entity_UpdateRigidBody(struct entity_s *ent, int force)
{
    if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
    {
        float tr[16];
        Physics_GetBodyWorldTransform(ent->physics, ent->transform, 0);
        Entity_UpdateRoomPos(ent);
        Mat4_E(ent->bf->bone_tags[0].full_transform);
        for(uint16_t i=1;i<ent->bf->bone_tag_count;i++)
        {
            Physics_GetBodyWorldTransform(ent->physics, tr, i);
            Mat4_inv_Mat4_affine_mul(ent->bf->bone_tags[i].full_transform, ent->transform, tr);
        }

        // fill bone frame transformation matrices;
        for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
        {
            if(ent->bf->bone_tags[i].parent != NULL)
            {
                Mat4_inv_Mat4_affine_mul(ent->bf->bone_tags[i].transform, ent->bf->bone_tags[i].parent->full_transform, ent->bf->bone_tags[i].full_transform);
            }
            else
            {
                Mat4_Copy(ent->bf->bone_tags[i].transform, ent->bf->bone_tags[i].full_transform);
            }
        }

        // recalculate visibility box
        if(ent->bf->bone_tag_count == 1)
        {
            vec3_copy(ent->bf->bb_min, ent->bf->bone_tags[0].mesh_base->bb_min);
            vec3_copy(ent->bf->bb_max, ent->bf->bone_tags[0].mesh_base->bb_max);
        }
        else
        {
            vec3_copy(ent->bf->bb_min, ent->bf->bone_tags[0].mesh_base->bb_min);
            vec3_copy(ent->bf->bb_max, ent->bf->bone_tags[0].mesh_base->bb_max);
            for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
            {
                float *pos = ent->bf->bone_tags[i].full_transform + 12;
                float *bb_min = ent->bf->bone_tags[i].mesh_base->bb_min;
                float *bb_max = ent->bf->bone_tags[i].mesh_base->bb_max;
                float r = bb_max[0] - bb_min[0];
                float t = bb_max[1] - bb_min[1];
                r = (t > r)?(t):(r);
                t = bb_max[2] - bb_min[2];
                r = (t > r)?(t):(r);
                r *= 0.5;

                if(ent->bf->bb_min[0] > pos[0] - r)
                {
                    ent->bf->bb_min[0] = pos[0] - r;
                }
                if(ent->bf->bb_min[1] > pos[1] - r)
                {
                    ent->bf->bb_min[1] = pos[1] - r;
                }
                if(ent->bf->bb_min[2] > pos[2] - r)
                {
                    ent->bf->bb_min[2] = pos[2] - r;
                }

                if(ent->bf->bb_max[0] < pos[0] + r)
                {
                    ent->bf->bb_max[0] = pos[0] + r;
                }
                if(ent->bf->bb_max[1] < pos[1] + r)
                {
                    ent->bf->bb_max[1] = pos[1] + r;
                }
                if(ent->bf->bb_max[2] < pos[2] + r)
                {
                    ent->bf->bb_max[2] = pos[2] + r;
                }
            }
        }
    }
    else
    {
        if((ent->bf->animations.model == NULL) || !Physics_IsBodyesInited(ent->physics) ||
           ((force == 0) && (ent->bf->animations.model->animation_count == 1) && (ent->bf->animations.model->animations->frames_count == 1)))
        {
            return;
        }

        Entity_UpdateRoomPos(ent);
        if(ent->self->collision_type & 0x0001)
        //if(ent->self->collision_type != COLLISION_TYPE_STATIC)
        {
            float tr[16];
            for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
            {
                Mat4_Mat4_mul(tr, ent->transform, ent->bf->bone_tags[i].full_transform);
                Physics_SetBodyWorldTransform(ent->physics, tr, i);
            }
        }
    }

    /*if(Physics_IsGhostsInited(ent->physics))
    {
        float v[3], tr[16];
        for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
        {
            Physics_GetBodyWorldTransform(ent->physics, tr, i);
            Mat4_vec3_mul(v, tr, ent->bf->bone_tags[i].mesh_base->centre);
            vec3_copy(tr+12, v);
            Physics_SetGhostWorldTransform(ent->physics, tr, i);
        }
    }*/

    Entity_RebuildBV(ent);
}


void Entity_GhostUpdate(struct entity_s *ent)
{
    if(Physics_IsGhostsInited(ent->physics))
    {
        /*if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
        {
            float tr[16], pos[3], *v;
            for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
            {
                Mat4_Mat4_mul(tr, ent->transform, ent->bf->bone_tags[i].full_transform);
                v = ent->bf->animations.model->mesh_tree[i].mesh_base->centre;
                Mat4_vec3_mul_macro(pos, tr, v);
                vec3_copy(tr + 12, v);
                Physics_SetGhostWorldTransform(ent->physics, tr, i);
            }
        }
        else*/
        {
            float tr[16], v[3];
            for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
            {
                Physics_GetBodyWorldTransform(ent->physics, tr, i);
                Mat4_vec3_mul(v, tr, ent->bf->bone_tags[i].mesh_base->centre);
                vec3_copy(tr + 12, v);
                Physics_SetGhostWorldTransform(ent->physics, tr, i);
            }
        }
    }
}


///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity_GetPenetrationFixVector(struct entity_s *ent, float reaction[3], float move_global[3])
{
    int ret = 0;

    vec3_set_zero(reaction);
    if(Physics_IsGhostsInited(ent->physics) && (ent->no_fix_all == 0x00))
    {
        float tmp[3], orig_pos[3];
        float tr[16];

        vec3_copy(orig_pos, ent->transform + 12);
        for(uint16_t i=0;i<ent->bf->animations.model->collision_map_size;i++)
        {
            //btTransform tr_current;
            float from[3], to[3], curr[3], move[3], move_len;
            uint16_t m = ent->bf->animations.model->collision_map[i];
            ss_bone_tag_p btag = ent->bf->bone_tags + m;

            if(btag->body_part & ent->no_fix_skeletal_parts)
            {
                continue;
            }

            // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
            if((btag->parent == NULL) || ((move_global != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
            {
                Physics_GetGhostWorldTransform(ent->physics, tr, m);
                from[0] = tr[12 + 0] + ent->transform[12+0] - orig_pos[0];
                from[1] = tr[12 + 1] + ent->transform[12+1] - orig_pos[1];
                from[2] = tr[12 + 2] + ent->transform[12+2] - orig_pos[2];
            }
            else
            {
                float parent_from[3];
                Mat4_vec3_mul(parent_from, btag->parent->full_transform, btag->parent->mesh_base->centre);
                Mat4_vec3_mul(from, ent->transform, parent_from);
            }

            Mat4_Mat4_mul(tr, ent->transform, btag->full_transform);
            Mat4_vec3_mul(to, tr, btag->mesh_base->centre);
            vec3_copy(curr, from);
            vec3_sub(move, to, from);
            move_len = vec3_abs(move);
            if((i == 0) && (move_len > 1024.0))                                 ///@FIXME: magick const 1024.0!
            {
                break;
            }
            int iter = (float)(2.0 * move_len / btag->mesh_base->R) + 1;     ///@FIXME (not a critical): magick const 4.0!
            move[0] /= (float)iter;
            move[1] /= (float)iter;
            move[2] /= (float)iter;

            for(int j=0;j<=iter;j++)
            {
                vec3_copy(tr+12, curr);
                Physics_SetGhostWorldTransform(ent->physics, tr, m);
                if(Physics_GetGhostPenetrationFixVector(ent->physics, m, tmp))
                {
                    vec3_add_to(ent->transform + 12, tmp);
                    vec3_add_to(curr, tmp);
                    vec3_add_to(from, tmp);
                    ret++;
                }
                vec3_add_to(curr, move);
            }
        }
        vec3_sub(reaction, ent->transform+12, orig_pos);
        vec3_copy(ent->transform + 12, orig_pos);
    }

    return ret;
}


/**
 * we check walls and other collision objects reaction. if reaction more then critacal
 * then cmd->horizontal_collide |= 0x01;
 * @param ent - cheked entity
 * @param cmd - here we fill cmd->horizontal_collide field
 * @param move - absolute 3d move vector
 */
int Entity_CheckNextPenetration(struct entity_s *ent, float move[3])
{
    int ret = 0;
    if(Physics_IsGhostsInited(ent->physics))
    {
        float t1, t2, reaction[3], *pos = ent->transform + 12;

        Entity_GhostUpdate(ent);
        vec3_add(pos, pos, move);
        //resp->horizontal_collide = 0x00;
        ret = Entity_GetPenetrationFixVector(ent, reaction, move);
        if((ret > 0) && (ent->character != NULL))
        {
            t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
            t2 = move[0] * move[0] + move[1] * move[1];
            if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))
            {
                t2 *= t1;
                t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
                if(t1 < ent->character->critical_wall_component)
                {
                    ent->character->resp.horizontal_collide |= 0x01;
                }
            }
        }
        vec3_sub(pos, pos, move);
        Entity_GhostUpdate(ent);
    }

    return ret;
}


void Entity_FixPenetrations(struct entity_s *ent, float move[3])
{
    if(Physics_IsGhostsInited(ent->physics))
    {
        float t1, t2, reaction[3];

        if((move != NULL) && (ent->character != NULL))
        {
            ent->character->resp.horizontal_collide    = 0x00;
            ent->character->resp.vertical_collide      = 0x00;
        }

        if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
        {
            return;
        }

        if(ent->no_fix_all)
        {
            Entity_GhostUpdate(ent);
            return;
        }

        int numPenetrationLoops = Entity_GetPenetrationFixVector(ent, reaction, move);
        vec3_add(ent->transform+12, ent->transform+12, reaction);

        if(ent->character != NULL)
        {
            Character_UpdateCurrentHeight(ent);
            if((move != NULL) && (numPenetrationLoops > 0))
            {
                t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
                t2 = move[0] * move[0] + move[1] * move[1];
                if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))    // we have horizontal move and horizontal correction
                {
                    t2 *= t1;
                    t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
                    if(t1 < ent->character->critical_wall_component)
                    {
                        ent->character->resp.horizontal_collide |= 0x01;
                    }
                }
                else if((reaction[2] * reaction[2] > t1) && (move[2] * move[2] > t2))
                {
                    if((reaction[2] > 0.0) && (move[2] < 0.0))
                    {
                        ent->character->resp.vertical_collide |= 0x01;
                    }
                    else if((reaction[2] < 0.0) && (move[2] > 0.0))
                    {
                        ent->character->resp.vertical_collide |= 0x02;
                    }
                }
            }

            if(ent->character->height_info.ceiling_hit && (reaction[2] < -0.1))
            {
                ent->character->resp.vertical_collide |= 0x02;
            }

            if(ent->character->height_info.floor_hit && (reaction[2] > 0.1))
            {
                ent->character->resp.vertical_collide |= 0x01;
            }
        }

        Entity_GhostUpdate(ent);
    }
}


int  Entity_GetSubstanceState(entity_p entity)
{
    if((!entity) || (!entity->character))
    {
        return 0;
    }

    if(entity->self->room->flags & TR_ROOM_FLAG_QUICKSAND)
    {
        if(entity->character->height_info.transition_level > entity->transform[12 + 2] + entity->character->Height)
        {
            return ENTITY_SUBSTANCE_QUICKSAND_CONSUMED;
        }
        else
        {
            return ENTITY_SUBSTANCE_QUICKSAND_SHALLOW;
        }
    }
    else if(!entity->character->height_info.water)
    {
        return ENTITY_SUBSTANCE_NONE;
    }
    else if( entity->character->height_info.water &&
            (entity->character->height_info.transition_level > entity->transform[12 + 2]) &&
            (entity->character->height_info.transition_level < entity->transform[12 + 2] + entity->character->wade_depth) )
    {
        return ENTITY_SUBSTANCE_WATER_SHALLOW;
    }
    else if( entity->character->height_info.water &&
            (entity->character->height_info.transition_level > entity->transform[12 + 2] + entity->character->wade_depth) )
    {
        return ENTITY_SUBSTANCE_WATER_WADE;
    }
    else
    {
        return ENTITY_SUBSTANCE_WATER_SWIM;
    }
}

float Entity_FindDistance(entity_p entity_1, entity_p entity_2)
{
    float *v1 = entity_1->transform + 12;
    float *v2 = entity_2->transform + 12;

    return vec3_dist(v1, v2);
}


void Entity_CheckCollisionCallbacks(entity_p ent)
{
    collision_node_p cn = Physics_GetCurrentCollisions(ent->physics);
    for(;cn;cn=cn->next)
    {
        // do callbacks here:
        if(cn->obj->object_type == OBJECT_ENTITY)
        {
            entity_p activator = (entity_p)cn->obj->object;

            if(activator->callback_flags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                lua_ExecEntity(engine_lua, ENTITY_CALLBACK_COLLISION, activator->id, ent->id);
                Con_Printf("collider_bone_index = %d, collider_type = %d", cn->part_self, cn->obj->object_type);
            }
        }
    }
}


void Entity_DoAnimCommands(entity_p entity, struct ss_animation_s *ss_anim, int changing)
{
    if((engine_world.anim_commands_count == 0) || (ss_anim->model == NULL))
    {
        return;  // If no anim commands
    }

    animation_frame_p af  = ss_anim->model->animations + ss_anim->current_animation;
    if(af->num_anim_commands <= 255)
    {
        uint32_t count        = af->num_anim_commands;
        int16_t *pointer      = engine_world.anim_commands + af->anim_command;
        int8_t   random_value = 0;

        for(uint32_t i = 0; i < count; i++, pointer++)
        {
            switch(*pointer)
            {
                case TR_ANIMCOMMAND_SETPOSITION:
                    // This command executes ONLY at the end of animation.
                    pointer += 3; // Parse through 3 operands.
                    break;

                case TR_ANIMCOMMAND_JUMPDISTANCE:
                    // This command executes ONLY at the end of animation.
                    pointer += 2; // Parse through 2 operands.
                    break;

                case TR_ANIMCOMMAND_EMPTYHANDS:
                    ///@FIXME: Behaviour is yet to be discovered.
                    break;

                case TR_ANIMCOMMAND_KILL:
                    // This command executes ONLY at the end of animation.
                    if(ss_anim->current_frame == af->frames_count - 1)
                    {
                        if(entity->character)
                        {
                            entity->character->resp.kill = 1;
                        }
                    }

                    break;

                case TR_ANIMCOMMAND_PLAYSOUND:
                    int16_t sound_index;

                    if(ss_anim->current_frame == *++pointer)
                    {
                        sound_index = *++pointer & 0x3FFF;

                        // Quick workaround for TR3 quicksand.
                        if((Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_QUICKSAND_CONSUMED) ||
                           (Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_QUICKSAND_SHALLOW)   )
                        {
                            sound_index = 18;
                        }

                        if(*pointer & TR_ANIMCOMMAND_CONDITION_WATER)
                        {
                            if(Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_WATER_SHALLOW)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }
                        else if(*pointer & TR_ANIMCOMMAND_CONDITION_LAND)
                        {
                            if(Entity_GetSubstanceState(entity) != ENTITY_SUBSTANCE_WATER_SHALLOW)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }
                        else
                        {
                            Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }

                    }
                    else
                    {
                        pointer++;
                    }
                    break;

                case TR_ANIMCOMMAND_PLAYEFFECT:
                    // Effects (flipeffects) are various non-typical actions which vary
                    // across different TR game engine versions. There are common ones,
                    // however, and currently only these are supported.
                    if(ss_anim->current_frame == *++pointer)
                    {
                        switch(*++pointer & 0x3FFF)
                        {
                            case TR_EFFECT_SHAKESCREEN:
                                if(engine_world.Character)
                                {
                                    float dist = Entity_FindDistance(engine_world.Character, entity);
                                    dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):((TR_CAM_MAX_SHAKE_DISTANCE - dist) / 1024.0);
                                    if(dist > 0)
                                        Cam_Shake(&engine_camera, (dist * TR_CAM_DEFAULT_SHAKE_POWER), 0.5);
                                }
                                break;

                            case TR_EFFECT_CHANGEDIRECTION:
                                break;

                            case TR_EFFECT_HIDEOBJECT:
                                entity->state_flags &= ~ENTITY_STATE_VISIBLE;
                                break;

                            case TR_EFFECT_SHOWOBJECT:
                                entity->state_flags |= ENTITY_STATE_VISIBLE;
                                break;

                            case TR_EFFECT_PLAYSTEPSOUND:
                                // Please note that we bypass land/water mask, as TR3-5 tends to ignore
                                // this flag and play step sound in any case on land, ignoring it
                                // completely in water rooms.
                                if(!Entity_GetSubstanceState(entity))
                                {
                                    // TR3-5 footstep map.
                                    // We define it here as a magic numbers array, because TR3-5 versions
                                    // fortunately have no differences in footstep sounds order.
                                    // Also note that some footstep types mutually share same sound IDs
                                    // across different TR versions.
                                    switch(entity->current_sector->material)
                                    {
                                        case SECTOR_MATERIAL_MUD:
                                            Audio_Send(288, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_SNOW:  // TR3 & TR5 only
                                            if(engine_world.version != TR_IV)
                                            {
                                                Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_SAND:  // Same as grass
                                            Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_GRAVEL:
                                            Audio_Send(290, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_ICE:   // TR3 & TR5 only
                                            if(engine_world.version != TR_IV)
                                            {
                                                Audio_Send(289, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_WATER: // BYPASS!
                                            // Audio_Send(17, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_STONE: // DEFAULT SOUND, BYPASS!
                                            // Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_WOOD:
                                            Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_METAL:
                                            Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_MARBLE:    // TR4 only
                                            if(engine_world.version == TR_IV)
                                            {
                                                Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_GRASS:     // Same as sand
                                            Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_CONCRETE:  // DEFAULT SOUND, BYPASS!
                                            Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_OLDWOOD:   // Same as wood
                                            Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_OLDMETAL:  // Same as metal
                                            Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;
                                    }
                                }
                                break;

                            case TR_EFFECT_BUBBLE:
                                ///@FIXME: Spawn bubble particle here, when particle system is developed.
                                random_value = rand() % 100;
                                if(random_value > 60)
                                {
                                    Audio_Send(TR_AUDIO_SOUND_BUBBLE, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                }
                                break;

                            default:
                                ///@FIXME: TODO ALL OTHER EFFECTS!
                                break;
                        }
                    }
                    else
                    {
                        pointer++;
                    }
                    break;
            }
        }
    }
}


void Entity_ProcessSector(entity_p ent)
{
    if(!ent->current_sector) return;

    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    room_sector_p highest_sector = Sector_GetHighest(ent->current_sector);
    room_sector_p lowest_sector  = Sector_GetLowest(ent->current_sector);

    if(ent->character)
    {
        ent->character->height_info.walls_climb_dir  = 0;
        ent->character->height_info.walls_climb_dir |= lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST  |
                                                                               SECTOR_FLAG_CLIMB_EAST  |
                                                                               SECTOR_FLAG_CLIMB_NORTH |
                                                                               SECTOR_FLAG_CLIMB_SOUTH );

        ent->character->height_info.walls_climb     = (ent->character->height_info.walls_climb_dir > 0);
        ent->character->height_info.ceiling_climb   = 0x00;

        if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
        {
            ent->character->height_info.ceiling_climb = 0x01;
        }

        if(lowest_sector->flags & SECTOR_FLAG_DEATH)
        {
            if((ent->move_type == MOVE_ON_FLOOR)    ||
               (ent->move_type == MOVE_UNDERWATER) ||
               (ent->move_type == MOVE_WADE)        ||
               (ent->move_type == MOVE_ON_WATER)    ||
               (ent->move_type == MOVE_QUICKSAND))
            {
                Character_SetParam(ent, PARAM_HEALTH, 0.0);
                ent->character->resp.kill = 1;
            }
        }
    }

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if((ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR) || (ent->type_flags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.

        int top = lua_gettop(engine_lua);
        lua_getglobal(engine_lua, "tlist_RunTrigger");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_pushnumber(engine_lua, lowest_sector->trig_index);
            lua_pushnumber(engine_lua, ((ent->bf->animations.model->id == 0) ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC));
            lua_pushnumber(engine_lua, ent->id);
            lua_CallAndLog(engine_lua, 3, 1, 0);
        }
        lua_settop(engine_lua, top);
    }
}


void Entity_SetAnimation(entity_p entity, int animation, int frame, int another_model)
{
    if(!entity || !entity->bf->animations.model || (animation >= entity->bf->animations.model->animation_count))
    {
        return;
    }

    animation = (animation < 0)?(0):(animation);
    entity->no_fix_all = 0x00;

    if(another_model >= 0)
    {
        skeletal_model_p model = World_GetModelByID(&engine_world, another_model);
        if((!model) || (animation >= model->animation_count)) return;
        entity->bf->animations.model = model;
    }

    animation_frame_p anim = &entity->bf->animations.model->animations[animation];

    entity->bf->animations.lerp = 0.0;
    frame %= anim->frames_count;
    frame = (frame >= 0)?(frame):(anim->frames_count - 1 + frame);
    entity->bf->animations.period = 1.0 / 30.0;

    entity->bf->animations.last_state = anim->state_id;
    entity->bf->animations.next_state = anim->state_id;
    entity->current_speed = anim->speed_x;
    entity->bf->animations.current_animation = animation;
    entity->bf->animations.current_frame = frame;
    entity->bf->animations.next_animation = animation;
    entity->bf->animations.next_frame = frame;

    entity->bf->animations.frame_time = (float)frame * entity->bf->animations.period;
    //long int t = (entity->bf->animations.frame_time) / entity->bf->animations.period;
    //float dt = entity->bf->animations.frame_time - (float)t * entity->bf->animations.period;
    entity->bf->animations.frame_time = (float)frame * entity->bf->animations.period;// + dt;

    Entity_UpdateCurrentBoneFrame(entity->bf, entity->transform);
    Entity_UpdateRigidBody(entity, 0);
}


struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim)
{
    if(state_change_anim >= 0)
    {
        state_change_p ret = anim->state_change;
        for(uint16_t i=0;i<anim->state_change_count;i++,ret++)
        {
            for(uint16_t j=0;j<ret->anim_dispatch_count;j++)
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
    for(uint16_t i=0;i<anim->state_change_count;i++,ret++)
    {
        if(ret->id == id)
        {
            return ret;
        }
    }

    return NULL;
}


int Entity_GetAnimDispatchCase(struct entity_s *entity, uint32_t id)
{
    animation_frame_p anim = entity->bf->animations.model->animations + entity->bf->animations.current_animation;
    state_change_p stc = anim->state_change;

    for(uint16_t i=0;i<anim->state_change_count;i++,stc++)
    {
        if(stc->id == id)
        {
            anim_dispatch_p disp = stc->anim_dispatch;
            for(uint16_t j=0;j<stc->anim_dispatch_count;j++,disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (entity->bf->animations.current_frame >= disp->frame_low) && (entity->bf->animations.current_frame <= disp->frame_high))// ||
                   //(disp->frame_high <  disp->frame_low) && ((entity->bf->current_frame >= disp->frame_low) || (entity->bf->current_frame <= disp->frame_high)))
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
void Entity_GetNextFrame(struct ss_bone_frame_s *bf, float time, struct state_change_s *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags)
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

    /*
     * State change check
     */
    if(stc != NULL)
    {
        anim_dispatch_p disp = stc->anim_dispatch;
        for(uint16_t i=0;i<stc->anim_dispatch_count;i++,disp++)
        {
            if((disp->frame_high >= disp->frame_low) && (*frame >= disp->frame_low) && (*frame <= disp->frame_high))
            {
                *anim  = disp->next_anim;
                *frame = disp->next_frame;
                //*frame = (disp->next_frame + (*frame - disp->frame_low)) % bf->model->animations[disp->next_anim].frames_count;
                return;                                                         // anim was changed
            }
        }
    }
}


void Entity_DoAnimMove(entity_p entity, int16_t *anim, int16_t *frame)
{
    if(entity->bf->animations.model != NULL)
    {
        animation_frame_p curr_af = entity->bf->animations.model->animations + entity->bf->animations.current_animation;
        bone_frame_p curr_bf = curr_af->frames + entity->bf->animations.current_frame;

        if(curr_bf->command & ANIM_CMD_JUMP)
        {
            Character_SetToJump(entity, -curr_bf->v_Vertical, curr_bf->v_Horizontal);
        }
        if(curr_bf->command & ANIM_CMD_CHANGE_DIRECTION)
        {
            //Con_Printf("ROTATED: anim = %d, frame = %d of %d", entity->bf->animations.current_animation, entity->bf->animations.current_frame, entity->bf->animations.model->animations[entity->bf->animations.current_animation].frames_count);
            entity->angles[0] += 180.0;
            if(entity->move_type == MOVE_UNDERWATER)
            {
                entity->angles[1] = -entity->angles[1];                         // for underwater case
            }
            if(entity->dir_flag == ENT_MOVE_BACKWARD)
            {
                entity->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(entity->dir_flag == ENT_MOVE_FORWARD)
            {
                entity->dir_flag = ENT_MOVE_BACKWARD;
            }
            Entity_UpdateTransform(entity);
            Entity_SetAnimation(entity, curr_af->next_anim->id, curr_af->next_frame);
            *anim = entity->bf->animations.current_animation;
            *frame = entity->bf->animations.current_frame;
        }
        if(curr_bf->command & ANIM_CMD_MOVE)
        {
            float tr[3];
            Mat4_vec3_rot_macro(tr, entity->transform, curr_bf->move);
            vec3_add(entity->transform+12, entity->transform+12, tr);
        }
    }
}

void Character_DoWeaponFrame(entity_p entity, float time);

/**
 * In original engine (+ some information from anim_commands) the anim_commands implement in beginning of frame
 */
///@TODO: rewrite as a cycle through all bf.animations list
int Entity_Frame(entity_p entity, float time)
{
    int16_t frame, anim, ret = 0x00;
    long int t;
    float dt;
    animation_frame_p af;
    state_change_p stc;
    ss_animation_p ss_anim;

    if((entity == NULL) || (entity->type_flags & ENTITY_TYPE_DYNAMIC) || !(entity->state_flags & ENTITY_STATE_ACTIVE)  || !(entity->state_flags & ENTITY_STATE_ENABLED) ||
       (entity->bf->animations.model == NULL) || ((entity->bf->animations.model->animation_count == 1) && (entity->bf->animations.model->animations->frames_count == 1)))
    {
        return 0;
    }

    if(entity->bf->animations.anim_flags & ANIM_LOCK) return 1;                  // penetration fix will be applyed in Character_Move... functions

    ss_anim = &entity->bf->animations;

    Entity_GhostUpdate(entity);

    entity->bf->animations.lerp = 0.0;
    stc = Anim_FindStateChangeByID(ss_anim->model->animations + ss_anim->current_animation, ss_anim->next_state);
    Entity_GetNextFrame(entity->bf, time, stc, &frame, &anim, ss_anim->anim_flags);
    if(ss_anim->current_animation != anim)
    {
        ss_anim->last_animation = ss_anim->current_animation;

        ret = 0x02;
        Entity_DoAnimCommands(entity, &entity->bf->animations, ret);
        Entity_DoAnimMove(entity, &anim, &frame);

        Entity_SetAnimation(entity, anim, frame);
        stc = Anim_FindStateChangeByID(ss_anim->model->animations + ss_anim->current_animation, ss_anim->next_state);
    }
    else if(ss_anim->current_frame != frame)
    {
        if(ss_anim->current_frame == 0)
        {
            ss_anim->last_animation = ss_anim->current_animation;
        }

        ret = 0x01;
        Entity_DoAnimCommands(entity, &entity->bf->animations, ret);
        Entity_DoAnimMove(entity, &anim, &frame);
    }

    af = entity->bf->animations.model->animations + entity->bf->animations.current_animation;
    entity->bf->animations.frame_time += time;

    t = (entity->bf->animations.frame_time) / entity->bf->animations.period;
    dt = entity->bf->animations.frame_time - (float)t * entity->bf->animations.period;
    entity->bf->animations.frame_time = (float)frame * entity->bf->animations.period + dt;
    entity->bf->animations.lerp = dt / entity->bf->animations.period;
    Entity_GetNextFrame(entity->bf, entity->bf->animations.period, stc, &entity->bf->animations.next_frame, &entity->bf->animations.next_animation, ss_anim->anim_flags);

    // Update acceleration.
    // With variable framerate, we don't know when we'll reach final
    // frame for sure, so we use native frame number check to increase acceleration.

    if((entity->character) && (ss_anim->current_frame != frame))
    {

        // NB!!! For Lara, we update ONLY X-axis speed/accel.

        if((af->accel_x == 0) || (frame < entity->bf->animations.current_frame))
        {
            entity->current_speed  = af->speed_x;
        }
        else
        {
            entity->current_speed += af->accel_x;
        }
    }

    entity->bf->animations.current_frame = frame;


    Character_DoWeaponFrame(entity, time);

    if(entity->bf->animations.onFrame != NULL)
    {
        entity->bf->animations.onFrame(entity, &entity->bf->animations, ret);
    }

    Entity_UpdateCurrentBoneFrame(entity->bf, entity->transform);
    if(entity->character != NULL)
    {
        Entity_FixPenetrations(entity, NULL);
    }

    return ret;
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity_RebuildBV(entity_p ent)
{
    if((ent != NULL) && (ent->bf->animations.model != NULL))
    {
        /*
         * get current BB from animation
         */
        OBB_Rebuild(ent->obb, ent->bf->bb_min, ent->bf->bb_max);
        OBB_Transform(ent->obb);
    }
}


void Entity_CheckActivators(struct entity_s *ent)
{
    if((ent != NULL) && (ent->self->room != NULL))
    {
        float ppos[3];

        ppos[0] = ent->transform[12+0] + ent->transform[4+0] * ent->bf->bb_max[1];
        ppos[1] = ent->transform[12+1] + ent->transform[4+1] * ent->bf->bb_max[1];
        ppos[2] = ent->transform[12+2] + ent->transform[4+2] * ent->bf->bb_max[1];
        engine_container_p cont = ent->self->room->containers;
        for(;cont;cont=cont->next)
        {
            if((cont->object_type == OBJECT_ENTITY) && (cont->object))
            {
                entity_p e = (entity_p)cont->object;
                float r = e->activation_offset[3];
                r *= r;
                if((e->type_flags & ENTITY_TYPE_INTERACTIVE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
                    if((e != ent) && (OBB_OBB_Test(e->obb, ent->obb) == 1))//(vec3_dist_sq(ent->transform+12, pos) < r))
                    {
                        lua_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->id, ent->id);
                    }
                }
                else if((e->type_flags & ENTITY_TYPE_PICKABLE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    float *v = e->transform + 12;
                    if((e != ent) && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r) &&
                                      (v[2] + 32.0 > ent->transform[12+2] + ent->bf->bb_min[2]) && (v[2] - 32.0 < ent->transform[12+2] + ent->bf->bb_max[2]))
                    {
                        lua_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->id, ent->id);
                    }
                }
            }
        }
    }
}


void Entity_MoveForward(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[4] * dist;
    ent->transform[13] += ent->transform[5] * dist;
    ent->transform[14] += ent->transform[6] * dist;
}


void Entity_MoveStrafe(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[0] * dist;
    ent->transform[13] += ent->transform[1] * dist;
    ent->transform[14] += ent->transform[2] * dist;
}


void Entity_MoveVertical(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[8] * dist;
    ent->transform[13] += ent->transform[9] * dist;
    ent->transform[14] += ent->transform[10] * dist;
}


/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Character_DoWeaponFrame(entity_p entity, float time)
{
    if(entity->character != NULL)
    {
        /* anims (TR_I - TR_V):
         * pistols:
         * 0: idle to fire;
         * 1: draw weapon (short?);
         * 2: draw weapon (full);
         * 3: fire process;
         *
         * shotgun, rifles, crossbow, harpoon, launchers (2 handed weapons):
         * 0: idle to fire;
         * 1: draw weapon;
         * 2: fire process;
         * 3: hide weapon;
         * 4: idle to fire (targeted);
         */
        if((entity->character->cmd.ready_weapon != 0x00) && (entity->character->current_weapon > 0) && (entity->character->weapon_current_state == WEAPON_STATE_HIDE))
        {
            Character_SetWeaponModel(entity, entity->character->current_weapon, 1);
        }

        float dt;
        int t;

        for(ss_animation_p ss_anim=entity->bf->animations.next;ss_anim!=NULL;ss_anim=ss_anim->next)
        {
            if((ss_anim->model != NULL) && (ss_anim->model->animation_count > 4))
            {
                switch(entity->character->weapon_current_state)
                {
                    case WEAPON_STATE_HIDE:
                        if(entity->character->cmd.ready_weapon)   // ready weapon
                        {
                            ss_anim->current_animation = 1;
                            ss_anim->next_animation = 1;
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 0;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_HIDE_TO_READY;
                        }
                        break;

                    case WEAPON_STATE_HIDE_TO_READY:
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 0;
                        }
                        else
                        {
                            ss_anim->current_frame = 0;
                            ss_anim->current_animation = 0;
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 0;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE:
                        ss_anim->current_frame = 0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                        ss_anim->frame_time = 0.0;
                        if(entity->character->cmd.ready_weapon)
                        {
                            ss_anim->current_animation = 3;
                            ss_anim->next_animation = 3;
                            ss_anim->current_frame = ss_anim->next_frame = 0;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE_TO_HIDE;
                        }
                        else if(entity->character->cmd.action)
                        {
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE_TO_FIRE;
                        }
                        else
                        {
                            // do nothing here, may be;
                        }
                        break;

                    case WEAPON_STATE_FIRE_TO_IDLE:
                        // Yes, same animation, reverse frames order;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                        if(ss_anim->current_frame > 0)
                        {
                            ss_anim->next_frame = ss_anim->current_frame - 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->next_frame = ss_anim->current_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE_TO_FIRE:
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = ss_anim->current_frame + 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 2;
                        }
                        else if(entity->character->cmd.action)
                        {
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 1;
                            ss_anim->current_animation = 2;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE;
                        }
                        else
                        {
                            ss_anim->frame_time = 0.0;
                            ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE_TO_IDLE;
                        }
                        break;

                    case WEAPON_STATE_FIRE:
                        if(entity->character->cmd.action)
                        {
                            // inc time, loop;
                            ss_anim->frame_time += time;
                            ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                            dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                            ss_anim->lerp = dt / ss_anim->period;
                            t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                            if(ss_anim->current_frame < t - 1)
                            {
                                ss_anim->next_frame = ss_anim->current_frame + 1;
                                ss_anim->next_animation = ss_anim->current_animation;
                            }
                            else if(ss_anim->current_frame < t)
                            {
                                ss_anim->next_frame = 0;
                                ss_anim->next_animation = ss_anim->current_animation;
                            }
                            else
                            {
                                ss_anim->frame_time = dt;
                                ss_anim->current_frame = 0;
                                ss_anim->next_frame = 1;
                            }
                        }
                        else
                        {
                            ss_anim->frame_time = 0.0;
                            ss_anim->current_animation = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                            ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE_TO_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE_TO_HIDE:
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = ss_anim->current_frame + 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->next_frame = ss_anim->current_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_HIDE;
                            Character_SetWeaponModel(entity, entity->character->current_weapon, 0);
                        }
                        break;
                };
            }
            else if((ss_anim->model != NULL) && (ss_anim->model->animation_count == 4))
            {
                switch(entity->character->weapon_current_state)
                {
                    case WEAPON_STATE_HIDE:
                        if(entity->character->cmd.ready_weapon)   // ready weapon
                        {
                            ss_anim->current_animation = 2;
                            ss_anim->next_animation = 2;
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 0;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_HIDE_TO_READY;
                        }
                        break;

                    case WEAPON_STATE_HIDE_TO_READY:
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 0;
                        }
                        else
                        {
                            ss_anim->current_frame = 0;
                            ss_anim->current_animation = 0;
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 0;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE:
                        ss_anim->current_frame = 0;
                        ss_anim->current_animation = 0;
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = 0;
                        ss_anim->frame_time = 0.0;
                        if(entity->character->cmd.ready_weapon)
                        {
                            ss_anim->current_animation = 2;
                            ss_anim->next_animation = 2;
                            ss_anim->current_frame = ss_anim->next_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                            ss_anim->frame_time = 0.0;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE_TO_HIDE;
                        }
                        else if(entity->character->cmd.action)
                        {
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE_TO_FIRE;
                        }
                        else
                        {
                            // do nothing here, may be;
                        }
                        break;

                    case WEAPON_STATE_FIRE_TO_IDLE:
                        // Yes, same animation, reverse frames order;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                        if(ss_anim->current_frame > 0)
                        {
                            ss_anim->next_frame = ss_anim->current_frame - 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->next_frame = ss_anim->current_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE_TO_FIRE:
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                        if(ss_anim->current_frame < t - 1)
                        {
                            ss_anim->next_frame = ss_anim->current_frame + 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else if(ss_anim->current_frame < t)
                        {
                            ss_anim->next_frame = 0;
                            ss_anim->next_animation = 3;
                        }
                        else if(entity->character->cmd.action)
                        {
                            ss_anim->current_frame = 0;
                            ss_anim->next_frame = 1;
                            ss_anim->current_animation = 3;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE;
                        }
                        else
                        {
                            ss_anim->frame_time = 0.0;
                            ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE_TO_IDLE;
                        }
                        break;

                    case WEAPON_STATE_FIRE:
                        if(entity->character->cmd.action)
                        {
                            // inc time, loop;
                            ss_anim->frame_time += time;
                            ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                            dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                            ss_anim->lerp = dt / ss_anim->period;
                            t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                            if(ss_anim->current_frame < t - 1)
                            {
                                ss_anim->next_frame = ss_anim->current_frame + 1;
                                ss_anim->next_animation = ss_anim->current_animation;
                            }
                            else if(ss_anim->current_frame < t)
                            {
                                ss_anim->next_frame = 0;
                                ss_anim->next_animation = ss_anim->current_animation;
                            }
                            else
                            {
                                ss_anim->frame_time = dt;
                                ss_anim->current_frame = 0;
                                ss_anim->next_frame = 1;
                            }
                        }
                        else
                        {
                            ss_anim->frame_time = 0.0;
                            ss_anim->current_animation = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                            ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                            entity->character->weapon_current_state = WEAPON_STATE_FIRE_TO_IDLE;
                        }
                        break;

                    case WEAPON_STATE_IDLE_TO_HIDE:
                        // Yes, same animation, reverse frames order;
                        t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
                        ss_anim->frame_time += time;
                        ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                        dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = dt / ss_anim->period;
                        ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                        if(ss_anim->current_frame > 0)
                        {
                            ss_anim->next_frame = ss_anim->current_frame - 1;
                            ss_anim->next_animation = ss_anim->current_animation;
                        }
                        else
                        {
                            ss_anim->next_frame = ss_anim->current_frame = 0;
                            ss_anim->next_animation = ss_anim->current_animation;
                            entity->character->weapon_current_state = WEAPON_STATE_HIDE;
                            Character_SetWeaponModel(entity, entity->character->current_weapon, 0);
                        }
                        break;
                };
            }

            Entity_DoAnimCommands(entity, ss_anim, 0);
        }
    }
}
