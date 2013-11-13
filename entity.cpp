#include <stdlib.h>
#include <math.h>

#include "vmath.h"
#include "mesh.h"
#include "entity.h"
#include "render.h"
#include "world.h"
#include "engine.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "bounding_volume.h"

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"


extern uint16_t                sounds_played;

entity_p Entity_Create()
{
    entity_p ret = (entity_p)calloc(1, sizeof(entity_t));
    ret->frame_time = 0.0;
    ret->move_type = MOVE_ON_FLOOR;
    Mat4_E(ret->transform);
    
    ret->self = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->self->next = NULL;
    ret->self->object = ret;
    ret->self->object_type = OBJECT_ENTITY;
    ret->self->room = NULL;
    ret->self->collide_flag = 0;
    ret->bv = BV_Create();
    ret->bv->transform = ret->transform;
    ret->bt_body = NULL;
    ret->character = NULL;
    ret->smooth_anim = 1;
    
    ret->lerp = 0.0;
    ret->next_bf = NULL;
    
    ret->bf.bone_tag_count = 0;;
    ret->bf.bone_tags = 0;
    vec3_set_zero(ret->bf.bb_max);     
    vec3_set_zero(ret->bf.bb_min);  
    vec3_set_zero(ret->bf.centre);  
    vec3_set_zero(ret->bf.pos);  
    vec4_set_zero(ret->collision_offset.m_floats);                              // not an error, really btVector3 has 4 points in array
    
    return ret;
}


void Entity_Clear(entity_p entity)
{
    if(entity)
    {
        if(entity->bv)
        {
            BV_Clear(entity->bv);
            free(entity->bv);
            entity->bv = NULL;
        }
        
        if(entity->model && entity->bt_body)
        {
            for(int i=0;i<entity->model->mesh_count;i++)
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

                    bt_engine_dynamicsWorld->removeRigidBody(body);
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


void Entity_UpdateRigidBody(entity_p ent)
{
    int i;
    btScalar tr[16], pos[3], c[3];
    btTransform	bt_tr;
    room_p new_room;
    
    if(!ent->model || !ent->bt_body || ((ent->model->animations->frames_count == 1) && (ent->model->animation_count == 1)))
    {
        return;
    }

    if(!ent->character)
    {
        vec3_add(c, ent->bf.bb_min, ent->bf.bb_max);
        c[0] /= 2.0;
        c[1] /= 2.0;
        c[2] /= 2.0;
        
        Mat4_vec3_mul_macro(pos, ent->transform, c);
        new_room = Room_FindPosCogerrence(&engine_world, pos, ent->self->room);
        if(ent->self->room != new_room)
        {
            if(new_room != NULL && !Room_IsOverlapped(ent->self->room, new_room))
            {
                if(ent->self->room && new_room)
                {
                    Room_RemoveEntity(ent->self->room, ent);
                }
                if(new_room)
                {
                    Room_AddEntity(new_room, ent);
                }
                ent->self->room = new_room;
            }
        }
    }
    
    for(i=0;i<ent->model->mesh_count;i++)
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


void Entity_UpdateCurrentBoneFrame(entity_p entity)
{
    long int k, stack_use;
    ss_bone_tag_p btag = entity->bf.bone_tags;
    bone_tag_p src_btag, next_btag;
    btScalar *stack, *sp, t;
    skeletal_model_p model = entity->model;
    bone_frame_p bf = model->animations[entity->current_animation].frames + entity->current_frame;    
        
    if(entity->next_bf == NULL)
    {
        entity->lerp = 0.0;
        entity->next_bf = bf;
    }

    t = 1.0 - entity->lerp;
    vec3_copy(entity->bf.bb_max, bf->bb_max);
    vec3_copy(entity->bf.bb_min, bf->bb_min);
    vec3_copy(entity->bf.centre, bf->centre);
    entity->bf.pos[0] = t * bf->pos[0] + entity->lerp * entity->next_bf->pos[0];
    entity->bf.pos[1] = t * bf->pos[1] + entity->lerp * entity->next_bf->pos[1];
    entity->bf.pos[2] = t * bf->pos[2] + entity->lerp * entity->next_bf->pos[2];
    
    next_btag = entity->next_bf->bone_tags;
    src_btag = bf->bone_tags;
    for(k=0;k<bf->bone_tag_count;k++,btag++,src_btag++,next_btag++)
    {
        btag->offset[0] = t * src_btag->offset[0] + entity->lerp * next_btag->offset[0];
        btag->offset[1] = t * src_btag->offset[1] + entity->lerp * next_btag->offset[1];
        btag->offset[2] = t * src_btag->offset[2] + entity->lerp * next_btag->offset[2];
        vec3_copy(btag->transform+12, btag->offset);
        btag->transform[15] = 1.0;
        if(k == 0)
        {
            vec3_add(btag->transform+12, btag->transform+12, entity->bf.pos);
        }
        
        vec4_slerp(btag->qrotate, src_btag->qrotate, next_btag->qrotate, entity->lerp);
        Mat4_set_qrotation(btag->transform, btag->qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    sp = stack = GetTempbtScalar(model->mesh_count * 16);
    stack_use = 0;

    btag = entity->bf.bone_tags;

    Mat4_Copy(btag->full_transform, btag->transform);
    Mat4_Copy(sp, btag->transform);
    btag++;
    
    for(k=1;k<bf->bone_tag_count;k++,btag++)
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
            if(stack_use < model->mesh_count - 1)
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


void Entity_DoAnimCommands(entity_p entity, int changing)
{
    if((engine_world.anim_commands_count == 0) || 
       (entity->model->animations[entity->current_animation].num_anim_commands > 255))
    {
        return;  // If no anim commands or current anim has more than 255.
    }
        
    animation_frame_p af = entity->model->animations + entity->current_animation;
    uint32_t count       = af->num_anim_commands;
    int16_t *pointer     = engine_world.anim_commands + af->anim_command;
    
    for(int i = 0; i < count; i++, pointer++)
    {
        switch(*pointer)
        {
            case TR_ANIMCOMMAND_SETPOSITION:
                // This command executes ONLY at the end of animation. 
                if(entity->current_frame == entity->model->animations[entity->current_animation].frames_count - 1)
                {
                    entity->transform[12] += (btScalar)(*++pointer);
                    entity->transform[14] -= (btScalar)(*++pointer); // Y is inverted in OpenGL coordinates.
                    entity->transform[13] += (btScalar)(*++pointer);
                }
                else
                {
                    pointer += 3; // Parse through 3 operands.
                }
                break;
                
            case TR_ANIMCOMMAND_JUMPDISTANCE:
                // This command executes ONLY at the end of animation. 
                if(entity->current_frame == entity->model->animations[entity->current_animation].frames_count - 1)
                {
                    int16_t v_Vertical   = *++pointer;
                    int16_t v_Horizontal = *++pointer;
                    
                    Character_SetToJump(entity, -v_Vertical, v_Horizontal);
                }
                else
                {
                    pointer += 2; // Parse through 2 operands.
                }
                break;
                
            case TR_ANIMCOMMAND_EMPTYHANDS:
                ///@FIXME: Behaviour is yet to be discovered.
                break;
                
            case TR_ANIMCOMMAND_KILL:
                // This command executes ONLY at the end of animation. 
                if(entity->current_frame == entity->model->animations[entity->current_animation].frames_count - 1)
                {
                    if(entity->character)
                    {
                        entity->character->cmd.kill = 1;
                    }
                    else
                    {
                        
                    }
                }
                
                break;
                
            case TR_ANIMCOMMAND_PLAYSOUND:
                ///@FIXME: This command should play desired sound effect.
                bool surface_flag[2];
                int16_t sound_index;
                
                if(entity->current_frame == *++pointer)
                {                        
                    sounds_played--;
                    sounds_played = (sounds_played <= 0)?(100):(sounds_played);
                
                    sound_index     =  *++pointer &  0x3FFF;
                    Audio_Send(sound_index, entity->ID, TR_SOUND_EMITTER_ENTITY);
                }
                else
                {
                    pointer++;
                }
                break;
                
            case TR_ANIMCOMMAND_PLAYEFFECT:
                if(entity->current_frame == *++pointer)
                {
                    switch(*++pointer & 0x3FFF)
                    {
                        case TR_EFFECT_CHANGEDIRECTION:
                            entity->angles[0] += 180.0;
                            if(entity->dir_flag == ENT_MOVE_BACKWARD)
                            {
                                entity->dir_flag = ENT_MOVE_FORWARD;
                            }
                            else if(entity->dir_flag == ENT_MOVE_FORWARD)
                            {
                                entity->dir_flag = ENT_MOVE_BACKWARD;
                            }
                            break;
                        case TR_EFFECT_HIDEOBJECT:
                            entity->hide = 1;
                            break;
                        case TR_EFFECT_SHOWOBJECT:
                            entity->hide = 0;
                            break;
                        case TR_EFFECT_PLAYSTEPSOUND:
                            if(*pointer && TR_ANIMCOMMAND_CONDITION_LAND)
                            {
                                //Audio_Send(0, entity->ID, TR_SOUND_EMITTER_ENTITY);
                                
                                sounds_played--;
                                sounds_played = (sounds_played <= 0)?(100):(sounds_played);
                            }
                            else if(*pointer && TR_ANIMCOMMAND_CONDITION_WATER)
                            {
                                sounds_played--;
                                sounds_played = (sounds_played <= 0)?(100):(sounds_played);
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


void Entity_SetAnimation(entity_p entity, int animation, int frame)
{
    animation_frame_p anim;
    long int t;
    btScalar dt;
    
    if(!entity || !entity->model)
    {
        return;
    }

    if(animation < 0 || animation >= entity->model->animation_count)
    {
        animation = 0;
    }
    
    entity->next_bf == NULL;
    entity->lerp = 0.0;
    anim = &entity->model->animations[animation];
    entity->period = (anim->frame_rate < 1)?(1.0 / 30.0):((btScalar)anim->frame_rate / 30.0);
    
    entity->current_stateID = anim->state_id;
    entity->current_animation = animation;
    entity->current_speed = anim->speed;
    entity->current_frame = frame;
    
    entity->frame_time = (btScalar)frame * entity->period;
    t = (entity->frame_time) / entity->period;
    dt = entity->frame_time - (btScalar)t * entity->period;
    entity->frame_time = (btScalar)frame * entity->period + dt;
    
    Entity_UpdateCurrentBoneFrame(entity);
}
    

struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim)
{
    int i, j;
    state_change_p ret = anim->state_change;
    
    if(state_change_anim < 0)
    {
        return NULL;
    }
    
    for(i=0;i<anim->state_change_count;i++,ret++)
    {
        for(j=0;j<ret->anim_dispath_count;j++)
        {
            if(ret->anim_dispath[j].next_anim == state_change_anim)
            {
                return ret;
            }
        }
    }
    
    return NULL;
}


struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, int id)
{
    int i;
    state_change_p ret = anim->state_change;
    
    if(id < 0)
    {
        return NULL;
    }
    
    for(i=0;i<anim->state_change_count;i++,ret++)
    {
        if(ret->ID == id)
        {
            return ret;
        }
    }
    
    return NULL;
}


int Entity_GetAnimDispatchCase(struct entity_s *ent, int id)
{
    int i, j;
    animation_frame_p anim = ent->model->animations + ent->current_animation;
    state_change_p stc = anim->state_change;
    anim_dispath_p disp;
    
    if(id < 0)
    {
        return -1;
    }
    
    for(i=0;i<anim->state_change_count;i++,stc++)
    {
        if(stc->ID == id)
        {
            disp = stc->anim_dispath;
            for(j=0;j<stc->anim_dispath_count;j++,disp++)
            {
                if((disp->frame_high >= disp->frame_low) && (ent->current_frame >= disp->frame_low) && (ent->current_frame <= disp->frame_high))// ||
                   //(disp->frame_high <  disp->frame_low) && ((ent->current_frame >= disp->frame_low) || (ent->current_frame <= disp->frame_high)))
                {
                    return j;
                }
            }
        }
    }
    
    return -1;
}


/*
 * Next frame and next anim calculation function. 
 */
void Entity_GetNextFrame(const entity_p entity, btScalar time, struct state_change_s *stc, int *frame, int *anim)
{
    animation_frame_p curr_anim = entity->model->animations + entity->current_animation;
    anim_dispath_p disp;
    int i;

    *frame = (entity->frame_time + time) / entity->period;
    *frame = (*frame >= 0.0)?(*frame):(0.0);                                    // paranoid checking
    *anim = entity->current_animation;
    
    /*
     * Flag has a highest priority 
     */
    if(entity->anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(*frame >= curr_anim->frames_count - 1)
        {
            *frame = curr_anim->frames_count - 1;
            *anim = entity->current_animation;                                  // paranoid dublicate
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
            *anim = curr_anim->next_anim->ID;
            return;
        }
        
        *frame %= curr_anim->frames_count;
        *anim = entity->current_animation;                                      // paranoid dublicate
        return;
    }
    
    /*
     * State change check
     */
    if(stc)
    {
        disp = stc->anim_dispath;
        for(i=0;i<stc->anim_dispath_count;i++,disp++)
        {
            if((disp->frame_high >= disp->frame_low) && (*frame >= disp->frame_low) && (*frame <= disp->frame_high))// ||
               //(disp->frame_high <  disp->frame_low) && ((*frame >= disp->frame_low) || (*frame <= disp->frame_high)))
            {
                *anim = disp->next_anim;
                *frame = disp->next_frame;
                //*frame = (disp->next_frame + (*frame - disp->frame_low)) % entity->model->animations[disp->next_anim].frames_count;
                return;                                                         // anim was changed
            }
        }
    }
}


/**
 *
 */
int Entity_Frame(entity_p entity, btScalar time, int state_id)
{
    int frame, anim, ret = 0;
    long int t;
    btScalar dt;
    animation_frame_p af;
    state_change_p stc;
    
    if(!entity || !entity->model || !entity->model->animations || ((entity->model->animations->frames_count == 1) && (entity->model->animation_count == 1)))
    {
        return 0;
    }

    entity->next_bf = NULL;
    entity->lerp = 0.0;
    stc = Anim_FindStateChangeByID(entity->model->animations + entity->current_animation, state_id);
    Entity_GetNextFrame(entity, time, stc, &frame, &anim);
    if(anim != entity->current_animation)
    {
        Entity_SetAnimation(entity, anim, frame);
        stc = NULL;
        ret = 2;
    }
    else if(entity->current_frame != frame)
    {
        ret = 1;
        entity->current_frame = frame;
    }
    
    if(ret)
    {
        Entity_DoAnimCommands(entity, ret);
    }
    
    af = entity->model->animations + entity->current_animation;
    entity->frame_time += time;
    
    t = (entity->frame_time) / entity->period;
    dt = entity->frame_time - (btScalar)t * entity->period;
    entity->frame_time = (btScalar)frame * entity->period + dt;
    entity->lerp = (entity->smooth_anim)?(dt / entity->period):(0.0);
    
    Entity_GetNextFrame(entity, entity->period, stc, &frame, &anim);
    entity->next_bf = entity->model->animations[anim].frames + frame;   

    /*
     * Update acceleration
     */
    if(entity->character)
    {
        entity->current_speed += time * entity->character->speed_mult * (btScalar)af->accel_hi;
    }
    
    Entity_UpdateCurrentBoneFrame(entity);
    Entity_UpdateRigidBody(entity);
    if(ret)
    {
        Entity_RebuildBV(entity);
    }
    return ret;
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity_RebuildBV(entity_p ent)
{
    if(!ent || !ent->model)
    {
        return;
    }

    /*
     * get current BB from animation
     */
    switch(ent->bv->bv_type)
    {
        case BV_CYLINDER:
            BV_TransformZZ(ent->bv, ent->bf.bb_min[2] + ent->bv->r, ent->bf.bb_max[2]);
            return;

        case BV_BOX:
            BV_RebuildBox(ent->bv, ent->bf.bb_min, ent->bf.bb_max);
            BV_Transform(ent->bv);
            return;

        case BV_SPHERE:
            Mat4_vec3_mul_macro(ent->bv->centre, ent->bv->transform, ent->bv->base_centre);
            return;

        case BV_FREEFORM:
            BV_Transform(ent->bv);
            return;

        case BV_EMPTY:
        default:
            return;
    };
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
