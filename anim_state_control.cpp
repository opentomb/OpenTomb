
#include <stdlib.h>
#include <stdio.h>

#include "vmath.h"
#include "polygon.h"
#include "engine.h"
#include "world.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"
#include "camera.h"
#include "render.h"
#include "portal.h"
#include "system.h"
#include "script.h"
#include "console.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "vt/tr_versions.h"

/*
 * WALL CLIMB:
 * preframe do {save pos}
 * postframe do {if(out) {load pos; do command;}}
 */

#define PENETRATION_TEST_OFFSET     (0.16 * ent->character->ry)
#define WALK_FORWARD_OFFSET         (96.0)        ///@FIXME: find real offset
#define WALK_FORWARD_STEP_UP        (256.0)       // by bone frame bb
#define RUN_FORWARD_OFFSET          (128.0)       ///@FIXME: find real offset
#define RUN_FORWARD_STEP_UP         (320.0)       // by bone frame bb
#define CRAWL_FORWARD_OFFSET        (256.0)
#define LARA_HANG_WALL_DISTANCE     (128.0 - 24.0)
#define LARA_HANG_VERTICAL_EPSILON  (64.0)
#define LARA_HANG_VERTICAL_OFFSET   (12.0)        // in original is 0, in real life hands are little more higher than edge
#define LARA_TRY_HANG_WALL_OFFSET   (72.0)        // It works more stable than 32 or 128
#define LARA_HANG_SENSOR_Z          (800.0)       // It works more stable than 1024 (after collision critical fix, of course)

#define OSCILLATE_HANG_USE 0

void ent_set_on_floor(entity_p ent)
{
    ent->move_type = MOVE_ON_FLOOR;
    ent->transform[12 + 2] = ent->character->climb.point[2];
    //vec3_copy(ent->transform+12,ent->character->climb.point);
}

void ent_set_underwater(entity_p ent)
{
    ent->move_type = MOVE_UNDER_WATER;
}

void ent_out_of_water(entity_p ent)
{
    btScalar *v = ent->character->climb.point;

    ent->transform[12 + 0] = v[0];
    ent->transform[12 + 1] = v[1];
    ent->transform[12 + 2] = v[2];
}

void ent_to_edge_climb(entity_p ent)
{
    btScalar *v = ent->character->climb.point;

    ent->transform[12 + 0] = v[0] - ent->transform[4 + 0] * ent->bf.bb_max[1];
    ent->transform[12 + 1] = v[1] - ent->transform[4 + 1] * ent->bf.bb_max[1];
    ent->transform[12 + 2] = v[2] - ent->bf.bb_max[2];
}

/**
 * Current animation != current state - use original TR state concept!
 */
int State_Control_Lara(struct entity_s *ent, struct character_command_s *cmd)
{
    int i;
    btScalar t, *pos = ent->transform + 12;
    btScalar offset[3], move[3];
    height_info_t next_fc, *curr_fc;
    climb_info_t *climb = &ent->character->climb;

    curr_fc = &ent->character->height_info;
    next_fc.sp = curr_fc->sp;
    next_fc.cb = ent->character->ray_cb;
    next_fc.cb->m_closestHitFraction = 1.0;
    next_fc.cb->m_collisionObject = NULL;
    next_fc.ccb = ent->character->convex_cb;
    next_fc.ccb->m_closestHitFraction = 1.0;
    next_fc.ccb->m_hitCollisionObject = NULL;

    ent->anim_flags = ANIM_NORMAL_CONTROL;
    Character_UpdateCurrentHeight(ent);

    // last frame is a stick... but is some cases works absolute correctly
    char   last_frame = ent->model->animations[ent->current_animation].frames_count <= ent->current_frame + 1;
    int8_t low_vertical_space = (curr_fc->floor_hit && curr_fc->ceiling_hit && (curr_fc->ceiling_point.m_floats[2] - curr_fc->floor_point.m_floats[2] < ent->character->Height));

    
    if(cmd->kill)   // Stop any music, if Lara is dead.
    {
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_ONESHOT);
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_CHAT);
    }

 /*
 * - On floor animations
 * - Climbing animations
 * - Landing animations
 * - Free fall animations
 * - Water animations
 */

    ent->character->complex_collision = 0x00;   // by default - simple collision
    
    switch(ent->last_state)
    {
        /*
         * Base onfloor animations
         */
        case TR_STATE_LARA_STOP:
            ent->dir_flag = ENT_STAY;
            cmd->rot[0] = 0;
            cmd->crouch |= low_vertical_space;

            Character_Lean(ent, cmd, 0.0);

            if( (climb->can_hang &&
                (climb->next_z_space >= ent->character->Height) &&
                (ent->move_type == MOVE_CLIMBING)) ||
                (ent->current_animation == TR_ANIMATION_LARA_STAY_SOLID) )
            {
                ent->move_type = MOVE_ON_FLOOR;
            }

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                ent->dir_flag = ENT_STAY;
            }
            else if(cmd->kill)
            {
                ent->next_state = TR_STATE_LARA_DEATH;
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                if(cmd->jump)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                    Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, ent->ID);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
                }
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                if(cmd->jump)
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                    Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, ent->ID);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                }
            }
            else if(cmd->jump)
            {
                ent->next_state = TR_STATE_LARA_JUMP_PREPARE;       // jump sideways
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
            }
            else if(cmd->crouch)
            {
                ent->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            /*else if(cmd->action)///@FIXME: Need to check if there is actually a block in front and also to activate it when pushed/pulled, don't push if next floor (after block) will not allow it!
            {
                ent->next_state = TR_STATE_LARA_PUSHABLE_GRAB;
            }*/
            else if(cmd->move[0] == 1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
                    offset[2] += ent->bf.bb_max[2];
                    vec3_add(offset, offset, pos);
                    Character_GetHeightInfo(offset, &next_fc);
                    if((cmd->horizontal_collide == 0) && (next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height)))
                    {
                        ent->move_type = MOVE_ON_FLOOR;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth))
                        {
                            ent->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ent->next_state = TR_STATE_LARA_WALK_FORWARD;
                        }
                    }
                }       // end IF CMD->SHIFT
                else
                {
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
                    offset[2] += ent->bf.bb_max[2];
                    i = Character_CheckNextStep(ent, offset, &next_fc);
                    if((cmd->horizontal_collide == 0) && (!Character_HasStopSlant(ent, &next_fc)))
                    {
                        ent->move_type = MOVE_ON_FLOOR;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth))
                        {
                            ent->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                        }
                    }
                }

                if( (cmd->action) &&
                    ((ent->current_animation == TR_ANIMATION_LARA_STAY_IDLE)        ||
                     (ent->current_animation == TR_ANIMATION_LARA_STAY_SOLID)       ||
                     (ent->current_animation == TR_ANIMATION_LARA_WALL_SMASH_LEFT)  ||
                     (ent->current_animation == TR_ANIMATION_LARA_WALL_SMASH_RIGHT)) )
                {
                    t = ent->character->ry + LARA_TRY_HANG_WALL_OFFSET;
                    vec3_mul_scalar(offset, ent->transform + 4, t);
                    offset[2] += DEFAULT_CLIMB_UP_HEIGHT;
                    *climb = Character_CheckClimbability(ent, offset, &next_fc, DEFAULT_CLIMB_UP_HEIGHT);

                    if(  climb->edge_hit                                                         &&
                        (climb->next_z_space >= ent->character->Height)                          &&
                        (pos[2] + ent->character->max_step_up_height < next_fc.floor_point[2])   &&
                        (pos[2] + 2944.0 >= next_fc.floor_point[2])                              &&
                        (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component)  ) // trying to climb on
                    {
                        if(pos[2] + 640.0 >= next_fc.floor_point[2])
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 512.0;
                            vec3_copy(climb->point, next_fc.floor_point);
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                            ent->character->no_fix = 0x01;
                            ent->onAnimChange = ent_set_on_floor;
                        }
                        else if(pos[2] + 896.0 >= next_fc.floor_point[2])
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 768.0;
                            vec3_copy(climb->point, next_fc.floor_point);
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                            ent->character->no_fix = 0x01;
                            ent->onAnimChange = ent_set_on_floor;
                        }
                        else if(pos[2] + 1920.0 >= next_fc.floor_point[2])
                        {
                            ent->next_state = TR_STATE_LARA_JUMP_UP;
                        }
                    }   // end IF MOVE_CLIMBING

                    *climb = Character_CheckWallsClimbability(ent);
                    if(climb->wall_hit)
                    {
                        ent->next_state = TR_STATE_LARA_JUMP_UP;
                        break;
                    }
                }
            }       // end CMD->MOVE FORWARD
            else if(cmd->move[0] == -1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    vec3_mul_scalar(offset, ent->transform + 4, -WALK_FORWARD_OFFSET);
                    offset[2] += ent->bf.bb_max[2];
                    vec3_add(offset, offset, pos);
                    Character_GetHeightInfo(offset, &next_fc);
                    if((cmd->horizontal_collide == 0) && (next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height)))
                    {
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ent->next_state = TR_STATE_LARA_WALK_BACK;
                    }
                }
                else    // RUN BACK
                {
                    vec3_mul_scalar(move, ent->transform + 4, - PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    if(cmd->horizontal_collide == 0)
                    {
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth))
                        {
                            ent->next_state = TR_STATE_LARA_WALK_BACK;
                        }
                        else
                        {
                            ent->next_state = TR_STATE_LARA_RUN_BACK;
                        }
                    }
                }
            }       // end CMD->MOVE BACK
            else if(cmd->move[1] == 1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    vec3_mul_scalar(offset, ent->transform + 0, RUN_FORWARD_OFFSET);
                    offset[2] += ent->bf.bb_max[2];
                    i = Character_CheckNextStep(ent, offset, &next_fc);
                    if((cmd->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                    {
                        cmd->rot[0] = 0.0;
                        ent->dir_flag = ENT_MOVE_RIGHT;
                        ent->next_state = TR_STATE_LARA_WALK_RIGHT;
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ent->dir_flag = ENT_MOVE_RIGHT;
                    ent->next_state = TR_STATE_LARA_TURN_RIGHT_SLOW;
                }
            }       // end MOVE RIGHT
            else if(cmd->move[1] == -1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                    Character_CheckNextPenetration(ent, cmd, move);
                    vec3_mul_scalar(offset, ent->transform + 0, -RUN_FORWARD_OFFSET);
                    offset[2] += ent->bf.bb_max[2];
                    i = Character_CheckNextStep(ent, offset, &next_fc);
                    if((cmd->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                    {
                        cmd->rot[0] = 0.0;
                        ent->dir_flag = ENT_MOVE_LEFT;
                        ent->next_state = TR_STATE_LARA_WALK_LEFT;
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ent->dir_flag = ENT_MOVE_LEFT;
                    ent->next_state = TR_STATE_LARA_TURN_LEFT_SLOW;
                }
            }       // end MOVE LEFT
            break;

        case TR_STATE_LARA_JUMP_PREPARE:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);

            if(cmd->slide == CHARACTER_SLIDE_BACK)      // Slide checking is only for jumps direction correction!
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                cmd->move[0] = -1;
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                cmd->move[0] = 1;
            }

            if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                    ent->next_state = TR_STATE_LARA_JUMP_FORWARD;       // jump forward
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                    ent->next_state = TR_STATE_LARA_JUMP_BACK;               // jump backward
            }
            else if(cmd->move[1] == 1)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
                vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                    ent->next_state = TR_STATE_LARA_JUMP_LEFT;               // jump right
            }
            else if(cmd->move[1] ==-1)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                    ent->next_state = TR_STATE_LARA_JUMP_RIGHT;               // jump left
            }
            break;

        case TR_STATE_LARA_JUMP_BACK:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                ent->next_state = TR_STATE_LARA_STOP;       // landing
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_FORWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else if(cmd->roll)
            {
                ent->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            else
            {
                ent->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_JUMP_LEFT:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                ent->next_state = TR_STATE_LARA_STOP;       // landing
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_RIGHT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                ent->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_JUMP_RIGHT:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                ent->next_state = TR_STATE_LARA_STOP;       // landing
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_LEFT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                ent->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_RUN_BACK:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_BACKWARD;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                if(cmd->action)
                {
                    ent->speed.m_floats[0] *= 0.7;
                    ent->speed.m_floats[1] *= 0.7;
                }

                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            break;

        case TR_STATE_LARA_TURN_RIGHT_SLOW:
            cmd->rot[0] *= 0.7;
            ent->dir_flag = ENT_STAY;
            Character_Lean(ent, cmd, 0.0);

            if(cmd->move[0] == 1)
            {
                if(cmd->shift == 1)
                {
                    ent->next_state = TR_STATE_LARA_WALK_FORWARD;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(cmd->move[1] == 1)
            {
                if(last_frame)
                {
                    ent->next_state = TR_STATE_LARA_TURN_FAST;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_TURN_LEFT_SLOW:
            cmd->rot[0] *= 0.7;
            ent->dir_flag = ENT_STAY;
            Character_Lean(ent, cmd, 0.0);

            if(cmd->move[0] == 1)
            {
                if(cmd->shift == 1)
                {
                    ent->next_state = TR_STATE_LARA_WALK_FORWARD;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(cmd->move[1] == -1)
            {
                if(last_frame)
                {
                    ent->next_state = TR_STATE_LARA_TURN_FAST;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_TURN_FAST:
            // 65 - wade
            ent->dir_flag = ENT_STAY;
            Character_Lean(ent, cmd, 0.0);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 1)
            {
                ent->next_state = TR_STATE_LARA_WALK_FORWARD;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[1] == 0)
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

            /*
             * RUN AND WALK animations section
             */
        case TR_STATE_LARA_RUN_FORWARD:
            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            offset[2] += ent->bf.bb_max[2];
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->crouch |= low_vertical_space;

            Character_Lean(ent, cmd, 6.0);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_DEATH;
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
            }
            else if(Character_HasStopSlant(ent, &next_fc))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(cmd->crouch == 1)
            {
                ent->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if((cmd->crouch == 0) && (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                i = Entity_GetAnimDispatchCase(ent, 2);                         // MOST CORRECT STATECHANGE!!!
                if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else //if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
                offset[2] += 1024.0;
                if(ent->current_animation == TR_ANIMATION_LARA_STAY_TO_RUN)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    Controls_JoyRumble(200.0, 200);

                    if(cmd->move[0] == 1)
                    {
                        i = Entity_GetAnimDispatchCase(ent, 2);
                        if(i == 1)
                        {
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                        }
                        else
                        {
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                        }
                    }
                    else
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_SOLID, 0);
                    }
                }
                Character_UpdateCurrentSpeed(ent, 0);
            }
            else if(cmd->move[0] == 1)                                          // If we continue running...
            {
                if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth))
                {
                    ent->next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else if(cmd->shift == 1)
                {
                    ent->next_state = TR_STATE_LARA_WALK_FORWARD;
                }
                else if((cmd->jump == 1) && (ent->last_animation != TR_ANIMATION_LARA_STAY_TO_RUN))
                {
                    ent->next_state = TR_STATE_LARA_JUMP_FORWARD;
                }
                else if(cmd->roll == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->sprint == 1)
                {
                    ent->next_state = TR_STATE_LARA_SPRINT;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_SPRINT:
            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            Character_Lean(ent, cmd, 12.0);
            offset[2] += ent->bf.bb_max[2];
            i = Character_CheckNextStep(ent, offset, &next_fc);
            cmd->crouch |= low_vertical_space;

            if(!ent->character->opt.sprint)
            {
                ent->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            else if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_RUN_FORWARD;    // Normal run then die
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if((next_fc.floor_normale.m_floats[2] < ent->character->critical_slant_z_component) && (i > CHARACTER_STEP_HORIZONTAL))
            {
                ent->current_speed = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if((ent->move_type == MOVE_CLIMBING) || ((i > CHARACTER_STEP_HORIZONTAL) && (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component)))  // trying to climb on
            {
                ent->next_state = TR_STATE_LARA_RUN_FORWARD;     // Interrupt sprint
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);

                i = Entity_GetAnimDispatchCase(ent, 2);                         // tested!
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                }
                else if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                }
                Character_UpdateCurrentSpeed(ent, 0);
            }
            else if(cmd->sprint == 0)
            {
                if(cmd->move[0] == 1)
                {
                    ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_STOP;
                }
            }
            else
            {
                if(cmd->jump == 1)
                {
                    ent->next_state = TR_STATE_LARA_SPRINT_ROLL;
                }
                else if(cmd->roll == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->crouch == 1)
                {
                    ent->next_state = TR_STATE_LARA_CROUCH_IDLE;
                }
                else if(cmd->move[0] == 0)
                {
                    ent->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_FORWARD:
            cmd->rot[0] *= 0.4;
            Character_Lean(ent, cmd, 0.0);

            vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
            offset[2] += ent->bf.bb_max[2];
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                /*
                 * climb up
                 */
                ent->dir_flag = ENT_STAY;
                i = Entity_GetAnimDispatchCase(ent, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                /*
                 * climb down
                 */
                ent->dir_flag = ENT_STAY;
                i = Entity_GetAnimDispatchCase(ent, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                    vec3_copy(climb->point, next_fc.floor_point);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else //if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
                    vec3_copy(climb->point, next_fc.floor_point);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((cmd->horizontal_collide & 0x01) || (i < CHARACTER_STEP_DOWN_BIG || i > CHARACTER_STEP_UP_BIG) || (low_vertical_space))
            {
                /*
                 * too high
                 */
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(cmd->move[0] != 1)
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            else if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth))
            {
                ent->next_state = TR_STATE_LARA_WADE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 0)
            {
                ent->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            break;


        case TR_STATE_LARA_WADE_FORWARD:
            cmd->rot[0] *= 0.4;
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(cmd->move[0] == 1)
            {
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
            }

            if(!curr_fc->floor_hit || ent->move_type == MOVE_FREE_FALLING)      // free fall, next swim
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water)
            {
                if((curr_fc->water_level - curr_fc->floor_point.m_floats[2] <= ent->character->wade_depth))
                {
                    // run / walk case
                    if((cmd->move[0] == 1) && (cmd->horizontal_collide == 0))
                    {
                        ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                    else
                    {
                        ent->next_state = TR_STATE_LARA_STOP;
                    }
                }
                else if(curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->Height)
                {
                    // swim case
                    if(curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->Height + ent->character->max_step_up_height)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);                                    // swim underwater
                    }
                    else
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_ONWATER_IDLE, 0);                                       // swim onwater
                        ent->move_type = MOVE_ON_WATER;
                        pos[2] = curr_fc->water_level;
                    }
                }
                else if(curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->wade_depth)              // wade case
                {
                    if((cmd->move[0] != 1) || (cmd->horizontal_collide != 0))
                    {
                        ent->next_state = TR_STATE_LARA_STOP;
                    }
                }
            }
            else                                                                // no water, stay or run / walk
            {
                if((cmd->move[0] == 1) && (cmd->horizontal_collide == 0))
                {
                    ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_BACK:
            cmd->rot[0] *= 0.4;
            vec3_mul_scalar(offset, ent->transform + 4, -WALK_FORWARD_OFFSET);
            offset[2] += ent->bf.bb_max[2];
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if((i < CHARACTER_STEP_DOWN_BIG) || (i > CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {                                                                   // works correct
                ent->dir_flag = ENT_STAY;
                i = Entity_GetAnimDispatchCase(ent, TR_STATE_LARA_STOP);
                if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                }
                else //if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, 0);
                    vec3_copy(pos, next_fc.floor_point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                }
            }
            else if(cmd->move[0] == -1)
            {
                if(cmd->shift)
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    ent->next_state = TR_STATE_LARA_WALK_BACK;
                }
                else
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    ent->next_state = TR_STATE_LARA_RUN_BACK;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_WALK_LEFT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == -1 && cmd->jump == 0 && cmd->shift)
            {
                vec3_mul_scalar(offset, ent->transform + 0, -RUN_FORWARD_OFFSET);  // not an error - RUN_... more correct here
                offset[2] += ent->bf.bb_max[2];
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    if(!curr_fc->water || (curr_fc->floor_point.m_floats[2] + ent->character->Height > curr_fc->water_level)) // if (floor_hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ent->next_state = TR_STATE_LARA_ONWATER_LEFT;
                        if(last_frame)
                        {
                            pos[2] = curr_fc->water_level;
                            ent->move_type = MOVE_ON_WATER;
                        }
                    }
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_WALK_RIGHT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift)
            {
                vec3_mul_scalar(offset, ent->transform + 0, RUN_FORWARD_OFFSET);// not an error - RUN_... more correct here
                offset[2] += ent->bf.bb_max[2];
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    if(!curr_fc->water || (curr_fc->floor_point.m_floats[2] + ent->character->Height > curr_fc->water_level)) // if (floor_hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ent->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                        if(last_frame)
                        {
                            pos[2] = curr_fc->water_level;
                            ent->move_type = MOVE_ON_WATER;
                        }
                    }
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

            /*
             * Slide animations section
             */
        case TR_STATE_LARA_SLIDE_BACK:
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                ent->speed.m_floats[0] *= 0.7;
                ent->speed.m_floats[1] *= 0.7;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->slide == 0)
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            else if(cmd->slide != 0 && cmd->jump == 1)
            {
                ent->next_state = TR_STATE_LARA_JUMP_BACK;
                if(last_frame)
                {
                    cmd->slide = 0x00;
                }
            }
            break;

        case TR_STATE_LARA_SLIDE_FORWARD:
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                ent->speed.m_floats[0] *= 0.2;
                ent->speed.m_floats[1] *= 0.2;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->slide == 0)
            {
                if((cmd->move[0] == 1) && (engine_world.version >= TR_III))
                {
                     ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                     ent->next_state = TR_STATE_LARA_STOP;  // stop
                }
            }
            else if(cmd->slide != 0 && cmd->jump == 1)
            {
                ent->next_state = TR_STATE_LARA_JUMP_FORWARD;       // jump
            }
            break;

            /*
             * Misk animations
             */
        case TR_STATE_LARA_PUSHABLE_GRAB:

			ent->move_type = MOVE_ON_FLOOR;
                        
            if(cmd->action == 1)//If Lara is grabbing the block
            {
				ent->dir_flag = ENT_STAY;
                ent->anim_flags = ANIM_LOOP_LAST_FRAME;         //We hold it (loop last frame)

                if(cmd->move[0] == 1)//If player press up push
                {
					 ent->dir_flag = ENT_MOVE_FORWARD;
                     ent->anim_flags = ANIM_NORMAL_CONTROL;
                     ent->next_state = TR_STATE_LARA_PUSHABLE_PUSH;
                }
                else if(cmd->move[0] == -1)//If player press down pull
                {
					 ent->dir_flag = ENT_MOVE_BACKWARD;
                     ent->anim_flags = ANIM_NORMAL_CONTROL;
                     ent->next_state = TR_STATE_LARA_PUSHABLE_PULL;
                }
            }
            else//Lara has let go of the block
            {
				ent->dir_flag = ENT_STAY;
                ent->anim_flags = ANIM_NORMAL_CONTROL;          //We no longer loop last frame
                ent->next_state = TR_STATE_LARA_STOP;           //Switch to next Lara state
            }
            break;
        case TR_STATE_LARA_PUSHABLE_PUSH:
            if(cmd->action == 0)//For TOMB4/5 If Lara is pushing and action let go, don't push
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            else//Or we keep our last state (pushing)
            {
                ent->next_state = ent->last_state; //Repeat last state (TR4/5)
            }
            break;
        case TR_STATE_LARA_PUSHABLE_PULL:
            if(cmd->action == 0)//For TOMB4/5 If Lara is pulling and action let go, don't pull
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            else//Or we keep our last state (pulling)
            {
                ent->next_state = ent->last_state; //Repeat last state (TR4/5)
            }
            break;
        case TR_STATE_LARA_ROLL_FORWARD:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0); ///@FIXME: check
            }
            else if(low_vertical_space)
            {
                ent->dir_flag = ENT_STAY;
            }
            break;

        case TR_STATE_LARA_ROLL_BACKWARD:
            ent->character->complex_collision = 0x01;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(low_vertical_space)
            {
                ent->dir_flag = ENT_STAY;
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            break;

        /*
         * Climbing section
         */
        case TR_STATE_LARA_JUMP_UP:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            if((cmd->action == 1) && (ent->move_type != MOVE_WALLS_CLMB) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * ent->speed.m_floats[2];
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    vec3_set_zero(ent->speed.m_floats);

                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                }
                else
                {
                    *climb = Character_CheckWallsClimbability(ent);
                    if(climb->wall_hit)
                    {
                        ent->move_type = MOVE_WALLS_CLMB;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_HANG_IDLE, -1);
                        break;
                    }
                }
            }

            if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == -1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
            }
            else if(cmd->move[1] == 1)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
            }
            else if(cmd->move[1] == -1)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
            }
            else
            {
                ent->dir_flag = ENT_STAY;
            }

            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if((curr_fc->ceiling_climb) && (curr_fc->ceiling_hit) && (pos[2] + ent->bf.bb_max[2] > curr_fc->ceiling_point.m_floats[2] - 64.0))
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                if(last_frame)
                {
                    ent->move_type = MOVE_CEILING_CLMB;
                    pos[2] = curr_fc->ceiling_point.m_floats[2] - ent->bf.bb_max[2];
                }
            }
            else if((cmd->action == 1) && (ent->move_type == MOVE_CLIMBING))
            {
                ent->next_state = TR_STATE_LARA_HANG;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_HANG_IDLE, -1);
            }
            else if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                ent->next_state = TR_STATE_LARA_STOP;                           // landing immediately
            }
            else
            {
                if(ent->speed.m_floats[2] < -FREE_FALL_SPEED_2)                 // next free fall stage
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    ent->next_state = TR_STATE_LARA_FREEFALL;
                }
                break;
            }
            break;

        case TR_STATE_LARA_REACH:
            ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                break;
            }

            if((cmd->action == 1) && (ent->move_type != MOVE_CLIMBING) && (ent->move_type != MOVE_WALLS_CLMB))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * ent->speed.m_floats[2];
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit && climb->can_hang)
                {
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    vec3_set_zero(ent->speed.m_floats);
                }

                // If Lara is moving backwards off the ledge we want to move Lara slightly forwards
                // depending on the current angle.

                if(ent->dir_flag == ENT_MOVE_BACKWARD)
                {
                    float old_speed = ent->speed.m_floats[2]; ///@FIXME Please find another way of doing this!
                    vec3_mul_scalar(ent->speed.m_floats, ent->transform+4, t);
                    ent->speed.m_floats[2] = old_speed;


                    if(ent->angles[0] > 45.0 && ent->angles[0] < 135.0)
                    {
                        ent->speed.m_floats[0] = -FREE_FALL_SPEED_1 / 2;
                    }
                    else if(ent->angles[0] > 135.0 && ent->angles[0] < 225.0)
                    {
                        ent->speed.m_floats[1] = -FREE_FALL_SPEED_1 / 2;
                    }
                    else if(ent->angles[0] > 225.0 && ent->angles[0] < 315.0)
                    {
                        ent->speed.m_floats[0] = FREE_FALL_SPEED_1 / 2;
                    }
                    else if(ent->angles[0] > 315.0 || ent->angles[0] < 45.0)
                    {
                        ent->speed.m_floats[1] = FREE_FALL_SPEED_1 / 2;
                    }
                }
            }

            if((ent->move_type != MOVE_CLIMBING) && (cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR))
            {
                ent->next_state = TR_STATE_LARA_STOP;                           // middle landing
                break;
            }

            if((ent->speed.m_floats[2] < -FREE_FALL_SPEED_2))
            {
                ent->move_type = MOVE_FREE_FALLING;
                ent->next_state = TR_STATE_LARA_FREEFALL;
                break;
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->speed.m_floats);
                ent->next_state = TR_STATE_LARA_HANG;
                ent->onAnimChange = ent_to_edge_climb;
#if OSCILLATE_HANG_USE
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                Character_CheckNextPenetration(ent, cmd, move);
                ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                if(!cmd->horizontal_collide)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    ent_to_edge_climb(ent);
                }
#endif
            }
            else if((curr_fc->ceiling_climb) && (curr_fc->ceiling_hit) && (pos[2] + ent->bf.bb_max[2] > curr_fc->ceiling_point.m_floats[2] - 64.0))
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                if(last_frame)
                {
                    ent->move_type = MOVE_CEILING_CLMB;
                    pos[2] = curr_fc->ceiling_point.m_floats[2] - ent->bf.bb_max[2];
                }
            }
            break;

        case TR_STATE_LARA_HANDSTAND:
        case TR_STATE_LARA_GRABBING:
            cmd->rot[0] = 0;
            ent->character->no_fix = 0x01;
            break;

        case TR_STATE_LARA_HANG:                               ///@FIXME: does not works correct in TR3+ versions - ceiling climb IDLE
            //ent->character->complex_collision = 0x00;
            cmd->rot[0] = 0.0;

            if(ent->move_type == MOVE_WALLS_CLMB)
            {
                if(cmd->action)
                {
                    if((climb->wall_hit == 0x02) && (cmd->move[0] == 0) && (cmd->move[1] == 0))
                    {
                        ent->next_state = TR_STATE_LARA_LADDER_IDLE;
                    }
                    else if(cmd->move[0] == 1)             // UP
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LADDER_UP_HANDS, 0);
                    }
                    else if(cmd->move[0] ==-1)             // DOWN
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LADDER_DOWN_HANDS, 0);
                    }
                    else if(cmd->move[1] == 1)
                    {
                        ent->dir_flag = ENT_MOVE_RIGHT;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_RIGHT, 0); // edge climb right
                    }
                    else if(cmd->move[1] ==-1)
                    {
                        ent->dir_flag = ENT_MOVE_LEFT;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_LEFT, 0); // edge climb right
                    }
                    else if(climb->wall_hit == 0x00)
                    {
                        ent->move_type = MOVE_FREE_FALLING;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    }
                    else
                    {
                        ent->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
                    }
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                }
                break;
            }

            if(cmd->action == 1)                                                // we have to update climb point every time so entity can move
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->can_hang)
                {
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                }
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                if(cmd->move[0] == 1)
                {
                    if(climb->edge_hit && (climb->next_z_space >= 512.0) && ((climb->next_z_space < ent->character->Height) || (cmd->crouch == 1)))
                    {
                        vec3_copy(climb->point, climb->edge_point.m_floats);
                        ent->next_state = TR_STATE_LARA_CLIMB_TO_CRAWL;         // crawlspace climb
                    }
                    else if(climb->edge_hit && (climb->next_z_space >= ent->character->Height))
                    {
                        vec3_copy(climb->point, climb->edge_point.m_floats);
                        ent->next_state = (cmd->shift)?(TR_STATE_LARA_HANDSTAND):(TR_STATE_LARA_GRABBING);               // climb up
                    }
                    else
                    {
                        pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                        pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                        pos[2] = climb->point[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                        vec3_set_zero(ent->speed.m_floats);
                        ent->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
                    }
                }
                else if(cmd->move[0] ==-1)                                      // check walls climbing
                {
                    *climb = Character_CheckWallsClimbability(ent);
                    if(climb->wall_hit)
                    {
                        ent->move_type = MOVE_WALLS_CLMB;
                    }
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                }
                else if(cmd->move[1] ==-1)
                {
                    vec3_mul_scalar(move, ent->transform + 0, -32.0);
                    Character_CheckNextPenetration(ent, cmd, move);
                    if(cmd->horizontal_collide == 0)
                    {
                        if(last_frame)  //we only want lara to shimmy when last frame is reached!
                        {
                            ent->move_type = ENT_MOVE_LEFT;
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_LEFT, 0);  // edge climb left
                        }
                    }
                    else
                    {
                        ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                    }
                }
                else if(cmd->move[1] == 1)
                {
                    vec3_mul_scalar(move, ent->transform + 0, 32.0);
                    Character_CheckNextPenetration(ent, cmd, move);
                    if(cmd->horizontal_collide == 0)
                    {
                        if(last_frame)  //we only want lara to shimmy when last frame is reached!
                        {
                            ent->dir_flag = ENT_MOVE_RIGHT;
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_RIGHT, 0); // edge climb right
                        }
                    }
                    else
                    {
                        ent->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
                    }
                }
                else
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed.m_floats);
                }
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_STATE_LARA_LADDER_IDLE:
            //ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0;
            ent->move_type = MOVE_WALLS_CLMB;
            ent->dir_flag = ENT_STAY;
            if(ent->move_type == MOVE_CLIMBING)
            {
                ent->next_state = TR_STATE_LARA_GRABBING;
                break;
            }
            if(cmd->action == 0)
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            else if(cmd->jump)
            {
                ent->next_state = TR_STATE_LARA_JUMP_BACK;
                ent->dir_flag = ENT_MOVE_BACKWARD;
            }
            else if(cmd->move[0] == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit && (climb->next_z_space >= 512.0))
                {
                    ent->move_type = MOVE_CLIMBING;
                    ent->next_state = TR_STATE_LARA_GRABBING;
                }
                else if((!curr_fc->ceiling_hit) || (pos[2] + ent->bf.bb_max[2] < curr_fc->ceiling_point.m_floats[2]))
                {
                    ent->next_state = TR_STATE_LARA_LADDER_UP;
                }
            }
            else if(cmd->move[0] == -1)
            {
                ent->next_state = TR_STATE_LARA_LADDER_DOWN;
            }
            else if(cmd->move[1] == 1)
            {
                ent->next_state = TR_STATE_LARA_LADDER_RIGHT;
            }
            else if(cmd->move[1] == -1)
            {
                ent->next_state = TR_STATE_LARA_LADDER_LEFT;
            }
            break;

        case TR_STATE_LARA_LADDER_LEFT:
            //ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_LEFT;
            ent->current_speed = 5.0;
            if((cmd->action == 0) || (ent->character->climb.wall_hit == 0))
            {
                ent->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ent->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_RIGHT:
            //ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_RIGHT;
            ent->current_speed = 5.0;
            if((cmd->action == 0) || (ent->character->climb.wall_hit == 0))
            {
                ent->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ent->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_UP:
            //ent->character->complex_collision = 0x01;
            if(ent->move_type == MOVE_CLIMBING)
            {
                ent->next_state = TR_STATE_LARA_LADDER_IDLE;
                break;
            }

            if(cmd->action && ent->character->climb.wall_hit)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit && (climb->next_z_space >= 512.0))
                {
                    ent->move_type = MOVE_CLIMBING;
                    ent->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
                else if((cmd->move[0] <= 0) && (curr_fc->ceiling_hit || (pos[2] + ent->bf.bb_max[2] >= curr_fc->ceiling_point.m_floats[2])))
                {
                    ent->next_state = TR_STATE_LARA_LADDER_IDLE;
                }

                if(curr_fc->ceiling_hit && (pos[2] + ent->bf.bb_max[2] > curr_fc->ceiling_point.m_floats[2]))
                {
                    pos[2] = curr_fc->ceiling_point.m_floats[2] - ent->bf.bb_max[2];
                }
            }
            else
            {
                // Free fall after stop
                ent->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_DOWN:
            //ent->character->complex_collision = 0x01;
            if(cmd->action && ent->character->climb.wall_hit && (cmd->move[1] < 0))
            {
                if(ent->character->climb.wall_hit != 0x02)
                {
                    ent->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_SHIMMY_LEFT:
            //ent->character->complex_collision = 0x00;
            ent->current_speed = 5.0;
            cmd->rot[0] = 0.0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(cmd->action == 0)
            {
                vec3_set_zero(ent->speed.m_floats);
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(ent->move_type == MOVE_WALLS_CLMB)
            {
                if(!ent->character->climb.wall_hit)
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed.m_floats);
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(cmd->move[1] ==-1)
            {
                t = engine_frame_time * ent->current_speed * ent->character->speed_mult;
                vec3_mul_scalar(move, ent->transform + 0, -t);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    ent->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_SHIMMY_RIGHT:
            //ent->character->complex_collision = 0x00;
            ent->current_speed = 5.0;
            cmd->rot[0] = 0.0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(cmd->action == 0)
            {
                vec3_set_zero(ent->speed.m_floats);
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(ent->move_type == MOVE_WALLS_CLMB)
            {
                if(!ent->character->climb.wall_hit)
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed.m_floats);
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(cmd->move[1] == 1)
            {
                t = engine_frame_time * ent->current_speed * ent->character->speed_mult;
                vec3_mul_scalar(move, ent->transform + 0, t);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    ent->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_ONWATER_EXIT:
            cmd->rot[0] *= 0.0;
            ent->character->no_fix = 0x01;
            ent->onAnimChange = ent_set_on_floor;
            break;

        case TR_STATE_LARA_JUMP_FORWARD:

            Character_Lean(ent, cmd, 4.0);

            ent->character->complex_collision = 0x01;
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if((cmd->action == 0) && (cmd->move[0] == 1) && (cmd->crouch == 0))
                {
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_STOP;
                }
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else if(ent->speed.m_floats[2] <= -FREE_FALL_SPEED_2)
            {
                ent->next_state = TR_STATE_LARA_FREEFALL;           // free falling
            }
            else if(cmd->action == 1)
            {
                if(ent->dir_flag == ENT_MOVE_BACKWARD)
                {
                    if(ent->current_frame > 3 && ent->current_animation == TR_ANIMATION_LARA_FREE_FALL_MIDDLE)///@FIXME This should stop the player from grabbing too early but sometimes it is not consistent? possibly the frame > 3
                    {
                        ent->next_state = TR_STATE_LARA_REACH;
                    }
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_REACH;
                }
            }
            else if(cmd->shift == 1)
            {
                ent->next_state = TR_STATE_LARA_SWANDIVE_BEGIN;     // fly like fish
            }
            else if(cmd->roll)
            {
                ent->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * FREE FALL TO UNDERWATER CASES
             */
        case TR_STATE_LARA_UNDERWATER_DIVING:
            ent->angles[1] = -45.0;
            cmd->rot[1] = 0.0;
            ent->character->complex_collision = 0x01;
            if(last_frame)
            {
                ent->angles[1] = -45.0;
            }
            break;

        case TR_STATE_LARA_FREEFALL:

            Character_Lean(ent, cmd, 1.0);

            ent->character->complex_collision = 0x01;

            ent->speed.m_floats[0] = 0.0; //Reset these to zero so Lara is only falling downwards
            ent->speed.m_floats[1] = 0.0;

            if( (int(ent->speed.m_floats[2]) <=  -FREE_FALL_SPEED_CRITICAL) &&
                (int(ent->speed.m_floats[2]) >= (-FREE_FALL_SPEED_CRITICAL-100)) )
            {
                Audio_Send(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->ID);       // Scream
                ent->speed.m_floats[2] = -FREE_FALL_SPEED_CRITICAL-101;
            }

            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->ID);       // Stop scream

                // Splash sound is hardcoded, beginning with TR3.
                if(engine_world.version > TR_II)
                {
                    Audio_Send(TR_AUDIO_SOUND_SPLASH, TR_AUDIO_EMITTER_ENTITY, ent->ID);
                }
            }
            else if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(ent->speed.m_floats[2] <= -FREE_FALL_SPEED_MAXSAFE)
                {
                    if(!Character_DecreaseHealth(ent, -(ent->speed.m_floats[2] + FREE_FALL_SPEED_MAXSAFE) / 2))
                    {
                        cmd->kill = 1;
                        Controls_JoyRumble(200.0, 500);
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_DEATH, 0);
                    }
                    else
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                    }
                }
                else if(ent->speed.m_floats[2] <= -FREE_FALL_SPEED_2)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
                }

                if(cmd->kill == 1)
                {
                    ent->next_state = TR_STATE_LARA_DEATH;
                    Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->ID);
                }
            }
            else if(cmd->action)
            {
                ent->next_state = TR_STATE_LARA_REACH;
            }
            break;

        case TR_STATE_LARA_SWANDIVE_BEGIN:
            cmd->rot[0] *= 0.4;
            ent->character->complex_collision = 0x01;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                ent->next_state = TR_STATE_LARA_STOP;               // landing - roll
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else
            {
                ent->next_state = TR_STATE_LARA_SWANDIVE_END;       // next stage
            }
            break;

        case TR_STATE_LARA_SWANDIVE_END:
            ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;

            ent->speed.m_floats[0] = 0.0;//Reset these to zero so Lara is only falling downwards
            ent->speed.m_floats[1] = 0.0;

            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                ent->next_state = TR_STATE_LARA_DEATH;
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else if(cmd->jump)
            {
                ent->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * WATER ANIMATIONS
             */
        case TR_STATE_LARA_UNDERWATER_STOP:
            if(ent->move_type != MOVE_UNDER_WATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            break;

        case TR_STATE_LARA_WATER_DEATH:
            if(ent->move_type != MOVE_ON_WATER)
            {
                pos[2] += 1; // go to the air
            }
            break;


        case TR_STATE_LARA_UNDERWATER_FORWARD:
            if(ent->move_type != MOVE_UNDER_WATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(curr_fc->floor_hit && curr_fc->water && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] <= ent->character->max_step_up_height))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_TO_WADE, 0); // go to the air
                ent->move_type = MOVE_ON_FLOOR;
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                if(ent->move_type == MOVE_ON_WATER)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_INERTIA;
            }
            break;

        case TR_STATE_LARA_UNDERWATER_INERTIA:
            if(cmd->kill == 1)
            {
                ent->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            else
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_STOP:
            if((cmd->action == 1) && (cmd->move[0] == 1) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += LARA_HANG_VERTICAL_EPSILON;                        // inc for water_surf.z
                *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb->edge_hit)
                {
                    low_vertical_space = 1;
                }
                else
                {
                    low_vertical_space = 0;
                    offset[2] += ent->character->max_step_up_height + LARA_HANG_VERTICAL_EPSILON;
                   *climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                }

                if(climb->edge_hit && (climb->next_z_space >= ent->character->Height))// && (climb->edge_point.m_floats[2] - pos[2] < ent->character->max_step_up_height))   // max_step_up_height is not correct value here
                {
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_CLIMBING;
                    ent->character->no_fix = 0x01;
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateRotation(ent);
                    vec3_copy(climb->point, climb->edge_point.m_floats);
                }
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->speed.m_floats);
                cmd->rot[0] = 0.0;
                ent->character->no_fix = 0x01;
                if(low_vertical_space)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW, 0);
                    ent_out_of_water(ent);
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_STOP;
                    ent->onAnimChange = ent_out_of_water;
                }
            }
            else if(cmd->kill)
            {
                ent->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ent->next_state = TR_STATE_LARA_ONWATER_FORWARD;
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                ent->next_state = TR_STATE_LARA_ONWATER_BACK;
            }
            else if(cmd->move[1] ==-1)
            {
                if(cmd->shift == 1)
                {
                    ent->dir_flag = ENT_MOVE_LEFT;
                    cmd->rot[0] = 0.0;
                    ent->next_state = TR_STATE_LARA_ONWATER_LEFT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(cmd->move[1] == 1)
            {
                if(cmd->shift == 1)
                {
                    ent->dir_flag = ENT_MOVE_RIGHT;
                    cmd->rot[0] = 0.0;
                    ent->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                }
                else
                {
                    // rotate on water
                }
            }
            break;

        case TR_STATE_LARA_ONWATER_FORWARD:
            ent->move_type = MOVE_ON_WATER;
            if(cmd->kill)
            {
                ent->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->jump == 1)
            {
                t = pos[2];
                Character_GetHeightInfo(pos, &next_fc);
                Character_FixPenetrations(ent, cmd, NULL);
                pos[2] = t;
                ent->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
                ent->onAnimChange = ent_set_underwater;                         // dive
            }
            else if((cmd->move[0] == 1) && (cmd->action == 0))
            {
                if(!curr_fc->floor_hit || (pos[2] - ent->character->Height > curr_fc->floor_point.m_floats[2]))
                {
                    //ent->last_state = ent->last_state;                      // swim forward
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_WADE_FORWARD;
                    if(last_frame)                                              // to wade
                    {
                        pos[2] = curr_fc->floor_point.m_floats[2];
                        ent->move_type = MOVE_ON_FLOOR;
                    }
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_BACK:
            if(cmd->move[0] == -1 && cmd->jump == 0)
            {
                if(!curr_fc->floor_hit || (curr_fc->floor_point.m_floats[2] + ent->character->Height < curr_fc->water_level))
                {
                    //ent->current_state = TR_STATE_CURRENT;     // continue swimming
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_LEFT:
            cmd->rot[0] = 0.0;
            if(cmd->jump == 0)
            {
                if(cmd->move[1] ==-1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - ent->character->Height > curr_fc->floor_point.m_floats[2]))
                    {
                        // walk left
                        ent->next_state = TR_STATE_LARA_ONWATER_LEFT;
                    }
                    else
                    {
                        // walk left
                        ent->next_state = TR_STATE_LARA_WALK_LEFT;
                        if(last_frame)
                        {
                            pos[2] = curr_fc->floor_point.m_floats[2];
                            ent->move_type = MOVE_ON_FLOOR;
                        }
                    }
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

        case TR_STATE_LARA_ONWATER_RIGHT:
            cmd->rot[0] = 0.0;
            if(cmd->jump == 0)
            {
                if(cmd->move[1] == 1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - ent->character->Height > curr_fc->floor_point.m_floats[2]))
                    {
                        // swim RIGHT
                        ent->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                    }
                    else
                    {
                        // walk left
                        ent->next_state = TR_STATE_LARA_WALK_RIGHT;
                        if(last_frame)
                        {
                            pos[2] = curr_fc->floor_point.m_floats[2];
                            ent->move_type = MOVE_ON_FLOOR;
                        }
                    }
                }
                else
                {
                    ent->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ent->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

            /*
             * CROUCH SECTION
             */
        case TR_STATE_LARA_CROUCH_IDLE:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            move[0] = pos[0];
            move[1] = pos[1];
            move[2] = pos[2] + 0.5 * (ent->bf.bb_max[2] - ent->bf.bb_min[2]);
            Character_GetHeightInfo(move, &next_fc);

            Character_Lean(ent, cmd, 0.0);

            if((cmd->crouch == 0) && !low_vertical_space)
            {
                ent->next_state = TR_STATE_LARA_STOP;       // Back to stand
            }
            else if((cmd->move[0] != 0) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CRAWL_IDLE; // Both forward & back provoke crawl stage
            }
            else if(cmd->jump == 1)
            {
                ent->next_state = TR_STATE_LARA_CROUCH_ROLL;// Crouch roll
            }
            else if(cmd->action)
            {
                ent->next_state = TR_STATE_LARA_PICKUP;     // Pick up item 67
            }
            else
            {
                if(engine_world.version > TR_III)
                {
                    if(cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift == 0)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        ent->next_state = TR_STATE_LARA_CROUCH_TURN_RIGHT;
                    }
                    else if(cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift == 0)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        ent->next_state = TR_STATE_LARA_CROUCH_TURN_LEFT;
                    }
                }
                else
                {
                    cmd->rot[0] = 0.0;
                }
            }
            break;

        case TR_STATE_LARA_CROUCH_ROLL:
        case TR_STATE_LARA_SPRINT_ROLL:
            ent->character->complex_collision = 0x01;
            cmd->rot[0] = 0.0;
            Character_Lean(ent, cmd, 0.0);
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                ent->speed.m_floats[0] *= 0.5;
                ent->speed.m_floats[1] *= 0.5;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }

            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            if(cmd->horizontal_collide == 1) // Smash into wall
            {
                ent->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_CRAWL_IDLE:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(cmd->kill == 1)
            {
                ent->dir_flag = ENT_STAY;
                ent->next_state = TR_STATE_LARA_DEATH;
            }
            else if(cmd->move[1] == -1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_TURN_LEFT, 0);
            }
            else if(cmd->move[1] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_TURN_RIGHT, 0);
            }
            else if(cmd->move[0] == 1)
            {
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 4, CRAWL_FORWARD_OFFSET);
                offset[2] += 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2]);
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) &&
                   (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
                   (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
                {
                    ent->next_state = TR_STATE_LARA_CRAWL_FORWARD;  // In TR4+, first state is crawlspace jump
                }
            }
            else if(cmd->move[0] == -1)
            {
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
                offset[2] += 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2]);
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) &&
                   (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
                   (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    ent->next_state = TR_STATE_LARA_CRAWL_BACK;
                }
                else if(cmd->action && (cmd->horizontal_collide == 0) &&
                   (next_fc.floor_point.m_floats[2] < pos[2] - ent->character->Height))
                {
                    btScalar temp[3];
                    vec3_copy(temp, pos);                                       // save entity position
                    pos[0] = next_fc.floor_point.m_floats[0];
                    pos[1] = next_fc.floor_point.m_floats[1];
                    vec3_mul_scalar(offset, ent->transform + 4, 0.5 * CRAWL_FORWARD_OFFSET);
                    offset[2] += 128.0;
                    curr_fc->floor_hit = next_fc.floor_hit;
                    curr_fc->floor_point = next_fc.floor_point;
                    curr_fc->floor_normale = next_fc.floor_normale;
                    curr_fc->floor_obj = next_fc.floor_obj;
                    curr_fc->ceiling_hit = next_fc.ceiling_hit;
                    curr_fc->ceiling_point = next_fc.ceiling_point;
                    curr_fc->ceiling_normale = next_fc.ceiling_normale;
                    curr_fc->ceiling_obj = next_fc.ceiling_obj;

                    *climb = Character_CheckClimbability(ent, offset, &next_fc, 1.5 * ent->bf.bb_max[2]);
                    vec3_copy(pos, temp);                                       // restore entity position
                    if(climb->can_hang)
                    {
                        ent->angles[0] = climb->edge_z_ang;
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ent->move_type = MOVE_CLIMBING;
                        vec3_copy(climb->point, climb->edge_point.m_floats);
                        ent->next_state = TR_STATE_LARA_CRAWL_TO_CLIMB;
                    }
                }
            }
            else if(!cmd->crouch)
            {
                ent->next_state = TR_STATE_LARA_CROUCH_IDLE;      // Back to crouch.
            }
            break;

        case TR_STATE_LARA_CRAWL_TO_CLIMB:
            ent->character->no_fix = 0x01;
            if(last_frame)     // to the climb
            {
                if(!cmd->action)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    ent->move_type = MOVE_FREE_FALLING;
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_HANG_IDLE, -1);
                }
                ent->onAnimChange = ent_to_edge_climb;
                ent->character->no_fix = 0x00;
            }
            break;

        case TR_STATE_LARA_CRAWL_FORWARD:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            vec3_mul_scalar(offset, ent->transform + 4, CRAWL_FORWARD_OFFSET);
            offset[2] += 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2]);
            vec3_add(offset, offset, pos);
            Character_GetHeightInfo(offset, &next_fc);

            if((cmd->move[0] != 1) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (cmd->horizontal_collide != 0)                                                   ||
                     (next_fc.floor_point.m_floats[2] >= pos[2] + ent->character->min_step_up_height) ||
                     (next_fc.floor_point.m_floats[2] <= pos[2] - ent->character->min_step_up_height)  )
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_BACK:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_BACKWARD;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            vec3_mul_scalar(offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
            offset[2] += 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2]);
            vec3_add(offset, offset, pos);
            Character_GetHeightInfo(offset, &next_fc);
            if((cmd->move[0] != -1) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (cmd->horizontal_collide != 0)                                                     ||
                     (next_fc.floor_point.m_floats[2] >= pos[2] + ent->character->min_step_up_height)   ||
                     (next_fc.floor_point.m_floats[2] <= pos[2] - ent->character->min_step_up_height)    )
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            else
            {
                if(ent->current_animation == TR_ANIMATION_LARA_CRAWL_BACKWARD)
                {
                    ent->current_speed = 16.0;      ///@FIXME: magick!
                }
                else
                {
                    ent->current_speed = 6.0;
                }
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_LEFT:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] *= ((ent->current_frame > 3) && (ent->current_frame < 14))?(1.0):(0.0);

            if((cmd->move[1] != -1) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_RIGHT:
            ent->character->complex_collision = 0x01;
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] *= ((ent->current_frame > 3) && (ent->current_frame < 14))?(1.0):(0.0);

            if((cmd->move[1] != 1) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CROUCH_TURN_LEFT:
        case TR_STATE_LARA_CROUCH_TURN_RIGHT:
            ent->character->complex_collision = 0x01;
            cmd->rot[0] *= ((ent->current_frame > 3) && (ent->current_frame < 23))?(0.6):(0.0);

            if((cmd->move[1] == 0) || (cmd->kill == 1))
            {
                ent->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            break;

            /*
             * CLIMB MONKEY
             */
        case TR_STATE_LARA_MONKEYSWING_IDLE:
            ent->dir_flag = ENT_STAY;
            if((ent->move_type != MOVE_CEILING_CLMB) || (!cmd->action))
            {
                ent->next_state = TR_STATE_LARA_GRAB_TO_FALL;
                if(last_frame)
                {
                    ent->move_type = MOVE_FREE_FALLING;
                }
            }
            else if(cmd->shift && (cmd->move[1] ==-1))
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_LEFT;
            }
            else if(cmd->shift && (cmd->move[1] == 1))
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_RIGHT;
            }
            else if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ent->next_state = TR_STATE_LARA_MONKEYSWING_FORWARD;
            }
            else if(cmd->move[1] ==-1)
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_TURN_LEFT;
            }
            else if(cmd->move[1] == 1)
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_TURN_RIGHT;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_LEFT:
            cmd->rot[0] *= 0.5;
            if((ent->move_type != MOVE_CEILING_CLMB) || (!cmd->action))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != -1)
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_RIGHT:
            cmd->rot[0] *= 0.5;
            if((ent->move_type != MOVE_CEILING_CLMB) || (!cmd->action))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != 1)
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_FORWARD:
            cmd->rot[0] *= 0.45;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if((ent->move_type != MOVE_CEILING_CLMB) || (!cmd->action))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ent->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_FALL_FORWARD:
            ent->character->complex_collision = 0x01;
            if(ent->speed.m_floats[3] < -FREE_FALL_SPEED_2)
            {
                ent->current_speed *= 0.5;
            }
            break;

            /*
             * intermediate animations are processed automatically.
             */
        default:
            cmd->rot[0] = 0.0;
            //ent->character->complex_collision = 0x01;
            if((ent->move_type == MOVE_CEILING_CLMB) || (ent->move_type == MOVE_WALLS_CLMB))
            {
                if(cmd->action == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_FREE_FALLING;
                }
            }
            break;
    };

    return 0;
}

/*
 * 187: wall climb up 2 hand
 * 188: wall climb down 2 hand
 * 189: stand mr. Poher
 * 194: 4 point climb to 2 hand climb
 * 201, 202 - right-left wall climb 4 point to 2 point
 * 395 4 point climb down to 2 hand climb // 2 ID
 * 421 - crawl to jump forward-down
 */
