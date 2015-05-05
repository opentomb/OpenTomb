#include <stdlib.h>
#include <math.h>

#include "vmath.h"
#include "mesh.h"
#include "entity.h"
#include "render.h"
#include "camera.h"
#include "world.h"
#include "engine.h"
#include "console.h"
#include "script.h"
#include "gui.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "obb.h"
#include "gameflow.h"
#include "string.h"

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"


entity_p Entity_Create()
{
    entity_p ret = (entity_p)calloc(1, sizeof(entity_t));

    ret->move_type = MOVE_ON_FLOOR;
    Mat4_E(ret->transform);
    ret->state_flags = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    ret->type_flags = ENTITY_TYPE_DECORATION;
    ret->callback_flags = 0x00000000;               // no callbacks by default

    ret->OCB = 0;
    ret->sector_status = 0;
    ret->locked = 0;

    ret->self = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->self->next = NULL;
    ret->self->object = ret;
    ret->self->object_type = OBJECT_ENTITY;
    ret->self->room = NULL;
    ret->self->collide_flag = 0;
    ret->obb = OBB_Create();
    ret->obb->transform = ret->transform;
    ret->bt_body = NULL;
    ret->character = NULL;
    ret->smooth_anim = 1;
    ret->current_sector = NULL;
    ret->bf.animations.model = NULL;
    ret->bf.animations.onFrame = NULL;
    ret->bf.animations.frame_time = 0.0;
    ret->bf.animations.next_state = 0;
    ret->bf.animations.lerp = 0.0;
    ret->bf.animations.current_animation = 0;
    ret->bf.animations.current_frame = 0;
    ret->bf.animations.next_animation = 0;
    ret->bf.animations.next_frame = 0;
    ret->bf.animations.next = NULL;
    ret->bf.bone_tag_count = 0;
    ret->bf.bone_tags = 0;
    vec3_set_zero(ret->bf.bb_max);
    vec3_set_zero(ret->bf.bb_min);
    vec3_set_zero(ret->bf.centre);
    vec3_set_zero(ret->bf.pos);
    vec4_set_zero(ret->speed.m_floats);

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

        if(entity->bf.animations.model && entity->bt_body)
        {
            for(int i=0;i<entity->bf.bone_tag_count;i++)
            {
                btRigidBody *body = entity->bt_body[i];
                if(body)
                {
                    body->setUserPointer(NULL);
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(NULL);
                    }
                    if(body->getCollisionShape())
                    {
                        delete body->getCollisionShape();
                        body->setCollisionShape(NULL);
                    }

                    if(body->isInWorld())
                    {
                        bt_engine_dynamicsWorld->removeRigidBody(body);
                    }
                    delete body;
                    entity->bt_body[i] = NULL;
                }
            }
        }

        if(entity->character)
        {
            Character_Clean(entity);
        }

        if(entity->self)
        {
            free(entity->self);
            entity->self = NULL;
        }

        if(entity->bf.bone_tag_count)
        {
            free(entity->bf.bone_tags);
            entity->bf.bone_tags = NULL;
            entity->bf.bone_tag_count = 0;
        }

        for(ss_animation_p ss_anim=entity->bf.animations.next;ss_anim!=NULL;)
        {
            ss_animation_p ss_anim_next = ss_anim->next;
            ss_anim->next = NULL;
            free(ss_anim);
            ss_anim = ss_anim_next;
        }
        entity->bf.animations.next = NULL;
    }
}


void Entity_Enable(entity_p ent)
{
    if(!(ent->state_flags & ENTITY_STATE_ENABLED))
    {
        if(ent->bt_body != NULL)
        {
            for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
            {
                btRigidBody *b = ent->bt_body[i];
                if((b != NULL) && !b->isInWorld())
                {
                    bt_engine_dynamicsWorld->addRigidBody(b);
                }
            }
        }
        ent->state_flags |= ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    }
}


void Entity_Disable(entity_p ent)
{
    if(ent->state_flags & ENTITY_STATE_ENABLED)
    {
        if(ent->bt_body != NULL)
        {
            for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
            {
                btRigidBody *b = ent->bt_body[i];
                if((b != NULL) && b->isInWorld())
                {
                    bt_engine_dynamicsWorld->removeRigidBody(b);
                }
            }
        }
        ent->state_flags = 0x0000;
    }
}

/**
 * This function enables collision for entity_p in all cases exept NULL models.
 * If collision models does not exists, function will create them;
 * @param ent - pointer to the entity.
 */
void Entity_EnableCollision(entity_p ent)
{
    if(ent->bt_body != NULL)
    {
        ent->self->collide_flag = 0x01;
        for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
        {
            btRigidBody *b = ent->bt_body[i];
            if((b != NULL) && !b->isInWorld())
            {
                bt_engine_dynamicsWorld->addRigidBody(b);
            }
        }
    }
    else
    {
        ent->self->collide_flag = 0x01;
        BT_GenEntityRigidBody(ent);
    }
}


void Entity_DisableCollision(entity_p ent)
{
    if(ent->bt_body != NULL)
    {
        ent->self->collide_flag = 0x00;
        for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
        {
            btRigidBody *b = ent->bt_body[i];
            if((b != NULL) && b->isInWorld())
            {
                bt_engine_dynamicsWorld->removeRigidBody(b);
            }
        }
    }
}


void BT_GenEntityRigidBody(entity_p ent)
{
    btScalar tr[16];
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;
    if(ent->bf.animations.model == NULL)
    {
        return;
    }

    ent->bt_body = (btRigidBody**)malloc(ent->bf.bone_tag_count * sizeof(btRigidBody*));

    for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
    {
        ent->bt_body[i] = NULL;
        cshape = BT_CSfromMesh(ent->bf.animations.model->mesh_tree[i].mesh_base, true, true, ent->self->collide_flag);
        if(cshape)
        {
            Mat4_Mat4_mul_macro(tr, ent->transform, ent->bf.bone_tags[i].full_transform);
            startTransform.setFromOpenGLMatrix(tr);
            btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
            ent->bt_body[i] = new btRigidBody(0.0, motionState, cshape, localInertia);
            bt_engine_dynamicsWorld->addRigidBody(ent->bt_body[i], COLLISION_GROUP_CINEMATIC, COLLISION_MASK_ALL);
            ent->bt_body[i]->setUserPointer(ent->self);
        }
    }
}


void Entity_UpdateRoomPos(entity_p ent)
{
    btScalar pos[3], v[3];
    room_p new_room;
    room_sector_p new_sector;

    vec3_add(v, ent->bf.bb_min, ent->bf.bb_max);
    v[0] /= 2.0;
    v[1] /= 2.0;
    v[2] /= 2.0;
    Mat4_vec3_mul_macro(pos, ent->transform, v);
    new_room = Room_FindPosCogerrence(&engine_world, pos, ent->self->room);
    if(new_room)
    {
        if(ent->current_sector)
        {
            Entity_ProcessSector(ent);
        }

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
            ent->sector_status = 0; // Reset sector status.
            ent->current_sector = new_sector;
        }
    }
}


void Entity_UpdateRigidBody(entity_p ent, int force)
{
    btScalar tr[16];
    btTransform bt_tr;

    if((ent->bf.animations.model == NULL) || (ent->bt_body == NULL) || ((force == 0) && (ent->bf.animations.model->animation_count == 1) && (ent->bf.animations.model->animations->frames_count == 1)))
    {
        return;
    }

    Entity_UpdateRoomPos(ent);

    if(ent->self->collide_flag != 0x00)
    {
        for(uint16_t i=0;i<ent->bf.bone_tag_count;i++)
        {
            if(ent->bt_body[i])
            {
                Mat4_Mat4_mul_macro(tr, ent->transform, ent->bf.bone_tags[i].full_transform);
                bt_tr.setFromOpenGLMatrix(tr);
                ent->bt_body[i]->setCollisionFlags(ent->bt_body[i]->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                ent->bt_body[i]->setWorldTransform(bt_tr);
            }
        }
    }

    Entity_RebuildBV(ent);
}


void Entity_UpdateRotation(entity_p entity)
{
    btScalar R[4], Rt[4], temp[4];
    btScalar sin_t2, cos_t2, t;
    btScalar *up_dir = entity->transform + 8;                                   // OZ
    btScalar *view_dir = entity->transform + 4;                                 // OY
    btScalar *right_dir = entity->transform + 0;                                // OX
    int i;

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
}


void Entity_UpdateCurrentSpeed(entity_p entity, int zeroVz)
{
    btScalar t  = entity->current_speed * entity->character->speed_mult;
    btScalar vz = (zeroVz)?(0.0):(entity->speed.m_floats[2]);

    if(entity->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(entity->speed.m_floats, entity->transform+4, t);
    }
    else if(entity->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(entity->speed.m_floats, entity->transform+4,-t);
    }
    else if(entity->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(entity->speed.m_floats, entity->transform+0,-t);
    }
    else if(entity->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(entity->speed.m_floats, entity->transform+0, t);
    }
    else
    {
        vec3_set_zero(entity->speed.m_floats);
    }

    entity->speed.m_floats[2] = vz;
}


void Entity_AddOverrideAnim(struct entity_s *ent, int model_id)
{
    skeletal_model_p sm = World_GetModelByID(&engine_world, model_id);

    if((sm != NULL) && (sm->mesh_count == ent->bf.bone_tag_count))
    {
        ss_animation_p ss_anim = (ss_animation_p)malloc(sizeof(ss_animation_t));

        ss_anim->model = sm;
        ss_anim->onFrame = NULL;
        ss_anim->next = ent->bf.animations.next;
        ent->bf.animations.next = ss_anim;

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


void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, btScalar etr[16])
{
    btScalar cmd_tr[3], tr[3];
    ss_bone_tag_p btag = bf->bone_tags;
    bone_tag_p src_btag, next_btag;
    btScalar *stack, *sp, t;
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
            btScalar tq[4];
            if(next_bf->command & ANIM_CMD_CHANGE_DIRECTION)
            {
                ///@TODO: add OX rotation inverse for underwater case
                tq[0] =-next_btag->qrotate[1];  // -  +
                tq[1] = next_btag->qrotate[0];  // +  -
                tq[2] = next_btag->qrotate[3];  // +  +
                tq[3] =-next_btag->qrotate[2];  // -  -

                btag->transform[12 + 0] -= bf->pos[0];
                btag->transform[12 + 1] -= bf->pos[1];
                btag->transform[12 + 2] += bf->pos[2];
            }
            else
            {
                vec4_copy(tq, next_btag->qrotate);
                vec3_add(btag->transform+12, btag->transform+12, bf->pos);
            }
            vec4_slerp(btag->qrotate, src_btag->qrotate, tq, bf->animations.lerp);
        }
        else
        {
            bone_tag_p ov_src_btag = src_btag;
            bone_tag_p ov_next_btag = next_btag;
            btScalar ov_lerp = bf->animations.lerp;
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
    sp = stack = GetTempbtScalar(model->mesh_count * 16);
    int16_t stack_use = 0;

    btag = bf->bone_tags;

    Mat4_Copy(btag->full_transform, btag->transform);
    Mat4_Copy(sp, btag->transform);
    btag++;

    for(uint16_t k=1;k<curr_bf->bone_tag_count;k++,btag++)
    {
        if(btag->flag & 0x01)
        {
            if(stack_use > 0)
            {
                sp -= 16;// glPopMatrix();
                stack_use--;
            }
        }
        if(btag->flag & 0x02)
        {
            if(stack_use + 1 < (int16_t)model->mesh_count)
            {
                Mat4_Copy(sp+16, sp);
                sp += 16;// glPushMatrix();
                stack_use++;
            }
        }
        Mat4_Mat4_mul(sp, sp, btag->transform); // glMultMatrixd(btag->transform);
        Mat4_Copy(btag->full_transform, sp);
    }

    ReturnTempbtScalar(model->mesh_count * 16);
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

btScalar Entity_FindDistance(entity_p entity_1, entity_p entity_2)
{
    btScalar *v1 = entity_1->transform + 12;
    btScalar *v2 = entity_2->transform + 12;

    return vec3_dist(v1, v2);
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
                                    btScalar dist = Entity_FindDistance(engine_world.Character, entity);
                                    dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):((TR_CAM_MAX_SHAKE_DISTANCE - dist) / 1024.0);
                                    if(dist > 0)
                                        Cam_Shake(renderer.cam, (dist * TR_CAM_DEFAULT_SHAKE_POWER), 0.5);
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
                                            Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
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


room_sector_s* Entity_GetLowestSector(room_sector_s* sector)
{
    room_sector_p lowest_sector = sector;

    for(room_sector_p rs=sector;rs!=NULL;rs=rs->sector_below)
    { lowest_sector = rs; }

    return lowest_sector;
}


room_sector_s* Entity_GetHighestSector(room_sector_s* sector)
{
    room_sector_p highest_sector = sector;

    for(room_sector_p rs=sector;rs!=NULL;rs=rs->sector_above)
    { highest_sector = rs; }

    return highest_sector;
}


void Entity_ProcessSector(struct entity_s *ent)
{
    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    room_sector_p highest_sector = Entity_GetHighestSector(ent->current_sector);
    room_sector_p lowest_sector  = Entity_GetLowestSector(ent->current_sector);

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
               (ent->move_type == MOVE_UNDER_WATER) ||
               (ent->move_type == MOVE_WADE)        ||
               (ent->move_type == MOVE_ON_WATER)    ||
               (ent->move_type == MOVE_QUICKSAND))
            {
                Character_SetParam(ent, PARAM_HEALTH, 0.0);
                ent->character->resp.kill = 1;
            }
        }
    }

    // Look up trigger function table and run trigger if it exists.

    int top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, "tlist_RunTrigger");
    if(lua_isfunction(engine_lua, -1))
    {
        lua_pushnumber(engine_lua, lowest_sector->trig_index);
        lua_pushnumber(engine_lua, ((ent->bf.animations.model->id == 0) ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC));
        lua_pushnumber(engine_lua, ent->id);
        lua_pcall(engine_lua, 3, 1, 0);
    }
    lua_settop(engine_lua, top);
}


void Entity_SetAnimation(entity_p entity, int animation, int frame)
{
    if(!entity || !entity->bf.animations.model || (animation >= entity->bf.animations.model->animation_count))
    {
        return;
    }

    animation = (animation < 0)?(0):(animation);

    if(entity->character)
    {
        entity->character->no_fix = 0x00;
    }

    entity->bf.animations.lerp = 0.0;
    animation_frame_p anim = &entity->bf.animations.model->animations[animation];
    frame %= anim->frames_count;
    frame = (frame >= 0)?(frame):(anim->frames_count - 1 + frame);
    entity->bf.animations.period = 1.0 / 30.0;

    entity->bf.animations.last_state = anim->state_id;
    entity->bf.animations.next_state = anim->state_id;
    entity->current_speed = anim->speed;
    entity->bf.animations.current_animation = animation;
    entity->bf.animations.current_frame = frame;
    entity->bf.animations.next_animation = animation;
    entity->bf.animations.next_frame = frame;

    entity->bf.animations.frame_time = (btScalar)frame * entity->bf.animations.period;
    long int t = (entity->bf.animations.frame_time) / entity->bf.animations.period;
    btScalar dt = entity->bf.animations.frame_time - (btScalar)t * entity->bf.animations.period;
    entity->bf.animations.frame_time = (btScalar)frame * entity->bf.animations.period + dt;

    Entity_UpdateCurrentBoneFrame(&entity->bf, entity->transform);
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
    animation_frame_p anim = entity->bf.animations.model->animations + entity->bf.animations.current_animation;
    state_change_p stc = anim->state_change;

    for(uint16_t i=0;i<anim->state_change_count;i++,stc++)
    {
        if(stc->id == id)
        {
            anim_dispatch_p disp = stc->anim_dispatch;
            for(uint16_t j=0;j<stc->anim_dispatch_count;j++,disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (entity->bf.animations.current_frame >= disp->frame_low) && (entity->bf.animations.current_frame <= disp->frame_high))// ||
                   //(disp->frame_high <  disp->frame_low) && ((entity->bf.current_frame >= disp->frame_low) || (entity->bf.current_frame <= disp->frame_high)))
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
void Entity_GetNextFrame(struct ss_bone_frame_s *bf, btScalar time, struct state_change_s *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags)
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


void Entity_DoAnimMove(entity_p entity)
{
    if(entity->bf.animations.model != NULL)
    {
        bone_frame_p curr_bf = entity->bf.animations.model->animations[entity->bf.animations.current_animation].frames + entity->bf.animations.current_frame;

        if(curr_bf->command & ANIM_CMD_JUMP)
        {
            Character_SetToJump(entity, -curr_bf->v_Vertical, curr_bf->v_Horizontal);
        }
        if(curr_bf->command & ANIM_CMD_CHANGE_DIRECTION)
        {
            entity->angles[0] += 180.0;
            if(entity->move_type == MOVE_UNDER_WATER)
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
            Entity_UpdateRotation(entity);
        }
        if(curr_bf->command & ANIM_CMD_MOVE)
        {
            btScalar tr[3];
            Mat4_vec3_rot_macro(tr, entity->transform, curr_bf->move);
            vec3_add(entity->transform+12, entity->transform+12, tr);
        }
    }
}

/**
 * In original engine (+ some information from anim_commands) the anim_commands implement in beginning of frame
 */
///@TODO: rewrite as a cycle through all bf.animations list
int Entity_Frame(entity_p entity, btScalar time)
{
    int16_t frame, anim, ret = 0x00;
    long int t;
    btScalar dt;
    animation_frame_p af;
    state_change_p stc;
    ss_animation_p ss_anim;

    if((entity == NULL) || !(entity->state_flags & ENTITY_STATE_ACTIVE)  || !(entity->state_flags & ENTITY_STATE_ENABLED) ||
       (entity->bf.animations.model == NULL) || ((entity->bf.animations.model->animation_count == 1) && (entity->bf.animations.model->animations->frames_count == 1)))
    {
        return 0;
    }

    ss_anim = &entity->bf.animations;

    entity->bf.animations.lerp = 0.0;
    stc = Anim_FindStateChangeByID(ss_anim->model->animations + ss_anim->current_animation, ss_anim->next_state);
    Entity_GetNextFrame(&entity->bf, time, stc, &frame, &anim, ss_anim->anim_flags);
    if(anim != ss_anim->current_animation)
    {
        ss_anim->last_animation = ss_anim->current_animation;

        ret = 0x02;
        Entity_DoAnimCommands(entity, &entity->bf.animations, ret);
        Entity_DoAnimMove(entity);
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
        Entity_DoAnimCommands(entity, &entity->bf.animations, ret);
        Entity_DoAnimMove(entity);
        entity->bf.animations.current_frame = frame;
    }

    af = entity->bf.animations.model->animations + entity->bf.animations.current_animation;
    entity->bf.animations.frame_time += time;

    t = (entity->bf.animations.frame_time) / entity->bf.animations.period;
    dt = entity->bf.animations.frame_time - (btScalar)t * entity->bf.animations.period;
    entity->bf.animations.frame_time = (btScalar)frame * entity->bf.animations.period + dt;
    entity->bf.animations.lerp = (entity->smooth_anim)?(dt / entity->bf.animations.period):(0.0);
    Entity_GetNextFrame(&entity->bf, entity->bf.animations.period, stc, &entity->bf.animations.next_frame, &entity->bf.animations.next_animation, ss_anim->anim_flags);

    /* There are stick code for multianimation (weapon mode) testing
     * Model replacing will be upgraded too, I have to add override
     * flags to model manually in the script*/
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

        for(ss_animation_p ss_anim=entity->bf.animations.next;ss_anim!=NULL;ss_anim=ss_anim->next)
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                            dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                            ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                            dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                            ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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
                        dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                        ss_anim->lerp = (entity->smooth_anim)?(dt / ss_anim->period):(0.0);
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

    /*
     * Update acceleration
     */
    if(entity->character)
    {
        entity->current_speed += time * entity->character->speed_mult * (btScalar)af->accel_hi;
    }

    Entity_UpdateCurrentBoneFrame(&entity->bf, entity->transform);
    if(entity->bf.animations.onFrame != NULL)
    {
        entity->bf.animations.onFrame(entity, &entity->bf.animations, ret);
    }

    return ret;
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity_RebuildBV(entity_p ent)
{
    if((ent != NULL) && (ent->bf.animations.model != NULL))
    {
        /*
         * get current BB from animation
         */
        OBB_Rebuild(ent->obb, ent->bf.bb_min, ent->bf.bb_max);
        OBB_Transform(ent->obb);
    }
}


void Entity_CheckActivators(struct entity_s *ent)
{
    if((ent != NULL) && (ent->self->room != NULL))
    {
        btScalar ppos[3];

        ppos[0] = ent->transform[12+0] + ent->transform[4+0] * ent->bf.bb_max[1];
        ppos[1] = ent->transform[12+1] + ent->transform[4+1] * ent->bf.bb_max[1];
        ppos[2] = ent->transform[12+2] + ent->transform[4+2] * ent->bf.bb_max[1];
        engine_container_p cont = ent->self->room->containers;
        for(;cont;cont=cont->next)
        {
            if((cont->object_type == OBJECT_ENTITY) && (cont->object))
            {
                entity_p e = (entity_p)cont->object;
                btScalar r = e->activation_offset[3];
                r *= r;
                if((e->type_flags & ENTITY_TYPE_INTERACTIVE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
                    if((e != ent) && (OBB_OBB_Test(e, ent) == 1))//(vec3_dist_sq(ent->transform+12, pos) < r))
                    {
                        lua_ExecEntity(engine_lua, e->id, ent->id, ENTITY_CALLBACK_ACTIVATE);
                    }
                }
                else if((e->type_flags & ENTITY_TYPE_PICKABLE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    btScalar *v = e->transform + 12;
                    if((e != ent) && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r) &&
                                      (v[2] + 32.0 > ent->transform[12+2] + ent->bf.bb_min[2]) && (v[2] - 32.0 < ent->transform[12+2] + ent->bf.bb_max[2]))
                    {
                        lua_ExecEntity(engine_lua, e->id, ent->id, ENTITY_CALLBACK_ACTIVATE);
                    }
                }
            }
        }
    }
}


void Entity_MoveForward(struct entity_s *ent, btScalar dist)
{
    ent->transform[12] += ent->transform[4] * dist;
    ent->transform[13] += ent->transform[5] * dist;
    ent->transform[14] += ent->transform[6] * dist;
}


void Entity_MoveStrafe(struct entity_s *ent, btScalar dist)
{
    ent->transform[12] += ent->transform[0] * dist;
    ent->transform[13] += ent->transform[1] * dist;
    ent->transform[14] += ent->transform[2] * dist;
}


void Entity_MoveVertical(struct entity_s *ent, btScalar dist)
{
    ent->transform[12] += ent->transform[8] * dist;
    ent->transform[13] += ent->transform[9] * dist;
    ent->transform[14] += ent->transform[10] * dist;
}
