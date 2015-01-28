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
    ret->onFrame = NULL;
    ret->bf.model = NULL;
    ret->bf.frame_time = 0.0;
    ret->bf.next_state = 0;
    ret->bf.lerp = 0.0;
    ret->bf.current_animation = 0;
    ret->bf.current_frame = 0;
    ret->bf.next_animation = 0;
    ret->bf.next_frame = 0;

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

        if(entity->bf.model && entity->bt_body)
        {
            for(int i=0;i<entity->bf.model->mesh_count;i++)
            {
                btRigidBody *body = entity->bt_body[i];
                if(body)
                {
                    body->setUserPointer(NULL);
                    if(body && body->getMotionState())
                    {
                        delete body->getMotionState();
                    }
                    if(body && body->getCollisionShape())
                    {
                        delete body->getCollisionShape();
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
    if(ent->bf.model == NULL)
    {
        return;
    }

    ent->bt_body = (btRigidBody**)malloc(ent->bf.model->mesh_count * sizeof(btRigidBody*));

    for(uint16_t i=0;i<ent->bf.model->mesh_count;i++)
    {
        ent->bt_body[i] = NULL;
        cshape = BT_CSfromMesh(ent->bf.model->mesh_tree[i].mesh, true, true, ent->self->collide_flag);
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
            Entity_ProcessSector(ent);
        
        new_sector = Room_GetSectorXYZ(new_room, pos);
        if(new_room != new_sector->owner_room)
        {
            new_room = new_sector->owner_room;
        }
        ent->self->room = new_room;
        if(ent->current_sector != new_sector)
        {
            ent->current_sector = new_sector;
            if(new_sector && (ent->state_flags & ENTITY_STATE_ENABLED) && (ent->state_flags & ENTITY_STATE_ACTIVE) && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
            {
                Entity_ParseFloorData(ent, &engine_world);
            }
        }
    }
}


void Entity_UpdateRigidBody(entity_p ent, int force)
{
    btScalar tr[16];
    btTransform bt_tr;
    room_p old_room;

    if((ent->bf.model == NULL) || (ent->bt_body == NULL) || ((force == 0) && (ent->bf.model->animation_count == 1) && (ent->bf.model->animations->frames_count == 1)))
    {
        return;
    }

    old_room = ent->self->room;
    Entity_UpdateRoomPos(ent);

#if 1
    if(!ent->character && (ent->self->room != old_room))
    {
        if((ent->self->room != NULL) && !Room_IsOverlapped(ent->self->room, old_room))
        {
            if(ent->self->room && old_room)
            {
                Room_RemoveEntity(old_room, ent);
            }
            if(ent->self->room)
            {
                Room_AddEntity(ent->self->room, ent);
            }
        }
    }
#endif

    if(ent->self->collide_flag != 0x00)
    {
        for(uint16_t i=0;i<ent->bf.model->mesh_count;i++)
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


void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, btScalar etr[16])
{
    btScalar cmd_tr[3], tr[3];
    ss_bone_tag_p btag = bf->bone_tags;
    bone_tag_p src_btag, next_btag;
    btScalar *stack, *sp, t;
    skeletal_model_p model = bf->model;
    bone_frame_p curr_bf, next_bf;

    next_bf = model->animations[bf->next_animation].frames + bf->next_frame;
    curr_bf = model->animations[bf->current_animation].frames + bf->current_frame;

    t = 1.0 - bf->lerp;
    if(etr && (curr_bf->command & ANIM_CMD_MOVE))
    {
        Mat4_vec3_rot_macro(tr, etr, curr_bf->move);
        vec3_mul_scalar(cmd_tr, tr, bf->lerp);
    }
    else
    {
        vec3_set_zero(tr);
        vec3_set_zero(cmd_tr);
    }

    vec3_interpolate_macro(bf->bb_max, curr_bf->bb_max, next_bf->bb_max, bf->lerp, t);
    vec3_add(bf->bb_max, bf->bb_max, cmd_tr);
    vec3_interpolate_macro(bf->bb_min, curr_bf->bb_min, next_bf->bb_min, bf->lerp, t);
    vec3_add(bf->bb_min, bf->bb_min, cmd_tr);
    vec3_interpolate_macro(bf->centre, curr_bf->centre, next_bf->centre, bf->lerp, t);
    vec3_add(bf->centre, bf->centre, cmd_tr);

    vec3_interpolate_macro(bf->pos, curr_bf->pos, next_bf->pos, bf->lerp, t);
    vec3_add(bf->pos, bf->pos, cmd_tr);
    next_btag = next_bf->bone_tags;
    src_btag = curr_bf->bone_tags;
    for(uint16_t k=0;k<curr_bf->bone_tag_count;k++,btag++,src_btag++,next_btag++)
    {
        vec3_interpolate_macro(btag->offset, src_btag->offset, next_btag->offset, bf->lerp, t);
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
            vec4_slerp(btag->qrotate, src_btag->qrotate, tq, bf->lerp);
        }
        else
        {
            vec4_slerp(btag->qrotate, src_btag->qrotate, next_btag->qrotate, bf->lerp);
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

void Entity_DoAnimCommands(entity_p entity, int changing)
{
    if((engine_world.anim_commands_count == 0) ||
       (entity->bf.model->animations[entity->bf.current_animation].num_anim_commands > 255))
    {
        return;  // If no anim commands or current anim has more than 255 (according to TRosettaStone).
    }

    animation_frame_p af  = entity->bf.model->animations + entity->bf.current_animation;
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
                if(entity->bf.current_frame == af->frames_count - 1)
                {
                    if(entity->character)
                    {
                        entity->character->cmd.kill = 1;
                    }
                }

                break;

            case TR_ANIMCOMMAND_PLAYSOUND:
                int16_t sound_index;

                if(entity->bf.current_frame == *++pointer)
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
                if(entity->bf.current_frame == *++pointer)
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
                                switch(entity->current_sector->box_index & 0x0F)
                                {
                                    case 0:                                     // Mud
                                        Audio_Send(288, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 1:                                     // Snow - TR3 & TR5 only
                                        if(engine_world.version != TR_IV)
                                        {
                                            Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        }
                                        break;

                                    case 2:                                     // Sand - same as grass
                                        Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 3:                                     // Gravel
                                        Audio_Send(290, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 4:                                     // Ice - TR3 & TR5 only
                                        if(engine_world.version != TR_IV)
                                        {
                                            Audio_Send(289, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        }
                                        break;

                                    case 5:                                     // Water
                                        // Audio_Send(17, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 6:                                     // Stone - DEFAULT SOUND, BYPASS!
                                        Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 7:                                     // Wood
                                        Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 8:                                     // Metal
                                        Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 9:                                     // Marble - TR4 only
                                        if(engine_world.version == TR_IV)
                                        {
                                            Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        }
                                        break;

                                    case 10:                                    // Grass - same as sand
                                        Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 11:                                    // Concrete - DEFAULT SOUND, BYPASS!
                                        Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 12:                                    // Old wood - same as wood
                                        Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                        break;

                                    case 13:                                    // Old metal - same as metal
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

void Entity_ProcessSector(struct entity_s *ent)
{
    if(ent->character)
    {
        ent->character->height_info.walls_climb_dir = ent->current_sector->flags & (SECTOR_FLAG_CLIMB_WEST  |
                                                                                    SECTOR_FLAG_CLIMB_EAST  |
                                                                                    SECTOR_FLAG_CLIMB_NORTH |
                                                                                    SECTOR_FLAG_CLIMB_SOUTH );
                                                                                    
        ent->character->height_info.walls_climb     = (ent->character->height_info.walls_climb_dir > 0);
        ent->character->height_info.ceiling_climb   = (ent->current_sector->flags & SECTOR_FLAG_CLIMB_CEILING);
        
        if(ent->current_sector->flags & SECTOR_FLAG_DEATH)
        {
            if((ent->move_type == MOVE_ON_FLOOR)    ||
               (ent->move_type == MOVE_UNDER_WATER) ||
               (ent->move_type == MOVE_WADE)        ||
               (ent->move_type == MOVE_ON_WATER)    ||
               (ent->move_type == MOVE_QUICKSAND)    )
            {
                Character_SetParam(ent, PARAM_HEALTH, 0.0);
                ent->character->cmd.kill = 1;
            }
        }
    }
}


int Entity_ParseFloorData(struct entity_s *ent, struct world_s *world)
{
    uint16_t function, sub_function, b3, FD_function, operands = 0x0000;
    //uint16_t slope_t13, slope_t12, slope_t11, slope_t10, slope_func;
    //int16_t slope_t01, slope_t00;
    int ret = 0;
    uint16_t *entry, *end_p, end_bit, cont_bit;
    room_sector_p sector = ent->current_sector;
    char skip = 0;

    // Trigger options.
    uint8_t  trigger_mask;
    uint8_t  only_once;
    int8_t   timer_field;

    if(!sector || (sector->fd_index <= 0) || (sector->fd_index >= world->floor_data_size))
    {
        return 0;
    }

    /*
     * PARSE FUNCTIONS
     */
    end_p = world->floor_data + world->floor_data_size - 1;
    entry = world->floor_data + sector->fd_index;

    do
    {
        end_bit = ((*entry) & 0x8000) >> 15;            // 0b10000000 00000000

        // TR_I - TR_II
        //function = (*entry) & 0x00FF;                   // 0b00000000 11111111
        //sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        //TR_III+, but works with TR_I - TR_II
        function = (*entry) & 0x001F;                   // 0b00000000 00011111
        sub_function = ((*entry) & 0x3FF0) >> 8;        // 0b01111111 11100000
        b3 = ((*entry) & 0x00E0) >> 5;                  // 0b00000000 11100000  TR_III+

        entry++;

        switch(function)
        {
            case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                if(sub_function == 0x00)
                {
                    entry++;
                }
                break;

            case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                if(sub_function == 0x00)
                {
                    entry++;
                }
                break;

            case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                if(sub_function == 0x00)
                {
                    entry++;
                }
                break;

            case TR_FD_FUNC_TRIGGER:          // TRIGGER
                timer_field      =   (*entry) &  0x00FF;
                trigger_mask     =  ((*entry) &  0x3E00) >> 9;
                only_once        =  ((*entry) &  0x0100) >> 8;

                Con_Printf("TRIGGER: timer - %d, mask - %02X", timer_field, trigger_mask);

                skip = 0;
                switch(sub_function)
                {
                    case TR_FD_TRIGTYPE_TRIGGER:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_TRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_PAD:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_PAD");
                        break;
                    case TR_FD_TRIGTYPE_SWITCH:
                        skip = 1;
                        // Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_SWITCH");
                        break;
                    case TR_FD_TRIGTYPE_KEY:
                        skip = 1;
                        //Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_KEY");
                        break;
                    case TR_FD_TRIGTYPE_PICKUP:
                        skip = 1;
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_PICKUP");
                        break;
                    case TR_FD_TRIGTYPE_HEAVY:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVY");
                        break;
                    case TR_FD_TRIGTYPE_ANTIPAD:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_ANTIPAD");
                        break;
                    case TR_FD_TRIGTYPE_COMBAT:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_COMBAT");
                        break;
                    case TR_FD_TRIGTYPE_DUMMY:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_DUMMY");
                        break;
                    case TR_FD_TRIGTYPE_ANTITRIGGER:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_ANTITRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_HEAVYSWITCH:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVYSWITCH");
                        break;
                    case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_HEAVYANTITRIGGER");
                        break;
                    case TR_FD_TRIGTYPE_MONKEY:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_MONKEY");
                        break;
                    case TR_FD_TRIGTYPE_SKELETON:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_SKELETON");
                        break;
                    case TR_FD_TRIGTYPE_TIGHTROPE:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_TIGHTROPE");
                        break;
                    case TR_FD_TRIGTYPE_CRAWLDUCK:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_CRAWLDUCK");
                        break;
                    case TR_FD_TRIGTYPE_CLIMB:
                        Con_Printf("TRIGGER TYPE: TR_FD_TRIGTYPE_CLIMB");
                        break;
                }

                do
                {
                    entry++;
                    cont_bit = ((*entry) & 0x8000) >> 15;                       // 0b10000000 00000000
                    FD_function = (((*entry) & 0x7C00)) >> 10;                  // 0b01111100 00000000
                    operands = (*entry) & 0x03FF;                               // 0b00000011 11111111

                    switch(FD_function)
                    {
                        case TR_FD_TRIGFUNC_OBJECT:                             // ACTIVATE / DEACTIVATE item
                            if(skip == 0)
                            {
                                entity_p e = World_GetEntityByID(&engine_world, operands);
                                if((e != NULL) && ((e->activation_mask ^ trigger_mask) == 0x1F))
                                {
                                    Con_Printf("Activate object %d by %d", operands, ent->id);
                                    e->activation_mask ^= trigger_mask;         // ask Lwmte about it
                                    e->state_flags |= ENTITY_STATE_ACTIVE;
                                }
                            }
                            break;

                        case TR_FD_TRIGFUNC_CAMERATARGET:                       // CAMERA SWITCH
                            {
                                uint8_t cam_index = (*entry) & 0x007F;
                                entry++;
                                uint8_t cam_timer = ((*entry) & 0x00FF);
                                uint8_t cam_once  = ((*entry) & 0x0100) >> 8;
                                uint8_t cam_zoom  = ((*entry) & 0x1000) >> 12;
                                        cont_bit  = ((*entry) & 0x8000) >> 15;                       // 0b10000000 00000000

                                Con_Printf("CAMERA: index = %d, timer = %d, once = %d, zoom = %d", cam_index, cam_timer, cam_once, cam_zoom);
                            }
                            break;

                        case TR_FD_TRIGFUNC_UWCURRENT:          // UNDERWATER CURRENT
                            Con_Printf("UNDERWATER CURRENT! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPMAP:          // SET ALTERNATE ROOM
                            Con_Printf("SET ALTERNATE ROOM! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPON:          // ALTER ROOM FLAGS (paired with 0x05)
                            Con_Printf("ALTER ROOM FLAGS 0x04! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLIPOFF:          // ALTER ROOM FLAGS (paired with 0x04)
                            Con_Printf("ALTER ROOM FLAGS 0x05! OP = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_LOOKAT:          // LOOK AT ITEM
                            Con_Printf("Look at %d item", operands);
                            break;

                        case TR_FD_TRIGFUNC_ENDLEVEL:          // END LEVEL
                            Con_Printf("End of level! id = %d", operands);
                            //If operands 0 we load next level, if not we load the level ID which matches operand!
                            Game_LevelTransition(operands);
                            Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, operands);
                            break;

                        case TR_FD_TRIGFUNC_PLAYTRACK:          // PLAY CD TRACK
                            Con_Printf("Play audiotrack id = %d", operands);
                            // We need to use only_once flag as extra trigger mask,
                            // as it's the way it works in original.
                            Audio_StreamPlay(operands, (trigger_mask << 1) + only_once);
                            break;

                        case TR_FD_TRIGFUNC_FLIPEFFECT:          // Various in-game actions.
                            Con_Printf("Flipeffect id = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_SECRET:          // PLAYSOUND SECRET_FOUND

                            if(!gameflow_manager.SecretsTriggerMap[operands])
                            {
                                Audio_StreamPlay(lua_GetSecretTrackNumber(engine_lua));
                                gameflow_manager.SecretsTriggerMap[operands] ^= 0x01; //Set our flag to on
                                Con_Printf("Play SECRET[%d] FOUND", operands);
                            }
                            else
                            {
                                Con_Printf("SECRET[%d] Has already been found!", operands);
                            }

                            break;

                        case TR_FD_TRIGFUNC_BODYBAG:          // UNKNOWN
                            Con_Printf("BODYBAG id = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_FLYBY:          // TR4-5: FLYBY CAMERA
                            Con_Printf("Flyby camera = %d", operands);
                            break;

                        case TR_FD_TRIGFUNC_CUTSCENE:          // USED IN TR4-5
                            Con_Printf("CUTSCENE id = %d", operands);
                            break;

                        case 0x0e:          // UNKNOWN
                            Con_Printf("TRIGGER: unknown 0x0e, OP = %d", operands);
                            break;

                        case 0x0f:          // UNKNOWN
                            Con_Printf("TRIGGER: unknown 0x0f, OP = %d", operands);
                            break;

                        default:
                            Con_Printf("UNKNOWN MEANING: %X", *entry);
                    };
                }
                while((cont_bit == 0x00) && (entry < end_p));
                break;

            case TR_FD_FUNC_DEATH:          // KILL LARA
                Con_Printf("KILL! sub = %d, b3 = %d", sub_function, b3);
                break;

            case TR_FD_FUNC_CLIMB:          // CLIMBABLE WALLS
                Con_Printf("Climbable walls! sub = %d, b3 = %d", sub_function, b3);
                break;

            case TR_FD_FUNC_FLOORTRIANGLE_NW:                       // TR3 SLANT
            case TR_FD_FUNC_FLOORTRIANGLE_NE:                       // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NW:                     // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NE:                     // TR3 SLANT
            case TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW:             // TR3 SLANT
            case TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE:             // TR3 SLANT
            case TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE:             // TR3 SLANT
            case TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW:             // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW:           // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE:           // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW:           // TR3 SLANT
            case TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE:           // TR3 SLANT
                cont_bit = ((*entry) & 0x8000) >> 15;       // 0b10000000 00000000
                //slope_t01 = ((*entry) & 0x7C00) >> 10;      // 0b01111100 00000000
                //slope_t00 = ((*entry) & 0x03E0) >> 5;       // 0b00000011 11100000
                //slope_func = ((*entry) & 0x001F);           // 0b00000000 00011111
                entry++;
                //slope_t13 = ((*entry) & 0xF000) >> 12;      // 0b11110000 00000000
                //slope_t12 = ((*entry) & 0x0F00) >> 8;       // 0b00001111 00000000
                //slope_t11 = ((*entry) & 0x00F0) >> 4;       // 0b00000000 11110000
                //slope_t10 = ((*entry) & 0x000F);            // 0b00000000 00001111
                break;

            case TR_FD_FUNC_MONKEY:          // Climbable ceiling
                Con_Printf("Climbable ceiling! sub = %d, b3 = %d", sub_function, b3);
                break;
                
            case TR_FD_FUNC_MINECART_LEFT:
                Con_Printf("Trigger Triggerer (TR4) / MINECART LEFT (TR3), OP = %d", operands);
                break;

            case TR_FD_FUNC_MINECART_RIGHT:
                Con_Printf("Clockwork Beetle mark (TR4) / MINECART RIGHT (TR3), OP = %d", operands);
                break;

            default:
                Con_Printf("UNKNOWN function id = %d, sub = %d, b3 = %d", function, sub_function, b3);
                break;
        };
        ret++;
    }
    while(!end_bit && entry < end_p);

    return ret;
}


void Entity_SetAnimation(entity_p entity, int animation, int frame)
{
    animation_frame_p anim;
    long int t;
    btScalar dt;

    if(!entity || !entity->bf.model || (animation >= entity->bf.model->animation_count))
    {
        return;
    }

    animation = (animation < 0)?(0):(animation);

    if(entity->character)
    {
        entity->character->no_fix = 0x00;
    }

    entity->bf.lerp = 0.0;
    anim = &entity->bf.model->animations[animation];
    frame %= anim->frames_count;
    frame = (frame >= 0)?(frame):(anim->frames_count - 1 + frame);
    entity->bf.period = 1.0 / 30.0;

    entity->bf.last_state = anim->state_id;
    entity->bf.next_state = anim->state_id;
    entity->current_speed = anim->speed;
    entity->bf.current_animation = animation;
    entity->bf.current_frame = frame;
    entity->bf.next_animation = animation;
    entity->bf.next_frame = frame;

    entity->bf.frame_time = (btScalar)frame * entity->bf.period;
    t = (entity->bf.frame_time) / entity->bf.period;
    dt = entity->bf.frame_time - (btScalar)t * entity->bf.period;
    entity->bf.frame_time = (btScalar)frame * entity->bf.period + dt;

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
            for(uint16_t j=0;j<ret->anim_dispath_count;j++)
            {
                if(ret->anim_dispath[j].next_anim == state_change_anim)
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
    if(id >= 0)
    {
        state_change_p ret = anim->state_change;
        for(uint16_t i=0;i<anim->state_change_count;i++,ret++)
        {
            if(ret->id == id)
            {
                return ret;
            }
        }
    }

    return NULL;
}


int Entity_GetAnimDispatchCase(struct entity_s *entity, uint32_t id)
{
    if(id >= 0)
    {
        animation_frame_p anim = entity->bf.model->animations + entity->bf.current_animation;
        state_change_p stc = anim->state_change;
        anim_dispath_p disp;
        for(uint16_t i=0;i<anim->state_change_count;i++,stc++)
        {
            if(stc->id == id)
            {
                disp = stc->anim_dispath;
                for(uint16_t j=0;j<stc->anim_dispath_count;j++,disp++)
                {
                    if((disp->frame_high >= disp->frame_low) && (entity->bf.current_frame >= disp->frame_low) && (entity->bf.current_frame <= disp->frame_high))// ||
                       //(disp->frame_high <  disp->frame_low) && ((entity->bf.current_frame >= disp->frame_low) || (entity->bf.current_frame <= disp->frame_high)))
                    {
                        return (int)j;
                    }
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
    animation_frame_p curr_anim = bf->model->animations + bf->current_animation;

    *frame = (bf->frame_time + time) / bf->period;
    *frame = (*frame >= 0.0)?(*frame):(0.0);                                    // paranoid checking
    *anim  = bf->current_animation;

    /*
     * Flag has a highest priority
     */
    if(anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(*frame >= curr_anim->frames_count - 1)
        {
            *frame = curr_anim->frames_count - 1;
            *anim  = bf->current_animation;                                     // paranoid dublicate
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
        *anim   = bf->current_animation;                                      // paranoid dublicate
        return;
    }

    /*
     * State change check
     */
    if(stc != NULL)
    {
        anim_dispath_p disp = stc->anim_dispath;
        for(uint16_t i=0;i<stc->anim_dispath_count;i++,disp++)
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
    if(entity->bf.model != NULL)
    {
        bone_frame_p curr_bf = entity->bf.model->animations[entity->bf.current_animation].frames + entity->bf.current_frame;

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
int Entity_Frame(entity_p entity, btScalar time)
{
    int16_t frame, anim, ret = 0x00;
    long int t;
    btScalar dt;
    animation_frame_p af;
    state_change_p stc;

    if((entity == NULL) || !(entity->state_flags & ENTITY_STATE_ACTIVE)  || !(entity->state_flags & ENTITY_STATE_ENABLED) || (entity->bf.model == NULL) || ((entity->bf.model->animation_count == 1) && (entity->bf.model->animations->frames_count == 1)))
    {
        return 0;
    }

    entity->bf.lerp = 0.0;
    stc = Anim_FindStateChangeByID(entity->bf.model->animations + entity->bf.current_animation, entity->bf.next_state);
    Entity_GetNextFrame(&entity->bf, time, stc, &frame, &anim, entity->anim_flags);
    if(anim != entity->bf.current_animation)
    {
        entity->bf.last_animation = entity->bf.current_animation;

        ret = 0x02;
        Entity_DoAnimCommands(entity, ret);
        Entity_DoAnimMove(entity);
        Entity_SetAnimation(entity, anim, frame);
        stc = Anim_FindStateChangeByID(entity->bf.model->animations + entity->bf.current_animation, entity->bf.next_state);
    }
    else if(entity->bf.current_frame != frame)
    {
        if(entity->bf.current_frame == 0)
        {
            entity->bf.last_animation = entity->bf.current_animation;
        }

        ret = 0x01;
        Entity_DoAnimCommands(entity, ret);
        Entity_DoAnimMove(entity);
        entity->bf.current_frame = frame;
    }

    af = entity->bf.model->animations + entity->bf.current_animation;
    entity->bf.frame_time += time;

    t = (entity->bf.frame_time) / entity->bf.period;
    dt = entity->bf.frame_time - (btScalar)t * entity->bf.period;
    entity->bf.frame_time = (btScalar)frame * entity->bf.period + dt;
    entity->bf.lerp = (entity->smooth_anim)?(dt / entity->bf.period):(0.0);
    Entity_GetNextFrame(&entity->bf, entity->bf.period, stc, &entity->bf.next_frame, &entity->bf.next_animation, entity->anim_flags);

    /*
     * Update acceleration
     */
    if(entity->character)
    {
        entity->current_speed += time * entity->character->speed_mult * (btScalar)af->accel_hi;
    }

    Entity_UpdateCurrentBoneFrame(&entity->bf, entity->transform);
    if(entity->onFrame != NULL)
    {
        entity->onFrame(entity, ret);
    }

    return ret;
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity_RebuildBV(entity_p ent)
{
    if((ent != NULL) && (ent->bf.model != NULL))
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
                if((e->type_flags & ENTITY_TYPE_TRIGGER) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
                    if((e != ent) && (OBB_OBB_Test(e, ent) == 1))//(vec3_dist_sq(ent->transform+12, pos) < r))
                    {
                        lua_ActivateEntity(engine_lua, e->id, ent->id, ENTITY_CALLBACK_ACTIVATE);
                    }
                }
                else if((e->type_flags & ENTITY_TYPE_PICKABLE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    btScalar *v = e->transform + 12;
                    if((e != ent) && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r) &&
                                      (v[2] + 32.0 > ent->transform[12+2] + ent->bf.bb_min[2]) && (v[2] - 32.0 < ent->transform[12+2] + ent->bf.bb_max[2]))
                    {
                        lua_ActivateEntity(engine_lua, e->id, ent->id, ENTITY_CALLBACK_ACTIVATE);
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
