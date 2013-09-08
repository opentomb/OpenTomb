
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

/* 65 - WADE
 * 71 - crouch
 * 73 - sprint
 * id           next_anim
 * ROTATE_CONT
 * 0            20
 * 1            6
 * 2            11
 * 20           44L - 69R
 * 65           177 - TR_ANIMATION_LARA_WADE
 * ANIM 65 // strafe
 * 2            66
 * 48           143
 * ANIM 67 // strafe
 * 2            66
 * 49           144
 *
 * ANIM 70      TR_ANIMATION_LARA_SLIDE_FORWARD
 * 1            246 - TR_ANIMATION_LARA_SLIDE_FORWARD_STEEP_STOP
 * 2            71  - TR_ANIMATION_LARA_SLIDE_FORWARD_END
 * 3            76  - TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN
 * 28           91  - TR_ANIMATION_LARA_TRY_HANG_VERTICAL_BEGIN
 *
 * ANIM 77      TR_ANIMATION_LARA_CONTINUE_FLY_FORWARD
 * 1            92  - TR_ANIMATION_LARA_LANDING_FROM_RUN                        ok
 * 2            82  - TR_ANIMATION_LARA_LANDING_MIDDLE                          ok
 * 9            45  - TR_ANIMATION_LARA_UNKNOWN3                                no
 * 11(x4)       94, 100, 248, 249 (diff. int-s)                                 no
 * 52           158 - TR_ANIMATION_LARA_UNKNOWN4                                no
 * 68           210 - TR_ANIMATION_LARA_STANDING_JUMP_ROLL_BEGIN                ok
 *
 * ANIM 96      TR_ANIMATION_LARA_HANG_IDLE
 * 19           97  - TR_ANIMATION_LARA_CLIMB_ON
 * 30           136 - TR_ANIMATION_LARA_CLIMB_LEFT
 * 31           137 - TR_ANIMATION_LARA_CLIMB_RIGHT
 * 54           159 - TR_ANIMATION_LARA_CLIMB_ON2
 * 56           172 - TR_ANIMATION_LARA_LADDER_HANG
 * 87           287 - TR_ANIMATION_LARA_HANG_TO_CROUCH_BEGIN
 * 107          355 - TR_ANIMATION_LARA_HANG_AROUND_LEFT_OUTER_BEGIN
 * 108          357 - TR_ANIMATION_LARA_HANG_AROUND_RIGHT_OUTER_BEGIN
 * 109          359 - TR_ANIMATION_LARA_HANG_AROUND_LEFT_INNER_BEGIN
 * 110          361 - TR_ANIMATION_LARA_HANG_AROUND_RIGHT_INNER_BEGIN
 *
 * SWIMMING!!!
 * ANIM 86
 * 44           124
 * 18           87, 198, 199, 200
 * ANIM 87
 * 13           107
 * 17           86
 * 44           124
 * ANIM 108
 * 17           109
 * 39           130
 * 40           129
 * 44           124
 * ANIM 110
 * 2            111
 * 34           118
 * 44           132
 * 47           140
 * 48           143
 * 49           144
 * 55           190
 * ANIM 116
 * 17           115
 * 33           117
 * 44           132
 * 65           177
 * ANIM 141
 * 33           142
 * ANIM 143 //strafe!
 * 22           65
 * 33           110, 143
 * ANIM 144 //strafe!
 * 21           67
 * 33           110, 144
 * ANIM 150 //UBER MANY STATES
 * ANIM 158
 * 2            24, 151
 * 35           152
 * 53           153
 *
 * ANIM 177 WATER WALK --- WADE
 * 1            180, 181
 * 2            184, 185
 */

#define FREE_FALL_SPEED_1 (3600)
#define FREE_FALL_SPEED_2 (4500)

#define PENETRATION_TEST_OFFSET (0.16 * ent->character->Radius)
#define WALK_FORWARD_OFFSET (96.0)              ///@FIXME: find real offset
#define WALK_FORWARD_STEP_UP (256.0)            // by bone frame bb
#define RUN_FORWARD_OFFSET (160.0)              ///@FIXME: find real offset
#define RUN_FORWARD_STEP_UP (320.0)             // by bone frame bb
#define CRAWL_FORWARD_OFFSET (256.0)
#define LARA_HANG_WALL_DISTANCE (128.0 - 24.0)
#define LARA_HANG_VERTICAL_EPSILON (16.0)
#define LARA_HANG_VERTICAL_OFFSET (12.0)        // in original is 0, in real life hands are little more higher than edhe
#define LARA_TRY_HANG_WALL_OFFSET (48.0)        // It works more stable than 32 or 128
#define LARA_HANG_SENSOR_Z (800.0)              // It works more stable than 1024 (after collision critical fix, of course)

#define OSCILLATE_HANG_USE 0

/**
 * Current animation == current state
 * works correct in tr1 - tr2
 * tr3 and next - jump like fish has other state change number
 * swim strafe has other state change number
 */
int State_Control_Lara(struct entity_s *ent, struct character_command_s *cmd)
{
    int i;
    btScalar t, *pos = ent->transform + 12;
    btScalar offset[3], move[3];
    height_info_t next_fc, *curr_fc;
    climb_info_t climb;
    
    curr_fc = &ent->character->height_info;
    next_fc.cb = ent->character->ray_cb;
    next_fc.cb->m_closestHitFraction = 1.0;
    next_fc.cb->m_collisionObject = NULL;
    next_fc.ccb = ent->character->convex_cb;
    next_fc.ccb->m_closestHitFraction = 1.0;
    next_fc.ccb->m_hitCollisionObject = NULL;
    ent->anim_flags = ANIM_NORMAL_CONTROL;
    Character_UpdateCurrentHeight(ent);
/*
 * - On floor animations
 * - Climbing animations
 * - Landing animations
 * - Free fall animations
 * - Water animations
 */

    switch(ent->current_animation)
    {
        /*
         * Base onfloor animations
         */
        case TR_ANIMATION_LARA_STAY_SOLID:
        case TR_ANIMATION_LARA_STAY_IDLE:
/* ID           anim
 * 99           326     // TR_ANIMATION_LARA_STAY_TO_POLE_GRAB
 * 98           325     // TR_ANIMATION_LARA_HOLE_GRAB
 * 90           419     // TR_ANIMATION_LARA_CROWBAR_USE_ON_WALL
 * 71           217     // TR_ANIMATION_LARA_tr2_ZIPLINE_FALL_tr345_STAND_TO_CROUCH
 * 65           177     // TR_ANIMATION_LARA_WADE
 * 56           160     // TR_ANIMATION_LARA_STAND_TO_LADDER
 * 43           134     // TR_ANIMATION_LARA_USE_PUZZLE
 * 41           64      // TR_ANIMATION_LARA_PULL_SWITCH_UP
 * 40           63      // TR_ANIMATION_LARA_PULL_SWITCH_DOWN
 * 39           135     // TR_ANIMATION_LARA_PICKUP
 * 38           120     // TR_ANIMATION_LARA_START_OBJECT_MOVING
 * 28           26      // TR_ANIMATION_LARA_STAY_TO_GRAB*/
        case TR_ANIMATION_LARA_CROUCH_TO_STAND:
        case TR_ANIMATION_LARA_LANDING_MIDDLE:
            ent->dir_flag = ENT_STAY;
            cmd->rot[0] = 0;
            if(cmd->action)
            {
                t = ent->character->Radius + LARA_TRY_HANG_WALL_OFFSET;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += 1024.0;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
            }
            if((climb.climb_flag == CLIMB_ABSENT) && (ent->move_type == MOVE_CLIMBING))
            {
                ent->move_type = MOVE_ON_FLOOR;
            }

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->kill)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT && cmd->jump == 0)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK && cmd->jump == 0)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if(cmd->roll == 1)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
            }
            else if(cmd->crouch == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_IDLE);
            }
            else if(curr_fc->water && curr_fc->floor_hit && (cmd->move[0] == 1) && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->max_step_up_height))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WADE_FORWARD);
            }
            else if((ent->move_type == MOVE_CLIMBING) || ((cmd->move[0] == 1) && (climb.climb_flag == CLIMB_FULL_HEIGHT) && (cmd->action) &&
                    (pos[2] + ent->character->max_step_up_height < next_fc.floor_point[2]) && (pos[2] + 2048.0 >= next_fc.floor_point[2]) && (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component)))  // trying to climb on
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                if(pos[2] + 640.0 >= next_fc.floor_point[2])
                {
                    ent->angles[0] = next_fc.edge_z_ang;
                    ent->move_type = MOVE_CLIMBING;
                    pos[2] = next_fc.floor_point[2] - 512.0;
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                }
                else if(pos[2] + 1024.0 >= next_fc.floor_point[2])
                {
                    ent->angles[0] = next_fc.edge_z_ang;
                    ent->move_type = MOVE_CLIMBING;
                    pos[2] = next_fc.floor_point[2] - 768.0;
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                }
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->shift == 1)     // walk forward
            {
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
                offset[2] += 512.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) && (next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height)))
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_FORWARD);
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->shift == 0)     // run forward
            {
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] ==-1 && cmd->jump == 0 && cmd->shift == 0)     // run back
            {
                vec3_mul_scalar(move, ent->transform + 4, - PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide == 0)
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_BACK);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift == 0)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TURN_RIGHT_SLOW);
            }
            else if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift == 0)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TURN_LEFT_SLOW);
            }
            else if((cmd->move[0] != 0 || cmd->move[1] != 0 || cmd->slide) && cmd->jump == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_PREPARE);       // jump sideways
            }
            else if(cmd->move[0] ==-1 && cmd->move[1] == 0 && cmd->jump == 0 && cmd->shift == 1)
            {
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 4, -WALK_FORWARD_OFFSET);
                offset[2] += 512.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) && (next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height)))
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_BACK);
                }
                else
                {
                    ent->dir_flag  = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift == 1)
            {
                vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 0, RUN_FORWARD_OFFSET);
                offset[2] += 512.0;
                i = Character_CheckNextStep(ent, offset, &next_fc);
                if((cmd->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                {
                    cmd->rot[0] = 0.0;
                    ent->dir_flag = ENT_MOVE_RIGHT;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_RIGHT);
                }
                else
                {
                    ent->dir_flag  = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift == 1)
            {
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 0, -RUN_FORWARD_OFFSET);
                offset[2] += 512.0;
                i = Character_CheckNextStep(ent, offset, &next_fc);
                if((cmd->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                {
                    cmd->rot[0] = 0.0;
                    ent->dir_flag = ENT_MOVE_LEFT;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_LEFT);
                }
                else
                {
                    ent->dir_flag  = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue standing still
                }
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 0 && cmd->jump == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_PREPARE);       // jump up
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue standing still
            }
            break;

        case TR_ANIMATION_LARA_STAY_JUMP_SIDES:
            cmd->rot[0] = 0;                                                    // Slide checking is only for jumps direction correction!
            if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                cmd->move[0] = -1;
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                cmd->move[0] = 1;
            }

            if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    goto vertical;
                }
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_FORWARD))       // jump forward
                {
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    goto vertical;
                }
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_BACK))               // jump backward
                {
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else if(cmd->move[1] == 1)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
                vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    goto vertical;
                }
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_LEFT))               // jump right
                {
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else if(cmd->move[1] ==-1)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                if(cmd->horizontal_collide != 0)
                {
                    goto vertical;
                }
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_RIGHT))               // jump left
                {
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else
            {
                vertical:
                ent->dir_flag = ENT_MOVE_FORWARD;
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT))               // jump vertical
                {
                    Character_UpdateCurrentSpeed(ent, 0);
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            break;

        case TR_ANIMATION_LARA_JUMP_BACK:
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // landing
            }
            else if(cmd->horizontal_collide)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_FORWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else if(cmd->roll)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_ROLL);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_JUMP_LEFT:
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // landing
            }
            else if(cmd->horizontal_collide)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_RIGHT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_JUMP_RIGHT:
            cmd->rot[0] = 0.0;
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // landing
            }
            else if(cmd->horizontal_collide)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_LEFT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_RUN_BACK_BEGIN:
        case TR_ANIMATION_LARA_RUN_BACK:
        case TR_ANIMATION_LARA_RUN_BACK_END:
            ent->dir_flag = ENT_MOVE_BACKWARD;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                if(cmd->action)                                                 // try to climb
                {//fff;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_SOLID, 0);
                    t = LARA_TRY_HANG_WALL_OFFSET + ent->bf.bb_max[1];
                    vec3_mul_scalar(offset, ent->transform + 4, t);
                    offset[2] += ent->character->max_step_up_height;
                    climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);

                    if(climb.climb_flag >= CLIMB_HANG_ONLY)
                    {
                        ent->character->speed.m_floats[0] = 0.0;
                        ent->character->speed.m_floats[1] = 0.0;
                        ent->current_speed = 0.0;
                    }
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            break;

        case TR_ANIMATION_LARA_TURN_RIGHT_SLOW:
            cmd->rot[0] *= 0.5;
            ent->dir_flag = ENT_STAY;
            if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->action == 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->action == 0 && cmd->crouch == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TURN_FAST);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_TURN_LEFT_SLOW:
            cmd->rot[0] *= 0.5;
            ent->dir_flag = ENT_STAY;
            if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->action == 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->action == 0 && cmd->crouch == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TURN_FAST);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_ROTATE_RIGHT:
        case TR_ANIMATION_LARA_ROTATE_LEFT:
            // 65 - wade
            ent->dir_flag = ENT_STAY;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 0 && cmd->move[1] != 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // continue rotating
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

            /*
             * RUN AND WALK animations section
             */
        case TR_ANIMATION_LARA_RUN:
        case TR_ANIMATION_LARA_STAY_TO_RUN:
            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
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
            else if((next_fc.floor_point[2] > pos[2]) && (next_fc.floor_normale.m_floats[2] < ent->character->critical_slant_z_component))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if((cmd->crouch == 0) && (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                i = Entity_GetAnimDispatchCase(ent, 2);                         // MOST CORRECT STATECHANGE!!!
                if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] += RUN_FORWARD_STEP_UP;
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] += RUN_FORWARD_STEP_UP;
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
                offset[2] += 1024.0;
                i = Entity_GetAnimDispatchCase(ent, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                }
                else if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue running
                }
                Character_UpdateCurrentSpeed(ent, 0);
            }
            else if(cmd->move[0] == 1)                                          // If we continue running...
            {
                if(cmd->crouch == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_IDLE);
                }
                else if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->max_step_up_height))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WADE_FORWARD);
                }
                else if(cmd->shift == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WALK_FORWARD);
                }
                else if(cmd->jump == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_FORWARD);
                }
                else if(cmd->roll == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->sprint == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SPRINT);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_RUN_TO_SPRINT_LEFT:
        case TR_ANIMATION_LARA_RUN_TO_SPRINT_RIGHT:
        case TR_ANIMATION_LARA_SPRINT:

            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);// Normal run then die
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
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);// Interrupt sprint
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
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
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
            }
            else
            {
                if(cmd->jump == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SPRINT_ROLL);
                }
                else if(cmd->crouch == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_IDLE);
                }
                else if(cmd->move[0] == 0)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                }
            }
            break;

        case TR_ANIMATION_LARA_WALK_FORWARD:
            cmd->rot[0] *= 0.4;
            vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                /*
                 * climb up
                 */
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                i = Entity_GetAnimDispatchCase(ent, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] += WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] += WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                /*
                 * climb down
                 */
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                i = Entity_GetAnimDispatchCase(ent, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] -= WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] -= WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((cmd->horizontal_collide & 0x01) || (i < CHARACTER_STEP_DOWN_BIG || i > CHARACTER_STEP_UP_BIG))
            {
                /*
                 * too high
                 */
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            else if(curr_fc->water && curr_fc->floor_hit && (curr_fc->water_level - curr_fc->floor_point.m_floats[2] > ent->character->max_step_up_height))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WADE_FORWARD);
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue walking
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_WALK_FORWARD_BEGIN:
            cmd->rot[0] *= 0.4;
            vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                vec3_copy(cmd->climb_pos, next_fc.floor_point);
                pos[2] += WALK_FORWARD_STEP_UP;                                 ///@FIXME: may be it is anim command!
                ent->move_type = MOVE_CLIMBING;
            }
            else if((next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                vec3_copy(cmd->climb_pos, next_fc.floor_point);
                pos[2] -= WALK_FORWARD_STEP_UP;                                 ///@FIXME: may be it is anim command!
                ent->move_type = MOVE_CLIMBING;
            }
            else if((cmd->horizontal_collide & 0x01) || (cmd->move[0] != 1) ||
                    (i < CHARACTER_STEP_DOWN_BIG) || (i > CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       // stop
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            break;

        case TR_ANIMATION_LARA_START_WALK_BACK:
            cmd->rot[0] *= 0.4;
            vec3_mul_scalar(offset, ent->transform + 4, -WALK_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if((ent->move_type == MOVE_CLIMBING) || (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                ent->move_type = MOVE_CLIMBING;

                Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, 0);
                vec3_copy(cmd->climb_pos, next_fc.floor_point);
                pos[2] -= WALK_FORWARD_STEP_UP;
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue walking
            }
           break;

        case TR_ANIMATION_LARA_WALK_BACK:
            vec3_mul_scalar(offset, ent->transform + 4, -WALK_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if((ent->move_type == MOVE_CLIMBING) || (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {                                                                   // works correct
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_CLIMBING;
                i = Entity_GetAnimDispatchCase(ent, TR_STATE_LARA_STOP);
                if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] -= WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                }
                else if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] -= WALK_FORWARD_STEP_UP;                             ///@FIXME: may be it is anim command!
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                }
            }
            else if(cmd->move[0] ==-1 && cmd->crouch == 0 && cmd->shift == 1)
            {
                if(next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// walk backward
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_Frame(ent, engine_frame_time, 0);
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == TR_STATE_LARA_WALK_FORWARD)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_WALK_LEFT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift)
            {
                vec3_mul_scalar(offset, ent->transform + 0, -RUN_FORWARD_OFFSET);  // not an error - RUN_... more correct here
                offset[2] += 512.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue walking
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        case TR_ANIMATION_LARA_WALK_RIGHT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift)
            {
                vec3_mul_scalar(offset, ent->transform + 0, RUN_FORWARD_OFFSET);  // not an error - RUN_... more correct here
                offset[2] += 512.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue walking
                }
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            break;

        /*case TR_ANIMATION_LARA_STAY_TO_RUN:
            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            offset[2] += 512.0;
            vec3_add(offset, offset, pos);
            Character_GetHeightInfo(offset, &next_fc);
            climb = (next_fc.floor_hit && (!next_fc.ceiling_hit || next_fc.ceiling_point[2] - next_fc.floor_point[2] >= ent->character->Height))?0x03:0x00;
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if((climb & 0x03) && (next_fc.floor_point[2] > pos[2]) && next_fc.floor_normale.m_floats[2] < ent->character->critical_slant_z_component)
            {
                ent->current_speed = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if((cmd->move[0] == 1) && (climb & 0x02) && (next_fc.floor_normale[2] >= ent->character->critical_slant_z_component) &&
                    (next_fc.floor_point.m_floats[2] > pos[2] + ent->character->min_step_up_height) && (next_fc.floor_point.m_floats[2] <= pos[2] + ent->character->max_step_up_height))  // trying to climb
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                i = Entity_GetAnimDispatchCase(ent, 2);                          ///@XXX: xxx
                if(i >= 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    vec3_copy(cmd->climb_pos, next_fc.floor_point);
                    pos[2] += RUN_FORWARD_STEP_UP;                              ///@FIXME: may be it is anim command!
                    ent->move_type = MOVE_CLIMBING;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 0)   // start run
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // walk next
            }
            else if(cmd->roll == 1)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);        // fast roll
            }
            else if(cmd->crouch == 1)
            {
                if(CVAR_get_val_d("engine_version") > 4)
                {
                    i = Entity_GetAnimDispatchCase(ent, 2);
                    if(i == 0)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_TO_CROUCH_LEFT_BEGIN, 0);
                    }
                    else if(i == 1)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_TO_CROUCH_RIGHT_BEGIN, 0);
                    }
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, 2);                        // stop
            }
            break;
*/
            /*
             * Run and walk stop animations
             */
        case TR_ANIMATION_LARA_RUN_TO_STAY_RIGHT:                               // run to stay! kep
        case TR_ANIMATION_LARA_RUN_TO_STAY_LEFT:
            vec3_mul_scalar(offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->kill)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if((i > CHARACTER_STEP_HORIZONTAL) && (next_fc.floor_normale.m_floats[2] < ent->character->critical_slant_z_component))
            {
                ent->current_speed = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if(i > CHARACTER_STEP_UP_LITTLE)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                ent->dir_flag = ENT_STAY;
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue standing still
            }
            break;

        case TR_ANIMATION_LARA_END_WALK_RIGHT:                                  // walk to stay! kep
        case TR_ANIMATION_LARA_END_WALK_LEFT:
            vec3_mul_scalar(offset, ent->transform + 4, WALK_FORWARD_OFFSET);
            offset[2] += 512.0;
            i = Character_CheckNextStep(ent, offset, &next_fc);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if((i > CHARACTER_STEP_UP_LITTLE) || (next_fc.floor_normale.m_floats[2] < ent->character->critical_slant_z_component))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
                ent->dir_flag = ENT_STAY;
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // останавливаться дальше
            }
            break;

            /*
             * Climb steps
             */
        case TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT:
        case TR_ANIMATION_LARA_RUN_UP_STEP_LEFT:
        case TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT:
        case TR_ANIMATION_LARA_WALK_UP_STEP_LEFT:
        case TR_ANIMATION_LARA_WALK_DOWN_LEFT:
        case TR_ANIMATION_LARA_WALK_DOWN_RIGHT:
            //ent->character->no_fix = 0x01;
            //Entity_Frame(ent, engine_frame_time, 0)                           // not needeed - prevents glitches
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))                   // climb on
            {
                ent->move_type = MOVE_ON_FLOOR;
                /*if(cmd->move[0] != 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_SOLID, 0);
                }*/
            }
            break;

        case TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT:
        case TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT:
            ent->character->no_fix = 1;
            ent->dir_flag = ENT_MOVE_BACKWARD;
            if(cmd->move[0] != -1)
            {
                ent->dir_flag = ENT_STAY;
                if(2 <= Entity_Frame(ent, engine_frame_time, 2))
                {
                    ent->move_type = MOVE_ON_FLOOR;
                }
            }
            else if(2 <= Entity_Frame(ent, engine_frame_time, -1))              // climb down
            {
                ent->move_type = MOVE_ON_FLOOR;
            }

            break;

        case TR_ANIMATION_LARA_CLIMB_2CLICK_END:
            if(cmd->move[0] == 1 && cmd->shift == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // stop
            }
            break;

            /*
             * Slide animations section
             */
        case TR_ANIMATION_LARA_START_SLIDE_BACKWARD:
             // 28           91  - TR_ANIMATION_LARA_TRY_HANG_VERTICAL_BEGIN
        case TR_ANIMATION_LARA_SLIDE_BACKWARD:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                if(cmd->action)                                                 // try to climb
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_SOLID, 0);
                    t = LARA_TRY_HANG_WALL_OFFSET + ent->bf.bb_max[1];
                    vec3_mul_scalar(offset, ent->transform + 4, t);
                    offset[2] += ent->character->max_step_up_height;
                    t = ent->character->max_step_up_height;
                    ent->character->max_step_up_height *= -1.0;
                    climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                    ent->character->max_step_up_height = t;

                    if(climb.climb_flag >= CLIMB_HANG_ONLY)
                    {
                        ent->character->speed.m_floats[0] = 0.0;
                        ent->character->speed.m_floats[1] = 0.0;
                        ent->current_speed = 0.0;
                    }
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                }
            }
            else if(cmd->slide == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            else if(cmd->slide != 0 && cmd->jump == 1)
            {
                if(2 == Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_BACK))
                {
                    cmd->slide = 0x00;
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // continue sliding
            }
            break;

        case TR_ANIMATION_LARA_SLIDE_FORWARD:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(cmd->slide == 0)
            {
                if(cmd->move[0] == 1 && (CVAR_get_val_d("engine_version") >= TR_III))
                {
                     Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                }
                else
                {
                     Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);  // stop
                }
            }
            else if(cmd->slide != 0 && cmd->jump == 1)
            {
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_FORWARD))       // jump
                {
                    Character_SetToJump(ent, cmd, 3000.0);
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // continue sliding
            }
            break;

        case TR_ANIMATION_LARA_SLIDE_FORWARD_END:
            ent->dir_flag = ENT_MOVE_FORWARD;
            Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            Character_UpdateCurrentSpeed(ent, 0);
            break;

        case TR_ANIMATION_LARA_SLIDE_BACKWARD_END:
            ent->dir_flag = ENT_MOVE_BACKWARD;
            Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            Character_UpdateCurrentSpeed(ent, 0);
            break;

            /*
             * Misk animations
             */
        case TR_ANIMATION_LARA_START_OBJECT_MOVING:
            /*if(cmd->action == 1 && cmd->move[0] == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // hold block
            }
            else if(cmd->action == 1 && cmd->move[0] == 1 && 1)                 // @FIXME
            {
                Entity_Frame(ent, engine_frame_time, 1);                        // push block
            }
            else if(cmd->action == 1 && cmd->move[0] ==-1 && 1)                 // @FIXME
            {
                Entity_Frame(ent, engine_frame_time, 2);                        // pop block
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, 0);                        // release block
            }*/
            break;

        case TR_ANIMATION_LARA_STANDING_JUMP_ROLL_BEGIN:
        case TR_ANIMATION_LARA_RUNNING_JUMP_ROLL_BEGIN:
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))                   // continue roll
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                ent->angles[0] += 180.0;
            }
            break;

        case TR_ANIMATION_LARA_BACKWARDS_JUMP_ROLL_BEGIN:
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))                   // continue roll
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ent->angles[0] += 180.0;
            }
            break;

        case TR_ANIMATION_LARA_ROLL_BEGIN:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0); ///@FIXME: check
            }
            else
            {
                if(2 <= Entity_Frame(ent, engine_frame_time, -1))               // continue roll
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    ent->angles[0] += 180.0;
                }
            }
            break;

        case TR_ANIMATION_LARA_ROLL_END:
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_VERTICAL, 0);
            }
            else if(cmd->roll == 1)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            break;

        /*
         * Climbing section
         */
        case TR_ANIMATION_LARA_TRY_HANG_VERTICAL:
            /* 94           318     // TR_ANIMATION_LARA_CEILING_TRAPDOOR_OPEN
             * 75           233     // TR_ANIMATION_LARA_MONKEY_GRAB
             * 10           29      // TR_ANIMATION_LARA_BEGIN_HANGING_VERTICAL
             * 9            30      // TR_ANIMATION_LARA_STOP_HANG_VERTICAL
             * 2            31      // TR_ANIMATION_LARA_LANDING_LIGHT*/
        case TR_ANIMATION_LARA_STOP_HANG_VERTICAL:
            cmd->rot[0] = 0.0;
            if((ent->move_type != MOVE_CLIMBING) && (cmd->action == 1))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * ent->character->speed.m_floats[2];
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                }
            }

            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if((cmd->action == 1) && (ent->move_type == MOVE_CLIMBING))
            {
                if(2 <= Entity_Frame(ent, engine_frame_time, 10))
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_HANG_IDLE, ent->model->animations[TR_ANIMATION_LARA_HANG_IDLE].frames_count - 1);
                    pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = cmd->climb_pos[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->character->speed.m_floats);
                }
            }
            else if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // landing immidiatly
            }
            else
            {
                if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_1)      // next free fall stage
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;
                }
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_TRY_HANG_SOLID:
            // 75           150  - TR_ANIMATION_LARA_OSCILLATE_HANG_ON
        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG:
            cmd->rot[0] = 0.0;
            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                break;
            }

            if((ent->move_type != MOVE_CLIMBING) && (cmd->action == 1))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + ent->bf.bb_max[1];              // here we need bbox coordinates... maybe...
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                }
            }

            if((cmd->action == 1) && (ent->move_type == MOVE_CLIMBING))
            {
                pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                pos[2] = cmd->climb_pos[2] -  ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                vec3_set_zero(ent->character->speed.m_floats);
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG))
                {
#if OSCILLATE_HANG_USE
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                    Character_CheckNextPenetration(ent, cmd, move);
                    ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                    if(!cmd->horizontal_collide)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    }
#endif
                }
            }
            else if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // middle landing
            }
            else if((cmd->action == 1) && (ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_2))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // continue trying to hang
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_TRY_HANG_VERTICAL_BEGIN:
            cmd->rot[0] = 0;
            Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            Character_UpdateCurrentSpeed(ent, 0);
            break;

        case TR_ANIMATION_LARA_CLIMB_ON2:
        case TR_ANIMATION_LARA_CLIMB_ON:
        case TR_ANIMATION_LARA_LADDER_TO_CROUCH:
        case TR_ANIMATION_LARA_HANG_TO_CROUCH_END:
            cmd->rot[0] = 0;
            if(2 == Entity_Frame(ent, engine_frame_time, -1))
            {
                ent->move_type = MOVE_ON_FLOOR;
                vec3_copy(pos, cmd->climb_pos);
            }
            break;

        case TR_ANIMATION_LARA_OSCILLATE_HANG_ON:                               ///@FIXME: does not works correct in TR3+ versions - ceiling climb IDLE
            cmd->rot[0] = 0;
            if(cmd->action == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT); 
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            break;
            
        case TR_ANIMATION_LARA_HANG_IDLE:
            cmd->rot[0] = 0.0;
            climb.climb_flag = 0x00;
            if(cmd->action == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 512.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                }
            }

            if(cmd->action == 1 && cmd->move[0] == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                if((climb.climb_flag == CLIMB_ALT_HEIGHT) || ((cmd->crouch == 1) && (climb.climb_flag == CLIMB_FULL_HEIGHT)))
                {
                    vec3_copy(cmd->climb_pos, next_fc.floor_point.m_floats);
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CLIMB_TO_CRAWL);               // crawlspace climb
                }
                else if(climb.climb_flag == CLIMB_FULL_HEIGHT)
                {
                    vec3_copy(cmd->climb_pos, next_fc.floor_point.m_floats);
                    Entity_Frame(ent, engine_frame_time, (cmd->shift)?(TR_STATE_LARA_HANDSTAND):(TR_STATE_LARA_GRABBING));               // climb up
                }
                else
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT); 
                }
            }
            else if(cmd->action == 1 && cmd->move[1] ==-1)
            {
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                Character_CheckNextPenetration(ent, cmd, move);
                ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                if(cmd->horizontal_collide == 0)
                {
                    ent->move_type = ENT_MOVE_LEFT;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_LEFT, 0);  // edge climb left
                }
                else
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT); 
                }
            }
            else if(cmd->action == 1 && cmd->move[1] == 1)
            {
                vec3_mul_scalar(move, ent->transform + 0, 32.0);
                ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                Character_CheckNextPenetration(ent, cmd, move);
                ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                if(cmd->horizontal_collide == 0)
                {
                    ent->dir_flag = ENT_MOVE_RIGHT;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_RIGHT, 0); // edge climb right
                }
                else
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;                     // disable shake
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT); 
                }
            }
            else if((cmd->action == 1) && (ent->move_type == MOVE_CLIMBING))
            {
                ent->anim_flags = ANIM_LOOP_LAST_FRAME;                         // disable shake
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // hang IDLE
                pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                pos[2] = cmd->climb_pos[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                vec3_set_zero(ent->character->speed.m_floats);
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_ANIMATION_LARA_CLIMB_LEFT:
            cmd->rot[0] = 0.0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(cmd->action == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = cmd->climb_pos[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->character->speed.m_floats);
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                break;
            }

            if((cmd->action == 1) && (cmd->move[1] ==-1))
            {
                t = engine_frame_time * ent->current_speed * ent->character->speed_mult;
                vec3_mul_scalar(move, ent->transform + 0, -t);
                ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                Character_CheckNextPenetration(ent, cmd, move);
                ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                if(cmd->horizontal_collide == 0)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// climb edge left
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG);
                }
            }
            else if(cmd->action == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG);
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_ANIMATION_LARA_CLIMB_RIGHT:
            cmd->rot[0] = 0.0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(cmd->action == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE ) * ent->transform[4 + 1];
                    pos[2] = cmd->climb_pos[2] - ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->character->speed.m_floats);
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                break;
            }

            if((cmd->action == 1) && (cmd->move[1] == 1))
            {
                t = engine_frame_time * ent->current_speed * ent->character->speed_mult;
                vec3_mul_scalar(move, ent->transform + 0, t);
                ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                Character_CheckNextPenetration(ent, cmd, move);
                ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                if(cmd->horizontal_collide == 0)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// climb edge right
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG);
                }
            }
            else if(cmd->action == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG);
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_ANIMATION_LARA_CLIMB_OUT_OF_WATER:
            ent->character->no_fix = 0x01;
            ent->current_speed = 0.0;
            vec3_copy(pos, cmd->climb_pos);
            //vec3_set_zero(ent->character->speed.m_floats);
        case TR_ANIMATION_LARA_CLIMB_2CLICK:
        case TR_ANIMATION_LARA_CLIMB_3CLICK:
            cmd->rot[0] = 0.0;
            ent->character->no_fix = 0x01;
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))
            {
                ent->move_type = MOVE_ON_FLOOR;
                vec3_copy(pos, cmd->climb_pos);
            }
            break;

            /*
             * Free fall section
             */
        // fall and landing  - test water cases
        case TR_ANIMATION_LARA_UNKNOWN3:
        case TR_ANIMATION_LARA_FREE_FALL_LONG_NO_HURT:
        case TR_ANIMATION_LARA_FORWARD_TO_FREE_FALL:
        case TR_ANIMATION_LARA_LEFT_TO_FREE_FALL:
        case TR_ANIMATION_LARA_RIGHT_TO_FREE_FALL:
        case TR_ANIMATION_LARA_FREE_FALL_TO_LONG:
        case TR_ANIMATION_LARA_FREE_FALL_TO_SIDE_LANDING:
        case TR_ANIMATION_LARA_START_FLY_LIKE_FISH_LEFT:
        case TR_ANIMATION_LARA_START_FLY_LIKE_FISH_RIGHT:
        case TR_ANIMATION_LARA_FREE_FALL_NO_HURT:
        case TR_ANIMATION_LARA_SMASH_JUMP_CONTINUE:
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if((ent->current_animation != TR_ANIMATION_LARA_UNKNOWN3) && (ent->current_animation != TR_ANIMATION_LARA_SMASH_JUMP_CONTINUE))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);   // landing hard
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
                ent->dir_flag = ENT_STAY;
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue fall down
            }
            break;

        case TR_ANIMATION_LARA_JUMPING_FORWARD_RIGHT:
        case TR_ANIMATION_LARA_JUMPING_FORWARD_LEFT:
            cmd->rot[0] *= 0.4;

            if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else                                                                // jump forward
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                if((ent->current_frame == 1) & (1 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT)))
                {
                    Character_SetToJump(ent, cmd, 3000.0);                      // activated on frame 2 starting
                }
            }
            break;

        case TR_ANIMATION_LARA_START_FLY_FORWARD_RIGHT:
        case TR_ANIMATION_LARA_START_FLY_FORWARD_LEFT:
            cmd->rot[0] *= 0.4;                                                 // Flying momentum is much less
            // state_id == 1 === state_id == 2
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(cmd->move[0] == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);   // landing to run (collide with ceiling)
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
            }
            else if(cmd->horizontal_collide & 0x01)                             ///Lwmte: Also smash-jump on fail.
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 1);
                break;
            }
            else if(cmd->action == 0 && cmd->shift == 1 && cmd->move[0] == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SWANDIVE_BEGIN);     // like fish
            }
            else if(cmd->roll)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_ROLL);
            }
            else if(cmd->action)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TRY_HANG_FALL);      // TR_ANIMATION_LARA_RUN_TO_GRAB_...
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue flying
            }
            break;

        case TR_ANIMATION_LARA_CONTINUE_FLY_FORWARD:
            // 9            45  - TR_ANIMATION_LARA_UNKNOWN3
            // 11(x4)       94, 100, 248, 249 (diff. int-s) TR_STATE_LARA_TRY_HANG_FALL
            // 52           158 - TR_ANIMATION_LARA_FREE_FALL_FISH_START - TR_STATE_LARA_SWANDIVE_BEGIN*/
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if((cmd->action == 0) && (cmd->move[0] == 1) && (cmd->crouch == 0))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
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
            else if(cmd->action == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SWANDIVE_BEGIN);     // fly like fish
            }
            else if(cmd->action == 0 && cmd->roll)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_ROLL);
            }
            else if(cmd->action == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TRY_HANG_FALL);
            }
            else
            {
                if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_1)
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue flying
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);       // free falling
                }
            }
            break;

        case TR_ANIMATION_LARA_START_FREE_FALL:
            //id == 1 === id == 2
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else
            {                                                                   ///@FIXME: check real speed
                if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_1)
                {
                    ent->anim_flags = ANIM_LOOP_LAST_FRAME;
                }
                Character_UpdateCurrentSpeed(ent, 0);
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue falling down
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_LONG:
            //id == 1 === id == 2
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(cmd->kill)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
                }
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue falling down
            }
            break;

            /*
             * FREE FALL TO UNDERWATER CASES
             */
        case TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER:
            ent->angles[1] = -45.0;
            cmd->rot[1] = 0.0;
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))
            {
                ent->angles[1] = -45.0;
            }
            break;

        case TR_ANIMATION_LARA_FISH_TO_UNDERWATER2:                             // ok
        case TR_ANIMATION_LARA_FISH_TO_UNDERWATER1:                             // ok
            ent->angles[1] = -45.0;
            cmd->rot[1] = 0.0;
            cmd->jump = 1;
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))
            {
                ent->angles[1] = -45.0;
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_FORWARD:
            if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
            }
            else if(cmd->action)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TRY_HANG_FALL);
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // next fall down stage
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_MIDDLE:
            if((cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR) && cmd->move[0] == 1 && cmd->jump == 1 && cmd->crouch == 0 && cmd->shift == 0 && cmd->kill == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_RUN_FORWARD);
            }
            else if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                //Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // middle landing
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
            }
            else if(ent->move_type == MOVE_UNDER_WATER)                         // to the water
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(cmd->horizontal_collide & 0x01)                             // smash to wall
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 0);
            }
            else if(cmd->action)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TRY_HANG_FALL);
            }
            else if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_VERTICAL:
             // 2            82  - TR_ANIMATION_LARA_LANDING_MIDDLE
             // 9            36  - TR_ANIMATION_LARA_FREE_FALL_LONG_NO_HURT
             // 11           94  - TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // middle landing
            }
            else if(cmd->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 0);
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(cmd->action)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_TRY_HANG_FALL);
            }
            else if(ent->character->speed.m_floats[2] < -FREE_FALL_SPEED_2)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);   // next free fall stage
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_FISH_START:
            if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // landing - roll
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_DIVING);
            }
            else if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_2)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SWANDIVE_END);       // next stage
            }
            break;

        case TR_ANIMATION_LARA_FREE_FALL_FISH:
            cmd->rot[0] = 0;
            /// 68 - 208
            if((cmd->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(cmd->kill == 1)
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
                }
                else
                {
                    //Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);   // landing - roll; does not works with statechange
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_ROLL, 0);
                }
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_DIVING);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // fly down
            }
            break;

        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_TO_HANG:
            if((ent->move_type != MOVE_CLIMBING) && (cmd->action == 1))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + ent->bf.bb_max[1];              // here we need bbox coordinates... maybe...
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                if(climb.climb_flag && next_fc.edge_hit)
                {
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                    ent->angles[0] = next_fc.edge_z_ang;
                    Entity_UpdateRotation(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                }
            }

            if((cmd->action == 1) && (ent->move_type == MOVE_CLIMBING))
            {
                pos[0] = cmd->climb_pos[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                pos[1] = cmd->climb_pos[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                pos[2] = cmd->climb_pos[2] -  ent->bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                vec3_set_zero(ent->character->speed.m_floats);
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_HANG))
                {
#if OSCILLATE_HANG_USE
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    ent->collision_offset.m_floats[2] -= ent->character->max_step_up_height;
                    Character_CheckNextPenetration(ent, cmd, move);
                    ent->collision_offset.m_floats[2] += ent->character->max_step_up_height;
                    if(!cmd->horizontal_collide)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    }
#endif
                }
            }
            else if(cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);
            }
            else if(ent->move_type == MOVE_UNDER_WATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_FREEFALL);   // next stage
            }
            break;

        case TR_ANIMATION_LARA_LANDING_FORWARD_BOTH:
            if(cmd->kill && (cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            }
            break;

            /*
             * LANDING SECTION
             */
        case TR_ANIMATION_LARA_LANDING_LIGHT:
            if(cmd->kill && (cmd->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if(cmd->jump == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_JUMP_PREPARE);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // stay
            }
            break;
         /*
        case TR_ANIMATION_LARA_LANDING_MIDDLE:                                  // It is a side landing too
            if(cmd->kill == 1)
            {
                cmd->rot[0] = 0.0;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_DEATH);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                cmd->rot[0] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                cmd->rot[0] = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_Frame(ent, engine_frame_time, 1);                        // start run
            }
            else if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->crouch == 0)
            {
                Entity_Frame(ent, engine_frame_time, 6);                        // rotate
            }
            else if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->crouch == 0)
            {
                Entity_Frame(ent, engine_frame_time, 7);                        // rotate
            }
            else if(cmd->jump == 1 && cmd->crouch == 0 && (cmd->move[0] != 0 || cmd->move[1] != 0))
            {
                cmd->rot[0] = 0.0;
                Entity_Frame(ent, engine_frame_time, 15);                       // side jumping
            }
            else
            {
                cmd->rot[0] = 0.0;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);                       // stay
            }
            break;
            */
        case TR_ANIMATION_LARA_LANDING_HARD:
            if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_1)          // landing light
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_LIGHT, 0);
            }
            else if(ent->character->speed.m_floats[2] > -FREE_FALL_SPEED_2)     // landing middle
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // landing hard
            }
            break;

        case TR_ANIMATION_LARA_LANDING_FROM_RUN:
            if(cmd->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(cmd->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // landing hard
            }
            break;

            /*
             * WATER ANIMATIONS
             */
        case TR_ANIMATION_LARA_UNDERWATER_IDLE:
            if(ent->move_type != MOVE_UNDER_WATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WATER_DEATH);
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_FORWARD);
            }
            else if(cmd->action == 1)
            {
                //Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_PICKUP);
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SWITCH_DOWN);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // underwater IDLE
            }
            break;

        case TR_ANIMATION_LARA_UNDERWATER_SWIM_FORWARD:
            if(ent->move_type != MOVE_UNDER_WATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WATER_DEATH);
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
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue swimming
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_INERTIA);
            }
            break;

        case TR_ANIMATION_LARA_UNDERWATER_SWIM_SOLID:
            if(cmd->kill == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WATER_DEATH);
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_FORWARD);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_STOP);
            }
            break;

        case TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN:
            if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT))
            {
                ent->angles[0] += 180.0;
            }
            break;

        case TR_ANIMATION_LARA_ONWATER_IDLE:
            if((cmd->action == 1) && (cmd->move[0] == 1) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->character->max_step_up_height + LARA_HANG_VERTICAL_EPSILON;                // inc for water_surf.z
                t = ent->character->max_step_up_height;
                ent->character->max_step_up_height *= -1.0;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                ent->character->max_step_up_height = t;
                if((climb.climb_flag == CLIMB_FULL_HEIGHT) && (next_fc.edge_point.m_floats[2] - pos[2] < ent->character->max_step_up_height))
                {
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_CLIMBING;
                    ent->angles[0] = next_fc.edge_z_ang;
                    ent->current_speed = 0.0;
                    Entity_UpdateRotation(ent);
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                }
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->character->speed.m_floats);
                cmd->rot[0] = 0.0;
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP))
                {
                    ent->current_speed = 0.0;
                    vec3_set_zero(ent->character->speed.m_floats);
                    vec3_copy(pos, cmd->climb_pos);
                    cmd->climb_pos[0] += ent->transform[4 + 0] * 64.0;
                    cmd->climb_pos[1] += ent->transform[4 + 1] * 64.0;
                }
            }
            else if(cmd->kill)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WATER_DEATH);
            }
            else if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_FORWARD);
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_BACK);
            }
            else if(cmd->move[1] ==-1 && cmd->shift == 1)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
                cmd->rot[0] = 0.0;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_LEFT);
            }
            else if(cmd->move[1] == 1 && cmd->shift == 1)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
                cmd->rot[0] = 0.0;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_RIGHT);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // onwater IDLE
            }
            break;

        case TR_ANIMATION_LARA_ONWATER_SWIM_FORWARD:
            if((cmd->action == 1) && (cmd->move[0] == 1) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                vec3_mul_scalar(offset, ent->transform + 4, t);
                offset[2] += ent->character->max_step_up_height + LARA_HANG_VERTICAL_EPSILON;   // inc for water_surf.z
                t = ent->character->max_step_up_height;
                ent->character->max_step_up_height *= -1.0;
                climb = Character_CheckClimbability(ent, offset, &next_fc, 0.0);
                ent->character->max_step_up_height = t;
                if((climb.climb_flag == CLIMB_FULL_HEIGHT) && (next_fc.edge_point.m_floats[2] - pos[2] < ent->character->max_step_up_height))
                {
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_CLIMBING;
                    ent->angles[0] = next_fc.edge_z_ang;
                    ent->current_speed = 0.0;
                    Entity_UpdateRotation(ent);
                    vec3_copy(cmd->climb_pos, next_fc.edge_point.m_floats);
                }
            }

            if(cmd->kill)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_WATER_DEATH);
            }
            else if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->character->speed.m_floats);
                cmd->rot[0] = 0.0;
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_STOP))
                {
                    ent->current_speed = 0.0;
                    vec3_set_zero(ent->character->speed.m_floats);
                    vec3_copy(pos, cmd->climb_pos);
                    cmd->climb_pos[0] += ent->transform[4 + 0] * 64.0;
                    cmd->climb_pos[1] += ent->transform[4 + 1] * 64.0;
                }
            }
            else if(cmd->jump == 1)
            {
                t = pos[2];
                Character_GetHeightInfo(pos, &next_fc);
                Character_FixPenetrations(ent, cmd, NULL);
                pos[2] = t;
                if(2 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_UNDERWATER_FORWARD)) // dive
                {
                    ent->move_type = MOVE_UNDER_WATER;
                }
            }
            else if(cmd->move[0] == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // swim forward
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_STOP);
            }
            break;

        case TR_ANIMATION_LARA_ONWATER_SWIM_BACK:
            if(cmd->move[0] == -1 && cmd->jump == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue swimming
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_STOP);
            }
            break;

        case TR_ANIMATION_LARA_ONWATER_SWIM_LEFT:
            cmd->rot[0] = 0.0;
            if(cmd->move[0] == 0 && cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue swimming
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_STOP);
            }
            break;

        case TR_ANIMATION_LARA_ONWATER_SWIM_RIGHT:
            cmd->rot[0] = 0.0;
            if(cmd->move[0] == 0 && cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // continue swimming
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_ONWATER_STOP);
            }
            break;

            /*
             * interactive animations
             */
        /*case TR_ANIMATION_LARA_STAY_TO_GRAB:
            if(2 <= Entity_Frame(ent, engine_frame_time, -1))
            {
                Character_SetToJump(ent, cmd, 3000);
            }
            break;*/

            /*
             * CROUCH SECTION
             */
        case TR_ANIMATION_LARA_CROUCH_IDLE:
            ent->dir_flag = ENT_MOVE_FORWARD;
            move[0] = pos[0];
            move[1] = pos[1];
            move[2] = pos[2] + 0.5 * ent->collision_offset.m_floats[2];
            Character_GetHeightInfo(move, &next_fc);
            if((cmd->crouch == 0) && (!next_fc.ceiling_hit || (next_fc.ceiling_point.m_floats[2] - pos[2] >= ent->character->Height)))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_STOP);       // Back to stand
            }
            else if(cmd->move[0] != 0 || cmd->kill == 1)
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_IDLE); // Both forward & back provoke crawl state
            }
            else if(cmd->jump == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_ROLL);// Crouch roll
            }
            else if(cmd->action)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_PICKUP);     // Pick up item 67
            }
            else
            {
                if(CVAR_get_val_d("engine_version") > 5)
                {
                    if(cmd->move[1] == 1 && cmd->jump == 0 && cmd->shift == 0)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_TURN_RIGHT);
                    }
                    else if(cmd->move[1] ==-1 && cmd->jump == 0 && cmd->shift == 0)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_TURN_LEFT);
                    }
                }
                else
                {
                    cmd->rot[0] = 0.0;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);// continue crouching
                }
            }
            break;

        case TR_ANIMATION_LARA_CRAWL_IDLE:
            ///id = 88 - 289 anim climb down
            ///id = 31 - stay here
            /// other anims ???
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(cmd->crouch == 0 || cmd->kill == 1)
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_IDLE);
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
                offset[2] += 128.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) &&
                   (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
                   (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_FORWARD);  // In TR4+, first state is crawlspace jump
                }
                else
                {
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SHIMMY_RIGHT);   // Don't go
                }
            }
            else if(cmd->move[0] == -1)
            {
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                Character_CheckNextPenetration(ent, cmd, move);
                vec3_mul_scalar(offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
                offset[2] += 128.0;
                vec3_add(offset, offset, pos);
                Character_GetHeightInfo(offset, &next_fc);
                if((cmd->horizontal_collide == 0) &&
                   (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
                   (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_BACK);
                }
                /*else if((cmd->horizontal_collide == 0) && (cmd->action) &&
                          (next_fc.floor_point.m_floats[2] <= pos[2] - ent->character->Height))
                {
                    TR_STATE_LARA_CRAWL_TO_CLIMB;
                    // Climb down here!
                }*/
                else
                {
                    ent->dir_flag = ENT_STAY;
                    Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SHIMMY_RIGHT);  // Don't go
                }
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_SHIMMY_RIGHT);      // Stay idle.
            }
            break;

        case TR_ANIMATION_LARA_CRAWL_FORWARD:
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            vec3_mul_scalar(offset, ent->transform + 4, CRAWL_FORWARD_OFFSET);
            offset[2] += 128.0;
            vec3_add(offset, offset, pos);
            Character_GetHeightInfo(offset, &next_fc);

            if((cmd->move[0] == 1) && (cmd->horizontal_collide == 0) &&
                (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
                (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // Move forward
            }
            else
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_IDLE); // Stop
            }
            break;

        case TR_ANIMATION_LARA_CRAWL_BACKWARD:
            ent->dir_flag = ENT_MOVE_BACKWARD;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            vec3_mul_scalar(offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
            offset[2] += 128.0;
            vec3_add(offset, offset, pos);
            Character_GetHeightInfo(offset, &next_fc);
            if((cmd->move[0] == -1) && (cmd->horizontal_collide == 0) &&
               (next_fc.floor_point.m_floats[2] < pos[2] + ent->character->min_step_up_height) &&
               (next_fc.floor_point.m_floats[2] > pos[2] - ent->character->min_step_up_height))
            {
                ent->current_speed = 16.0;                                      ///@FIXME: magick!
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // Move backward
            }
            /*else if((cmd->move[0] == -1) && (cmd->horizontal_collide == 0) && (cmd->action) &&
                    (next_fc.floor_point.m_floats[2] <= pos[2] - ent->character->Height))
            {
                TR_STATE_LARA_CRAWL_TO_CLIMB;
                // Climb down here!
            }*/
            else
            {
                ent->dir_flag = ENT_STAY;
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_IDLE); // Stop
            }
            break;

        case TR_ANIMATION_LARA_CRAWL_TURN_LEFT:
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] *= ((ent->current_frame > 1) && (ent->current_frame < 7))?(0.6):(0.0);

            if(cmd->move[1] == -1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // rotate next
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_IDLE); // stop
            }
            break;

        case TR_ANIMATION_LARA_CRAWL_TURN_RIGHT:
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->rot[0] *= ((ent->current_frame > 1) && (ent->current_frame < 7))?(0.6):(0.0);

            if(cmd->move[1] == 1)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // rotate next
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CRAWL_IDLE); // stop
            }
            break;

        case TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN:
        case TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_CONTINUE:
            cmd->rot[0] = 0.0;
            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            Character_CheckNextPenetration(ent, cmd, move);
            if(cmd->horizontal_collide == 1)                                    // Smash into wall
            {
                Entity_SetAnimation(ent,TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_END, 0);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // Rollin'
            }
            break;

        case TR_ANIMATION_LARA_CROUCH_SMASH_BACKWARD:
            i = ent->current_frame;
            Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            if(i > ent->current_frame)                                      // if cycled, than next anim!
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CROUCH_IDLE, 0);     // Because anim has no state changes.
            }
            break;

        case TR_ANIMATION_LARA_CROUCH_TURN_LEFT:
        case TR_ANIMATION_LARA_CROUCH_TURN_RIGHT:
            cmd->rot[0] *= ((ent->current_frame > 3) && (ent->current_frame < 23))?(0.6):(0.0);

            if(cmd->move[1] == 0)
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CROUCH_IDLE);
            }
            else
            {
                Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);    // Continue rotating
            }
            break;

        case TR_ANIMATION_LARA_WALL_SMASH_LEFT:
        case TR_ANIMATION_LARA_WALL_SMASH_RIGHT:
        case TR_ANIMATION_LARA_SMASH_JUMP:
            if( (1 <= Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT)) &&
                (ent->current_frame == 1) )
                    Controls_JoyRumble(1, 150);
            cmd->rot[0] = 0;
            break;			
			
            /*
             * intermediate animations are processed automatically.
             */
        default:
            cmd->rot[0] = 0;
            Entity_Frame(ent, engine_frame_time, TR_STATE_LARA_CURRENT);
            break;
    };

    return 0;
}
