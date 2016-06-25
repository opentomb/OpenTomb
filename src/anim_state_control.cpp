
#include <stdlib.h>
#include <stdio.h>

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/polygon.h"

#include "render/camera.h"
#include "render/frustum.h"
#include "render/render.h"
#include "engine.h"
#include "audio.h"
#include "physics.h"
#include "room.h"
#include "world.h"
#include "game.h"
#include "skeletal_model.h"
#include "entity.h"
#include "script.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "vt/tr_versions.h"
#include "resource.h"
#include "controls.h"

/*
 * WALL CLIMB:
 * preframe do {save pos}
 * postframe do {if(out) {load pos; do command;}}
 */

#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)

#define PENETRATION_TEST_OFFSET     (48.0)        ///@TODO: tune it!
#define WALK_FORWARD_OFFSET         (96.0)        ///@FIXME: find real offset
#define WALK_BACK_OFFSET            (16.0)
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

void ent_stop_traverse(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        float *v = ent->character->traversed_object->transform + 12;
        int i = v[0] / TR_METERING_SECTORSIZE;
        v[0] = i * TR_METERING_SECTORSIZE + 512.0;
        i = v[1] / TR_METERING_SECTORSIZE;
        v[1] = i * TR_METERING_SECTORSIZE + 512.0;
        Entity_UpdateRigidBody(ent->character->traversed_object, 1);
        ent->character->traversed_object = NULL;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_on_floor(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->move_type = MOVE_ON_FLOOR;
        ent->transform[12 + 2] = ent->character->height_info.floor_hit.point[2];
        Entity_GhostUpdate(ent);
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_turn_fast(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->bf->animations.next_state = TR_STATE_LARA_TURN_FAST;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_on_floor_after_climb(entity_p ent, ss_animation_p ss_anim, int state)
{
    animation_frame_p af = ss_anim->model->animations + ss_anim->current_animation;
    if(ss_anim->current_frame >= af->frames_count - 1)
    {
        float p[3], move[3];

        Mat4_vec3_mul(move, ent->transform, ent->bf->bone_tags[0].full_transform + 12);
        Entity_SetAnimation(ent, af->next_anim->id, af->next_frame);
        Mat4_vec3_mul(p, ent->transform, ent->bf->bone_tags[0].full_transform + 12);
        vec3_sub(move, move, p);
        vec3_add(ent->transform+12, ent->transform+12, move);
        ent->transform[12 + 2] = ent->character->climb.point[2];
        SSBoneFrame_Update(ent->bf);
        Entity_UpdateRigidBody(ent, 0);
        Entity_GhostUpdate(ent);
        ent->move_type = MOVE_ON_FLOOR;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_underwater(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->move_type = MOVE_UNDERWATER;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_free_falling(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->move_type = MOVE_FREE_FALLING;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_set_cmd_slide(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->character->resp.slide = 0x01;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_correct_diving_angle(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->angles[1] = -45.0;
        Entity_UpdateTransform(ent);
        ss_anim->onEndFrame = NULL;
    }
}

void ent_to_on_water(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->transform[12 + 2] = ent->character->height_info.transition_level;
        Entity_GhostUpdate(ent);
        ent->move_type = MOVE_ON_WATER;
        ss_anim->onEndFrame = NULL;
    }
}

void ent_climb_out_of_water(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        float *v = ent->character->climb.point;

        vec3_add_mul(ent->transform+12, v, ent->transform+4, 48.0);             // temporary stick
        ent->transform[12 + 2] = v[2];
        Entity_GhostUpdate(ent);
        ss_anim->onEndFrame = NULL;
    }
}

void ent_to_edge_climb(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        float *v = ent->character->climb.point;
        ent->transform[12 + 0] = v[0] - ent->transform[4 + 0] * ent->bf->bb_max[1];
        ent->transform[12 + 1] = v[1] - ent->transform[4 + 1] * ent->bf->bb_max[1];
        ent->transform[12 + 2] = v[2] - ent->bf->bb_max[2];
        Entity_GhostUpdate(ent);
        ss_anim->onEndFrame = NULL;
    }
}

void ent_to_monkey_swing(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        ent->move_type = MOVE_MONKEYSWING;
        ent->transform[12 + 2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
        Entity_GhostUpdate(ent);
        ss_anim->onEndFrame = NULL;
    }
}

void ent_crawl_to_climb(entity_p ent, ss_animation_p ss_anim, int state)
{
    if(state == 0x02)
    {
        character_command_p cmd = &ent->character->cmd;

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

        ent->no_fix_all = 0x01;
        ent_to_edge_climb(ent, ss_anim, state);
        ss_anim->onEndFrame = NULL;
    }
}

/**
 * Current animation != current state - use original TR state concept!
 */
int State_Control_Lara(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    int i;
    int clean_action = (ent->character->cmd.action && (ent->character->weapon_current_state == WEAPON_STATE_HIDE));
    float t, *pos = ent->transform + 12;
    float global_offset[3], move[3], climb_from[3], climb_to[3];
    height_info_t next_fc, *curr_fc;
    climb_info_t *climb = &ent->character->climb;
    character_command_p cmd = &ent->character->cmd;
    character_response_p resp = &ent->character->resp;

    curr_fc = &ent->character->height_info;
    next_fc.self = ent->self;
    ent->no_fix_skeletal_parts = BODY_PART_LEGS_3 | BODY_PART_HANDS_3;
    ent->character->rotate_speed_mult = 1.0f;

    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    int8_t low_vertical_space = (curr_fc->floor_hit.hit && curr_fc->ceiling_hit.hit && (curr_fc->ceiling_hit.point[2] - curr_fc->floor_hit.point[2] < ent->character->Height - LARA_HANG_VERTICAL_EPSILON));
    int8_t last_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count <= ss_anim->current_frame + 1;

    if(resp->kill == 1)   // Stop any music, if Lara is dead.
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
    switch(ss_anim->last_state)
    {
        /*
         * Base onfloor animations
         */
        case TR_STATE_LARA_STOP:
            // Reset directional flag only on intermediate animation!

            if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)
            {
                ent->dir_flag = ENT_STAY;
            }

            cmd->rot[0] = 0;
            cmd->crouch |= low_vertical_space;
            Character_Lean(ent, cmd, 0.0);

            if( (climb->can_hang &&
                (climb->next_z_space >= ent->character->Height - LARA_HANG_VERTICAL_EPSILON) &&
                (ent->move_type == MOVE_CLIMBING)) ||
                (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID) )
            {
                ent->move_type = MOVE_ON_FLOOR;
            }

            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ent->no_fix_skeletal_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                ent->dir_flag = ENT_STAY;
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_DEATH;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, ent->id);

                if(cmd->jump)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
                }
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                if(cmd->jump)
                {
                    ent->dir_flag = ENT_MOVE_BACKWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                    Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                }
            }
            else if(cmd->jump)
            {
                if(!curr_fc->quicksand)
                    ss_anim->next_state = TR_STATE_LARA_JUMP_PREPARE;       // jump sideways
            }
            else if(cmd->roll)
            {
                if(!curr_fc->quicksand && ss_anim->current_animation != TR_ANIMATION_LARA_CLIMB_2CLICK)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
            }
            else if(cmd->crouch)
            {
                if(!curr_fc->quicksand)
                    ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if(clean_action && Character_FindTraverse(ent))
            {
                ss_anim->next_state = TR_STATE_LARA_PUSHABLE_GRAB;
                if(ent->transform[4 + 0] > 0.9)
                {
                    t = -ent->character->traversed_object->bf->bb_min[0] + 72.0;
                }
                else if(ent->transform[4 + 0] < -0.9)
                {
                    t = ent->character->traversed_object->bf->bb_max[0] + 72.0;
                }
                else if(ent->transform[4 + 1] > 0.9)
                {
                    t = -ent->character->traversed_object->bf->bb_min[1] + 72.0;
                }
                else if(ent->transform[4 + 1] < -0.9)
                {
                    t = ent->character->traversed_object->bf->bb_max[1] + 72.0;
                }
                else
                {
                    t = 512.0 + 72.0;                                           ///@PARANOID
                }
                float *v = ent->character->traversed_object->transform + 12;
                pos[0] = v[0] - ent->transform[4 + 0] * t;
                pos[1] = v[1] - ent->transform[4 + 1] * t;
            }
            else if(cmd->move[0] == 1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    vec3_mul_scalar(global_offset, ent->transform + 4, WALK_FORWARD_OFFSET);
                    global_offset[2] += ent->bf->bb_max[2];
                    vec3_add(global_offset, global_offset, pos);
                    Character_GetHeightInfo(global_offset, &next_fc);
                    if(((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00)) &&
                       (next_fc.floor_hit.hit && (next_fc.floor_hit.point[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_hit.point[2] <= pos[2] + ent->character->max_step_up_height)))
                    {
                        ent->move_type = MOVE_ON_FLOOR;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                        }
                    }
                }       // end IF CMD->SHIFT
                else
                {
                    vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                    vec3_mul_scalar(global_offset, ent->transform + 4, RUN_FORWARD_OFFSET);
                    global_offset[2] += ent->bf->bb_max[2];
                    Character_CheckNextStep(ent, global_offset, &next_fc);
                    if(((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00)) && (!Character_HasStopSlant(ent, &next_fc)))
                    {
                        ent->move_type = MOVE_ON_FLOOR;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                        }
                    }
                }

                if( (clean_action) &&
                    ((ss_anim->current_animation == TR_ANIMATION_LARA_STAY_IDLE)        ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)       ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_LEFT)  ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_RIGHT)) )
                {
                    t = ent->character->forvard_size + LARA_TRY_HANG_WALL_OFFSET;
                    climb_from[0] = pos[0];
                    climb_from[1] = pos[1];
                    climb_from[2] = pos[2] + 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    climb_to[0] = pos[0] + t * ent->transform[4 + 0];
                    climb_to[1] = pos[1] + t * ent->transform[4 + 1];
                    climb_to[2] = pos[2] + ent->character->max_step_up_height;
                    if(curr_fc->ceiling_hit.hit && (climb_from[2] >= curr_fc->ceiling_hit.point[2] - ent->character->climb_r))
                    {
                        climb_from[2] = curr_fc->ceiling_hit.point[2] - ent->character->climb_r;
                    }
                    Character_CheckClimbability(ent, climb, climb_from, climb_to);
                    if(  climb->edge_hit                                                                &&
                        (climb->next_z_space >= ent->character->Height - LARA_HANG_VERTICAL_EPSILON)    &&
                        (pos[2] + ent->character->max_step_up_height < climb->edge_point[2])      &&
                        (pos[2] + 2944.0 >= climb->edge_point[2]))              // trying to climb on
                    {
                        if(pos[2] + 640.0 >= climb->edge_point[2])
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            pos[2] = climb->edge_point[2] - 512.0;
                            vec3_copy(climb->point, climb->edge_point);
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                            ent->no_fix_all = 0x01;
                            ss_anim->onEndFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                        else if(pos[2] + 896.0 >= climb->edge_point[2])
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            pos[2] = climb->edge_point[2] - 768.0;
                            vec3_copy(climb->point, climb->edge_point);
                            Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                            ent->no_fix_all = 0x01;
                            ss_anim->onEndFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                    }   // end IF MOVE_LITTLE_CLIMBING

                    climb_from[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    if(curr_fc->ceiling_hit.hit && (climb_from[2] >= curr_fc->ceiling_hit.point[2] - ent->character->climb_r))
                    {
                        climb_from[2] = curr_fc->ceiling_hit.point[2] - ent->character->climb_r;
                    }
                    Character_CheckClimbability(ent, climb, climb_from, climb_to);
                    if(climb->edge_hit                                                                &&
                       (climb->next_z_space >= ent->character->Height - LARA_HANG_VERTICAL_EPSILON)   &&
                       (pos[2] + ent->character->max_step_up_height < climb->edge_point[2])     &&
                       (pos[2] + 2944.0 >= climb->edge_point[2]))               // trying to climb on
                    {
                        if(pos[2] + 1920.0 >= climb->edge_point[2])
                        {
                            ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                            break;
                        }
                    }   // end IF MOVE_BIG_CLIMBING

                    Character_CheckWallsClimbability(ent, climb);
                    if(climb->wall_hit)
                    {
                        ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                        break;
                    }
                }
            }       // end CMD->MOVE FORWARD
            else if(cmd->move[0] == -1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                    {
                        vec3_mul_scalar(global_offset, ent->transform + 4, -WALK_BACK_OFFSET);
                        global_offset[2] += ent->bf->bb_max[2];
                        vec3_add(global_offset, global_offset, pos);
                        Character_GetHeightInfo(global_offset, &next_fc);
                        if((next_fc.floor_hit.hit && (next_fc.floor_hit.point[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_hit.point[2] <= pos[2] + ent->character->max_step_up_height)))
                        {
                            ent->dir_flag = ENT_MOVE_BACKWARD;
                            ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                        }
                    }
                }
                else    // RUN BACK
                {
                    vec3_mul_scalar(move, ent->transform + 4, - PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                    {
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_RUN_BACK;
                        }
                    }
                }
            }       // end CMD->MOVE BACK
            else if(cmd->move[1] == 1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                    {
                        vec3_mul_scalar(global_offset, ent->transform + 0, RUN_FORWARD_OFFSET);
                        global_offset[2] += ent->bf->bb_max[2];
                        i = Character_CheckNextStep(ent, global_offset, &next_fc);
                        if((resp->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                        {
                            cmd->rot[0] = 0;
                            ent->dir_flag = ENT_MOVE_RIGHT;
                            ss_anim->next_state = TR_STATE_LARA_WALK_RIGHT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_TURN_RIGHT_SLOW;
                }
            }       // end MOVE RIGHT
            else if(cmd->move[1] == -1)
            {
                if(cmd->shift)
                {
                    vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                    {
                        vec3_mul_scalar(global_offset, ent->transform + 0, -RUN_FORWARD_OFFSET);
                        global_offset[2] += ent->bf->bb_max[2];
                        i = Character_CheckNextStep(ent, global_offset, &next_fc);
                        if((resp->horizontal_collide == 0) && (i >= CHARACTER_STEP_DOWN_LITTLE && i <= CHARACTER_STEP_UP_LITTLE))
                        {
                            cmd->rot[0] = 0;
                            ent->dir_flag = ENT_MOVE_LEFT;
                            ss_anim->next_state = TR_STATE_LARA_WALK_LEFT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_TURN_LEFT_SLOW;
                }
            }       // end MOVE LEFT
            else
            {
                Character_FixPosByFloorInfoUnderLegs(ent);
            }
            break;

        case TR_STATE_LARA_JUMP_PREPARE:
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);

            if(resp->slide == CHARACTER_SLIDE_BACK)      // Slide checking is only for jumps direction correction!
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                cmd->move[0] = -1;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                cmd->move[0] = 1;
            }
            if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
            {
                //Stay, directional jumps are not allowed whilst in wade depth
            }
            else if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                if(Entity_CheckNextPenetration(ent, move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;           // jump forward
                }
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                if(Entity_CheckNextPenetration(ent, move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;              // jump backward
                }
            }
            else if(cmd->move[1] == 1)
            {
                ent->dir_flag = ENT_MOVE_RIGHT;
                vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                if(Entity_CheckNextPenetration(ent, move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_LEFT;              // jump right
                }
            }
            else if(cmd->move[1] ==-1)
            {
                ent->dir_flag = ENT_MOVE_LEFT;
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                if(Entity_CheckNextPenetration(ent, move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_RIGHT;             // jump left
                }
            }
            break;

        case TR_STATE_LARA_JUMP_BACK:
            cmd->rot[0] = 0;
            if(resp->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Engine_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_FORWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else if((ent->move_type == MOVE_UNDERWATER) || (ent->speed[2] <= -FREE_FALL_SPEED_2))
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                   // free falling
            }
            else if(cmd->roll)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

        case TR_STATE_LARA_JUMP_LEFT:
            cmd->rot[0] = 0;
            if(resp->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Engine_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_RIGHT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_JUMP_RIGHT:
            cmd->rot[0] = 0;
            if(resp->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Engine_JoyRumble(200.0, 200);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_LEFT;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_RUN_BACK:
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_BACK, 0);
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            break;


        case TR_STATE_LARA_TURN_LEFT_SLOW:
        case TR_STATE_LARA_TURN_RIGHT_SLOW:
            ent->character->rotate_speed_mult = 0.7f;
            ent->dir_flag = ENT_STAY;
            Character_Lean(ent, cmd, 0.0);
            ent->no_fix_skeletal_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

            if(cmd->move[0] == 1)
            {
                int substance_state = Entity_GetSubstanceState(ent);
                if((substance_state == ENTITY_SUBSTANCE_NONE) ||
                   (substance_state == ENTITY_SUBSTANCE_WATER_SHALLOW))
                {
                    if(cmd->shift == 1)
                    {
                        ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                        ent->dir_flag = ENT_MOVE_FORWARD;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }

            }
            else if(((ss_anim->last_state == TR_STATE_LARA_TURN_LEFT_SLOW ) && (cmd->move[1] == -1)) ||
                    ((ss_anim->last_state == TR_STATE_LARA_TURN_RIGHT_SLOW) && (cmd->move[1] ==  1))  )
            {
                int substance_state = Entity_GetSubstanceState(ent);
                if((last_frame) &&
                   (substance_state != ENTITY_SUBSTANCE_WATER_WADE) &&
                   (substance_state != ENTITY_SUBSTANCE_QUICKSAND_CONSUMED) &&
                   (substance_state != ENTITY_SUBSTANCE_QUICKSAND_SHALLOW))
                 {
                     ss_anim->next_state = TR_STATE_LARA_TURN_FAST;
                 }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
            break;

        case TR_STATE_LARA_TURN_FAST:
            // 65 - wade
            ent->dir_flag = ENT_STAY;
            ent->no_fix_skeletal_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            Character_Lean(ent, cmd, 0.0);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->jump == 0 && cmd->crouch == 0 && cmd->shift == 0)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[1] == 0)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
            break;

            /*
             * RUN AND WALK animations section
             */
        case TR_STATE_LARA_RUN_FORWARD:
            vec3_mul_scalar(global_offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            global_offset[2] += ent->bf->bb_max[2];
            i = Character_CheckNextStep(ent, global_offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;
            cmd->crouch |= low_vertical_space;

            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ent->no_fix_skeletal_parts = BODY_PART_HANDS | BODY_PART_LEGS;
            }
            Character_Lean(ent, cmd, 6.0);

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_DEATH;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
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
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if((cmd->move[0] == 1) && (cmd->crouch == 0) && (next_fc.floor_hit.normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                i = Anim_GetAnimDispatchCase(ent->bf, 2);                     // MOST CORRECT STATECHANGE!!!
                if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    pos[2] = next_fc.floor_hit.point[2];
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else //if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
                    pos[2] = next_fc.floor_hit.point[2];
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                vec3_mul_scalar(global_offset, ent->transform + 4, RUN_FORWARD_OFFSET);
                global_offset[2] += 1024.0;
                if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_TO_RUN)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    Engine_JoyRumble(200.0, 200);

                    if(cmd->move[0] == 1)
                    {
                        i = Anim_GetAnimDispatchCase(ent->bf, 2);
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
                if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else if(cmd->shift == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                }
                else if((cmd->jump == 1) && (ss_anim->last_animation != TR_ANIMATION_LARA_STAY_TO_RUN))
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;
                }
                else if(cmd->roll == 1)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->sprint == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_SPRINT;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_SPRINT:
            vec3_mul_scalar(global_offset, ent->transform + 4, RUN_FORWARD_OFFSET);
            Character_Lean(ent, cmd, 12.0);
            global_offset[2] += ent->bf->bb_max[2];
            i = Character_CheckNextStep(ent, global_offset, &next_fc);
            cmd->crouch |= low_vertical_space;

            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ent->no_fix_skeletal_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(!Character_GetParam(ent, PARAM_STAMINA))
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            else if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;    // Normal run then die
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if((next_fc.floor_hit.normale[2] < ent->character->critical_slant_z_component) && (i > CHARACTER_STEP_HORIZONTAL))
            {
                ent->anim_linear_speed = 0.0;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if((next_fc.floor_hit.normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;     // Interrupt sprint
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Engine_JoyRumble(200.0, 200);

                i = Anim_GetAnimDispatchCase(ent->bf, 2);
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
                    ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            else
            {
                if(cmd->jump == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_SPRINT_ROLL;
                }
                else if(cmd->roll == 1)
                {
                    ent->dir_flag = ENT_MOVE_FORWARD;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->crouch == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
                }
                else if(cmd->move[0] == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_FORWARD:
            ent->character->rotate_speed_mult = 0.4f;
            Character_Lean(ent, cmd, 0.0);

            vec3_mul_scalar(global_offset, ent->transform + 4, WALK_FORWARD_OFFSET);
            global_offset[2] += ent->bf->bb_max[2];
            i = Character_CheckNextStep(ent, global_offset, &next_fc);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ent->no_fix_skeletal_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if((next_fc.floor_hit.normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_UP_BIG))
            {
                /*
                 * climb up
                 */
                ent->dir_flag = ENT_STAY;
                i = Anim_GetAnimDispatchCase(ent->bf, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                    vec3_copy(pos, next_fc.floor_hit.point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
                    vec3_copy(pos, next_fc.floor_hit.point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((next_fc.floor_hit.normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                /*
                 * climb down
                 */
                ent->dir_flag = ENT_STAY;
                i = Anim_GetAnimDispatchCase(ent->bf, 2);
                if(i == 1)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                    vec3_copy(climb->point, next_fc.floor_hit.point);
                    vec3_copy(pos, next_fc.floor_hit.point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
                else //if(i == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
                    vec3_copy(climb->point, next_fc.floor_hit.point);
                    vec3_copy(pos, next_fc.floor_hit.point);
                    ent->move_type = MOVE_ON_FLOOR;
                    ent->dir_flag = ENT_MOVE_FORWARD;
                }
            }
            else if((resp->horizontal_collide & 0x01) || (i < CHARACTER_STEP_DOWN_BIG || i > CHARACTER_STEP_UP_BIG) || (low_vertical_space))
            {
                /*
                 * too high
                 */
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit.hit && (curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth))
            {
                ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
            }
            else if(cmd->move[0] == 1 && cmd->crouch == 0 && cmd->shift == 0)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
            break;


        case TR_STATE_LARA_WADE_FORWARD:
            ent->character->rotate_speed_mult = 0.4f;
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->character->height_info.quicksand)
            {
                ent->anim_linear_speed = (ent->anim_linear_speed > MAX_SPEED_QUICKSAND) ? MAX_SPEED_QUICKSAND : ent->anim_linear_speed;
            }

            if(cmd->move[0] == 1)
            {
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                Entity_CheckNextPenetration(ent, move);
            }

            if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if(!curr_fc->floor_hit.hit || ent->move_type == MOVE_FREE_FALLING)  // free fall, next swim
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water)
            {
                if((curr_fc->transition_level - curr_fc->floor_hit.point[2] <= ent->character->wade_depth))
                {
                    // run / walk case
                    if((cmd->move[0] == 1) && (resp->horizontal_collide == 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_STOP;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_hit.point[2] > (ent->character->Height - ent->character->swim_depth))
                {
                    // swim case
                    if(curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->Height + ent->character->max_step_up_height)
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);                                    // swim underwater
                    }
                    else
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_ONWATER_IDLE, 0);                                       // swim onwater
                        ent->move_type = MOVE_ON_WATER;
                        pos[2] = curr_fc->transition_level;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_hit.point[2] > ent->character->wade_depth)              // wade case
                {
                    if((cmd->move[0] != 1) || (resp->horizontal_collide != 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_STOP;
                    }
                }
            }
            else                                                                // no water, stay or run / walk
            {
                if((cmd->move[0] == 1) && (resp->horizontal_collide == 0))
                {
                    if(!curr_fc->quicksand)
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_BACK:
            ent->character->rotate_speed_mult = 0.4f;
            ent->dir_flag = ENT_MOVE_BACKWARD;

            if(ent->character->height_info.quicksand)
            {
                ent->anim_linear_speed = (ent->anim_linear_speed > MAX_SPEED_QUICKSAND) ? MAX_SPEED_QUICKSAND : ent->anim_linear_speed;
            }

            vec3_mul_scalar(global_offset, ent->transform + 4, -WALK_BACK_OFFSET);
            global_offset[2] += ent->bf->bb_max[2];
            i = Character_CheckNextStep(ent, global_offset, &next_fc);
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water && (curr_fc->floor_hit.point[2] + ent->character->Height < curr_fc->transition_level))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_ONWATER_SWIM_BACK, 0);
                ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
                ent->move_type = MOVE_ON_WATER;
            }
            else if((i < CHARACTER_STEP_DOWN_BIG) || (i > CHARACTER_STEP_UP_BIG))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            else if((next_fc.floor_hit.normale[2] >= ent->character->critical_slant_z_component) && (i == CHARACTER_STEP_DOWN_BIG))
            {
                if(ent->no_fix_all == 0x00)
                {
                    int frames_count = ss_anim->model->animations[TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT].frames_count;
                    int frames_count2 = (frames_count + 1) / 2;
                    if((ss_anim->current_frame >= 0) && (ss_anim->current_frame <= frames_count2))
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, ss_anim->current_frame);
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ent->transform[12 + 2] -= (curr_fc->floor_hit.point[2] - next_fc.floor_hit.point[2]);
                        ent->no_fix_all = 0x01;
                    }
                    else if((ss_anim->current_frame >= frames_count) && (ss_anim->current_frame <= frames_count + frames_count2))
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, ss_anim->current_frame - frames_count);
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ent->transform[12 + 2] -= (curr_fc->floor_hit.point[2] - next_fc.floor_hit.point[2]);
                        ent->no_fix_all = 0x01;
                    }
                    else
                    {
                        ent->dir_flag = ENT_STAY;                               // waiting for correct frame
                    }
                }
            }
            else if((cmd->move[0] == -1) && ((cmd->shift) || (ent->character->height_info.quicksand)))
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
            break;

        case TR_STATE_LARA_WALK_LEFT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[1] == -1 && cmd->shift)
            {
                vec3_mul_scalar(global_offset, ent->transform + 0, -RUN_FORWARD_OFFSET);  // not an error - RUN_... more correct here
                global_offset[2] += ent->bf->bb_max[2];
                vec3_add(global_offset, global_offset, pos);
                Character_GetHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit.hit && (next_fc.floor_hit.point[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_hit.point[2] <= pos[2] + ent->character->max_step_up_height))
                //if(curr_fc->leg_l_floor.hit && (curr_fc->leg_l_floor.point[2] > pos[2] - ent->character->max_step_up_height) && (curr_fc->leg_l_floor.point[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    if(!curr_fc->water || (curr_fc->floor_hit.point[2] + ent->character->Height > curr_fc->transition_level)) // if (floor_hit.hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
                        ss_anim->onEndFrame = ent_to_on_water;
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
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
            break;

        case TR_STATE_LARA_WALK_RIGHT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[1] == 1 && cmd->shift)
            {
                vec3_mul_scalar(global_offset, ent->transform + 0, RUN_FORWARD_OFFSET);// not an error - RUN_... more correct here
                global_offset[2] += ent->bf->bb_max[2];
                vec3_add(global_offset, global_offset, pos);
                Character_GetHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit.hit && (next_fc.floor_hit.point[2] > pos[2] - ent->character->max_step_up_height) && (next_fc.floor_hit.point[2] <= pos[2] + ent->character->max_step_up_height))
                //if(curr_fc->leg_r_floor.hit && (curr_fc->leg_r_floor.point[2] > pos[2] - ent->character->max_step_up_height) && (curr_fc->leg_r_floor.point[2] <= pos[2] + ent->character->max_step_up_height))
                {
                    if(!curr_fc->water || (curr_fc->floor_hit.point[2] + ent->character->Height > curr_fc->transition_level)) // if (floor_hit.hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                        ss_anim->onEndFrame = ent_to_on_water;
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
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            Character_FixPosByFloorInfoUnderLegs(ent);
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
                if(clean_action)
                {
                    ent->speed[0] = -ent->transform[4 + 0] * 128.0;
                    ent->speed[1] = -ent->transform[4 + 1] * 128.0;
                    ent->transform[12 + 0] -= ent->transform[4 + 0] * 128.0;    ///@HACK!
                    ent->transform[12 + 1] -= ent->transform[4 + 1] * 128.0;
                }

                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->slide == 0)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if(resp->slide != 0 && cmd->jump == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
            }
            else
            {
                break;
            }

            Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, ent->id);
            break;

        case TR_STATE_LARA_SLIDE_FORWARD:
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);
            ent->dir_flag = ENT_MOVE_FORWARD;

            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->slide == 0)
            {
                if((cmd->move[0] == 1) && (World_GetVersion() >= TR_III))
                {
                     ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                     ss_anim->next_state = TR_STATE_LARA_STOP;                  // stop
                }
            }
            else if(resp->slide != 0 && cmd->jump == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;               // jump
            }
            else
            {
                break;
            }

            Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, ent->id);
            break;

            /*
             * Misk animations
             */
        case TR_STATE_LARA_PUSHABLE_GRAB:
            ent->move_type = MOVE_ON_FLOOR;
            ent->no_fix_all = 0x01;
            cmd->rot[0] = 0;

            if(clean_action)//If Lara is grabbing the block
            {
                ent->dir_flag = ENT_STAY;
                ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;               //We hold it (loop last frame)
                if(cmd->move[0] != 0)
                {
                    int tf = Character_CheckTraverse(ent, ent->character->traversed_object);
                    if((cmd->move[0] == 1) && (tf & 0x01))                      //If player press up push
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;
                        ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PUSH;
                    }
                    else if((cmd->move[0] == -1) && (tf & 0x02))                //If player press down pull
                    {
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;
                        ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PULL;
                    }
                }
            }
            else//Lara has let go of the block
            {
                ent->dir_flag = ENT_STAY;
                ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;                //We no longer loop last frame
                ss_anim->next_state = TR_STATE_LARA_STOP;                       //Switch to next Lara state
            }
            break;

        case TR_STATE_LARA_PUSHABLE_PUSH:
            ent->no_fix_all = 0x01;
            ss_anim->onEndFrame = ent_stop_traverse;
            cmd->rot[0] = 0;
            ent->character->cam_follow_center = 64;
            i = ss_anim->model->animations[ss_anim->current_animation].frames_count;

            if(!clean_action || !(0x01 & Character_CheckTraverse(ent, ent->character->traversed_object)))   //For TOMB4/5 If Lara is pushing and action let go, don't push
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if((ent->character->traversed_object != NULL) && (ss_anim->current_frame > 16) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 16
            {
                bool was_traversed = false;

                if(ent->transform[4 + 0] > 0.9)
                {
                    t = ent->transform[12 + 0] + (ent->bf->bb_max[1] - ent->character->traversed_object->bf->bb_min[0] - 32.0);
                    if(t > ent->character->traversed_object->transform[12 + 0])
                    {
                        ent->character->traversed_object->transform[12 + 0] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 0] < -0.9)
                {
                    t = ent->transform[12 + 0] - (ent->bf->bb_max[1] + ent->character->traversed_object->bf->bb_max[0] - 32.0);
                    if(t < ent->character->traversed_object->transform[12 + 0])
                    {
                        ent->character->traversed_object->transform[12 + 0] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 1] > 0.9)
                {
                    t = ent->transform[12 + 1] + (ent->bf->bb_max[1] - ent->character->traversed_object->bf->bb_min[1] - 32.0);
                    if(t > ent->character->traversed_object->transform[12 + 1])
                    {
                        ent->character->traversed_object->transform[12 + 1] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 1] < -0.9)
                {
                    t = ent->transform[12 + 1] - (ent->bf->bb_max[1] + ent->character->traversed_object->bf->bb_max[1] - 32.0);
                    if(t < ent->character->traversed_object->transform[12 + 1])
                    {
                        ent->character->traversed_object->transform[12 + 1] = t;
                        was_traversed = true;
                    }
                }

                if(World_GetVersion() > TR_III)
                {
                    if(was_traversed)
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,ent->id) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    else
                    {
                        Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                }
                else
                {
                    if( (ss_anim->current_frame == 49)   ||
                        (ss_anim->current_frame == 110)  ||
                        (ss_anim->current_frame == 142)   )
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,ent->id) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                }

                Entity_UpdateRigidBody(ent->character->traversed_object, 1);
            }
            else
            {
                if(World_GetVersion() > TR_III)
                {
                    Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
            }
            break;

        case TR_STATE_LARA_PUSHABLE_PULL:
            ent->no_fix_all = 0x01;
            ss_anim->onEndFrame = ent_stop_traverse;
            cmd->rot[0] = 0;
            ent->character->cam_follow_center = 64;
            i = ss_anim->model->animations[ss_anim->current_animation].frames_count;

            if(!clean_action || !(0x02 & Character_CheckTraverse(ent, ent->character->traversed_object)))   //For TOMB4/5 If Lara is pulling and action let go, don't pull
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if((ent->character->traversed_object != NULL) && (ss_anim->current_frame > 20) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 20
            {
                bool was_traversed = false;

                if(ent->transform[4 + 0] > 0.9)
                {
                    t = ent->transform[12 + 0] + (ent->bf->bb_max[1] - ent->character->traversed_object->bf->bb_min[0] - 32.0);
                    if(t < ent->character->traversed_object->transform[12 + 0])
                    {
                        ent->character->traversed_object->transform[12 + 0] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 0] < -0.9)
                {
                    t = ent->transform[12 + 0] - (ent->bf->bb_max[1] + ent->character->traversed_object->bf->bb_max[0] - 32.0);
                    if(t > ent->character->traversed_object->transform[12 + 0])
                    {
                        ent->character->traversed_object->transform[12 + 0] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 1] > 0.9)
                {
                    t = ent->transform[12 + 1] + (ent->bf->bb_max[1] - ent->character->traversed_object->bf->bb_min[1] - 32.0);
                    if(t < ent->character->traversed_object->transform[12 + 1])
                    {
                        ent->character->traversed_object->transform[12 + 1] = t;
                        was_traversed = true;
                    }
                }
                else if(ent->transform[4 + 1] < -0.9)
                {
                    t = ent->transform[12 + 1] - (ent->bf->bb_max[1] + ent->character->traversed_object->bf->bb_max[1] - 32.0);
                    if(t > ent->character->traversed_object->transform[12 + 1])
                    {
                        ent->character->traversed_object->transform[12 + 1] = t;
                        was_traversed = true;
                    }
                }

                if(World_GetVersion() > TR_III)
                {
                    if(was_traversed)
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,ent->id) == -1)

                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    else
                    {
                        Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                }
                else
                {
                    if( (ss_anim->current_frame == 40)  ||
                        (ss_anim->current_frame == 92)  ||
                        (ss_anim->current_frame == 124) ||
                        (ss_anim->current_frame == 156)  )
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,ent->id) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                }

                Entity_UpdateRigidBody(ent->character->traversed_object, 1);
            }
            else
            {
                if(World_GetVersion() > TR_III)
                {
                    Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
            }
            break;

        case TR_STATE_LARA_ROLL_FORWARD:
            break;

        case TR_STATE_LARA_ROLL_BACKWARD:
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(low_vertical_space)
            {
                ent->dir_flag = ENT_STAY;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            break;

        /*
         * Climbing section
         */
        case TR_STATE_LARA_JUMP_UP:
            cmd->rot[0] = 0;
            if(clean_action && (ent->move_type != MOVE_WALLS_CLIMB) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE + ent->character->climb_r;
                Character_GetMiddleHandsPos(ent, climb_from);
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_from[2] += ent->character->climb_r + engine_frame_time * ent->speed[2];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    vec3_set_zero(ent->speed);
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf->bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    break;
                }
                else
                {
                    Character_CheckWallsClimbability(ent, climb);
                    if((climb->wall_hit) &&
                       (ent->speed[2] < 0.0)) // Only hang if speed is lower than zero.
                    {
                        // Fix the position to the TR metering step.
                        ent->transform[12+2] = (float)((int)((ent->transform[12+2]) / TR_METERING_STEP) * TR_METERING_STEP);
                        ent->move_type = MOVE_WALLS_CLIMB;
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

            if(ent->move_type == MOVE_UNDERWATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_UpdateTransform(ent);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(clean_action && (curr_fc->ceiling_climb) && (curr_fc->ceiling_hit.hit) && (pos[2] + ent->bf->bb_max[2] > curr_fc->ceiling_hit.point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onEndFrame = ent_to_monkey_swing;
            }
            else if(clean_action && (ent->move_type == MOVE_CLIMBING))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_HANG_IDLE, -1);
            }
            else if((resp->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing immediately
            }
            else
            {
                if(ent->speed[2] < -FREE_FALL_SPEED_2)                 // next free fall stage
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    ss_anim->next_state = TR_STATE_LARA_FREEFALL;
                }
                break;
            }
            break;

        case TR_STATE_LARA_REACH:
            cmd->rot[0] = 0;
            if(ent->move_type == MOVE_UNDERWATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_UpdateTransform(ent);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                break;
            }

            if(clean_action && (ent->move_type == MOVE_FREE_FALLING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                Character_GetMiddleHandsPos(ent, climb_from);
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_from[2] += ent->character->climb_r + engine_frame_time * ent->speed[2];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->Height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);  //global_offset, &next_fc, ent->character->Height);
                if(climb->edge_hit && climb->can_hang)
                {
                    vec3_copy(climb->point, climb->edge_point);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    vec3_set_zero(ent->speed);
                }

                // If Lara is moving backwards off the ledge we want to move Lara slightly forwards
                // depending on the current angle.
                if((ent->dir_flag == ENT_MOVE_BACKWARD) && (ent->move_type == MOVE_CLIMBING))
                {
                    pos[0] = climb->point[0] - ent->transform[4 + 0] * (ent->character->forvard_size + 16.0);
                    pos[1] = climb->point[1] - ent->transform[4 + 1] * (ent->character->forvard_size + 16.0);
                }
            }

            if(((ent->move_type != MOVE_ON_FLOOR)) && clean_action && (curr_fc->ceiling_climb) && (curr_fc->ceiling_hit.hit) && (pos[2] + ent->bf->bb_max[2] > curr_fc->ceiling_hit.point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onEndFrame = ent_to_monkey_swing;
                break;
            }
            if(((resp->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR)) && (!clean_action || (climb->can_hang == 0)))
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                       // middle landing
                break;
            }

            if((ent->speed[2] < -FREE_FALL_SPEED_2))
            {
                ent->move_type = MOVE_FREE_FALLING;
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
                break;
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->speed);
                ss_anim->next_state = TR_STATE_LARA_HANG;
                ss_anim->onEndFrame = ent_to_edge_climb;
#if OSCILLATE_HANG_USE
                vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
                if(Entity_CheckNextPenetration(ent, cmd, move) == 0)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    ent_to_edge_climb(ent);
                }
#endif
            }
            break;

            /*other code here prevents to UGLY Lara's move in end of "climb on", do not loose ent_set_on_floor_after_climb callback here!*/
        case TR_STATE_LARA_HANDSTAND:
        case TR_STATE_LARA_GRABBING:
        case TR_STATE_LARA_CLIMB_TO_CRAWL:
            cmd->rot[0] = 0;
            ent->no_fix_all = 0x01;
            ss_anim->onEndFrame = ent_set_on_floor_after_climb;
            break;

        case TR_STATE_LARA_HANG:
            cmd->rot[0] = 0;

            if(ent->move_type == MOVE_WALLS_CLIMB)
            {
                if(clean_action)
                {
                    if((climb->wall_hit == 0x02) && (cmd->move[0] == 0) && (cmd->move[1] == 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
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
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_LEFT, 0); // edge climb left
                    }
                    else if(climb->wall_hit == 0x00)
                    {
                        ent->move_type = MOVE_FREE_FALLING;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    }
                    else
                    {
                        ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;       // disable shake
                    }
                }
                else
                {
                    ent->move_type = MOVE_FREE_FALLING;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                }
                break;
            }

            if((resp->kill == 0) && clean_action)                               // we have to update climb point every time so entity can move
            {
                //t = LARA_TRY_HANG_WALL_OFFSET;
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                Character_GetMiddleHandsPos(ent, climb_from);
                if(climb->edge_hit && (climb->edge_point[2] > climb_from[2]))
                {
                    climb_from[2] = climb->edge_point[2];
                }
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->can_hang)
                {
                    vec3_copy(climb->point, climb->edge_point);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
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
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf->bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    if(climb->edge_hit && (climb->next_z_space >= 512.0) && ((climb->next_z_space < ent->character->Height - LARA_HANG_VERTICAL_EPSILON) || (cmd->crouch == 1)))
                    {
                        vec3_copy(climb->point, climb->edge_point);
                        ss_anim->next_state = TR_STATE_LARA_CLIMB_TO_CRAWL;     // crawlspace climb
                    }
                    else if(climb->edge_hit && (climb->next_z_space >= ent->character->Height - LARA_HANG_VERTICAL_EPSILON))
                    {
                        vec3_copy(climb->point, climb->edge_point);
                        ss_anim->next_state = (cmd->shift)?(TR_STATE_LARA_HANDSTAND):(TR_STATE_LARA_GRABBING);               // climb up
                    }
                    else
                    {
                        vec3_set_zero(ent->speed);
                        ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;       // disable shake
                    }
                }
                else if(cmd->move[0] ==-1)                                      // check walls climbing
                {
                    Character_CheckWallsClimbability(ent, climb);
                    if(climb->wall_hit)
                    {
                        ent->move_type = MOVE_WALLS_CLIMB;
                    }
                    ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;           // disable shake
                }
                else if(cmd->move[1] ==-1)
                {
                    vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        ent->move_type = ENT_MOVE_LEFT;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_LEFT, 0);
                    }
                    else
                    {
                        ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;       // disable shake
                    }
                }
                else if(cmd->move[1] == 1)
                {
                    vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                    if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        ent->move_type = ENT_MOVE_RIGHT;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_CLIMB_RIGHT, 0);
                    }
                    else
                    {
                        ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;       // disable shake
                    }
                }
                else
                {
                    ss_anim->anim_frame_flags = ANIM_LOOP_LAST_FRAME;           // disable shake
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf->bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed);
                }
            }
            else if(clean_action && (curr_fc->ceiling_climb) && (curr_fc->ceiling_hit.hit) && (pos[2] + ent->bf->bb_max[2] > curr_fc->ceiling_hit.point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onEndFrame = ent_to_monkey_swing;
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_STATE_LARA_LADDER_IDLE:
            cmd->rot[0] = 0;
            ent->move_type = MOVE_WALLS_CLIMB;
            ent->dir_flag = ENT_STAY;
            ent->character->cam_follow_center = 64;
            if(ent->move_type == MOVE_CLIMBING)
            {
                ss_anim->next_state = TR_STATE_LARA_GRABBING;
                break;
            }
            if(!clean_action)
            {
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
                ent->dir_flag = ENT_MOVE_BACKWARD;
            }
            else if(cmd->move[0] == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET;
                Character_GetMiddleHandsPos(ent, climb_from);
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->edge_hit && (climb->next_z_space >= 512.0))
                {
                    ent->move_type = MOVE_CLIMBING;
                    ss_anim->next_state = TR_STATE_LARA_GRABBING;
                }
                else if((!curr_fc->ceiling_hit.hit) || (pos[2] + ent->bf->bb_max[2] < curr_fc->ceiling_hit.point[2]))
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_UP;
                }
            }
            else if(cmd->move[0] == -1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_DOWN;
            }
            else if(cmd->move[1] == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_RIGHT;
            }
            else if(cmd->move[1] == -1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_LEFT;
            }
            break;

        case TR_STATE_LARA_LADDER_LEFT:
            ent->dir_flag = ENT_MOVE_LEFT;
            if(!clean_action || (ent->character->climb.wall_hit == 0))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_RIGHT:
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(!clean_action || (ent->character->climb.wall_hit == 0))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_UP:
            ent->character->cam_follow_center = 64;
            if(ent->move_type == MOVE_CLIMBING)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                break;
            }

            if(clean_action && ent->character->climb.wall_hit)
            {
                t = LARA_TRY_HANG_WALL_OFFSET;
                Character_GetMiddleHandsPos(ent, climb_from);
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->edge_hit && (climb->next_z_space >= 512.0))
                {
                    ent->move_type = MOVE_CLIMBING;
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
                else if((cmd->move[0] <= 0) && (curr_fc->ceiling_hit.hit || (pos[2] + ent->bf->bb_max[2] >= curr_fc->ceiling_hit.point[2])))
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }

                if(curr_fc->ceiling_hit.hit && (pos[2] + ent->bf->bb_max[2] > curr_fc->ceiling_hit.point[2]))
                {
                    pos[2] = curr_fc->ceiling_hit.point[2] - ent->bf->bb_max[2];
                }
            }
            else
            {
                // Free fall after stop
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_DOWN:
            ent->character->cam_follow_center = 64;
            if(clean_action && ent->character->climb.wall_hit && (cmd->move[1] < 0))
            {
                if(ent->character->climb.wall_hit != 0x02)
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_SHIMMY_LEFT:
            ent->no_fix_skeletal_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3 | BODY_PART_HANDS_3;

            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_LEFT;
            if(!clean_action)
            {
                vec3_set_zero(ent->speed);
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(ent->move_type == MOVE_WALLS_CLIMB)
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
                Character_GetMiddleHandsPos(ent, climb_from);
                if(climb->edge_hit && (climb->edge_point[2] > climb_from[2]))
                {
                    climb_from[2] = climb->edge_point[2];
                }
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf->bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed);
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
                vec3_mul_scalar(move, ent->transform + 0, -PENETRATION_TEST_OFFSET);
                if((Entity_CheckNextPenetration(ent, move) > 0) && (resp->horizontal_collide != 0x00))
                {
                    ss_anim->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_SHIMMY_RIGHT:
            ent->no_fix_skeletal_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3 | BODY_PART_HANDS_3;

            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_RIGHT;
            if(!clean_action)
            {
                vec3_set_zero(ent->speed);
                ent->move_type = MOVE_FREE_FALLING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(ent->move_type == MOVE_WALLS_CLIMB)
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
                Character_GetMiddleHandsPos(ent, climb_from);
                if(climb->edge_hit && (climb->edge_point[2] > climb_from[2]))
                {
                    climb_from[2] = climb->edge_point[2];
                }
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                if(climb->edge_hit)
                {
                    vec3_copy(climb->point, climb->edge_point);
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
                    ent->move_type = MOVE_CLIMBING;                             // hang on
                    pos[0] = climb->point[0] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 0];
                    pos[1] = climb->point[1] - (LARA_HANG_WALL_DISTANCE) * ent->transform[4 + 1];
                    pos[2] = climb->point[2] - ent->bf->bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    vec3_set_zero(ent->speed);
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
                vec3_mul_scalar(move, ent->transform + 0, PENETRATION_TEST_OFFSET);
                if((Entity_CheckNextPenetration(ent, move) > 0) && (resp->horizontal_collide != 0x00))
                {
                    ss_anim->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_ONWATER_EXIT:
            cmd->rot[0] = 0;
            ent->no_fix_all = 0x01;
            ss_anim->onEndFrame = ent_set_on_floor_after_climb;
            break;

        case TR_STATE_LARA_JUMP_FORWARD:
        case TR_STATE_LARA_FALL_BACKWARD:
            ent->no_fix_skeletal_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            Character_Lean(ent, cmd, 4.0);

            if((resp->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(ent->self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else if(!clean_action && (cmd->move[0] == 1) && (cmd->crouch == 0))
                {
                    ent->move_type = MOVE_ON_FLOOR;
                    ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            else if(ent->move_type == MOVE_UNDERWATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_UpdateTransform(ent);
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_SMASH_JUMP, 0);
                ent->dir_flag = ENT_MOVE_BACKWARD;
                Character_UpdateCurrentSpeed(ent, 1);
            }
            else if(ent->speed[2] <= -FREE_FALL_SPEED_2)
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(clean_action)
            {
                ss_anim->next_state = TR_STATE_LARA_REACH;
            }
            else if(cmd->shift == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_SWANDIVE_BEGIN;              // fly like fish
            }
            else if(ent->speed[2] <= -FREE_FALL_SPEED_2)
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(cmd->roll)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * FREE FALL TO UNDERWATER CASES
             */
        case TR_STATE_LARA_UNDERWATER_DIVING:
            ent->angles[1] = -45.0;
            cmd->rot[1] = 0.0;
            Entity_UpdateTransform(ent);
            ss_anim->onEndFrame = ent_correct_diving_angle;
            break;

        case TR_STATE_LARA_FREEFALL:
            Character_Lean(ent, cmd, 1.0);

            if( (int(ent->speed[2]) <=  -FREE_FALL_SPEED_CRITICAL) &&
                (int(ent->speed[2]) >= (-FREE_FALL_SPEED_CRITICAL-100)) )
            {
                ent->speed[2] = -FREE_FALL_SPEED_CRITICAL-101;
                Audio_Send(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->id);       // Scream
            }
            else if(ent->speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
            {
                //Reset these to zero so Lara is only falling downwards
                ent->speed[0] = 0.0;
                ent->speed[1] = 0.0;
            }

            if(ent->move_type == MOVE_UNDERWATER)
            {
                ent->angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                Entity_UpdateTransform(ent);                                     // needed here to fix underwater in wall collision bug
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->id);       // Stop scream

                // Splash sound is hardcoded, beginning with TR3.
                if(World_GetVersion() > TR_II)
                {
                    Audio_Send(TR_AUDIO_SOUND_SPLASH, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
            }
            else if((resp->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(ent->self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_STAY_IDLE, 0);
                    Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
                else if(ent->speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
                {
                    if(!Character_ChangeParam(ent, PARAM_HEALTH, (ent->speed[2] + FREE_FALL_SPEED_MAXSAFE) / 2))
                    {
                        resp->kill = 1;
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_DEATH, 0);
                        Engine_JoyRumble(200.0, 500);
                    }
                    else
                    {
                        Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                    }
                }
                else if(ent->speed[2] <= -FREE_FALL_SPEED_2)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
                else
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
                }

                if(resp->kill == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_DEATH;
                    Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, ent->id);
                }
            }
            else if(clean_action)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_REACH;
            }
            break;

        case TR_STATE_LARA_SWANDIVE_BEGIN:
            ent->character->rotate_speed_mult = 0.4f;
            if(resp->vertical_collide & 0x01 || ent->move_type == MOVE_ON_FLOOR)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing - roll
            }
            else if(ent->move_type == MOVE_UNDERWATER)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_SWANDIVE_END;                // next stage
            }
            break;

        case TR_STATE_LARA_SWANDIVE_END:
            cmd->rot[0] = 0;

            //Reset these to zero so Lara is only falling downwards
            ent->speed[0] = 0.0;
            ent->speed[1] = 0.0;

            if((resp->vertical_collide & 0x01) || (ent->move_type == MOVE_ON_FLOOR))
            {
                if(curr_fc->quicksand)
                {
                    resp->kill = 1;
                    Character_SetParam(ent, PARAM_HEALTH, 0.0);
                    Character_SetParam(ent, PARAM_AIR, 0.0);
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_LANDING_DEATH, -1);
                }
                else
                {
                    Character_SetParam(ent, PARAM_HEALTH, 0.0);
                    ss_anim->next_state = TR_STATE_LARA_DEATH;
                }
            }
            else if(ent->move_type == MOVE_UNDERWATER)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * WATER ANIMATIONS
             */
        case TR_STATE_LARA_UNDERWATER_STOP:
            if(ent->move_type != MOVE_UNDERWATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            break;

        case TR_STATE_LARA_WATER_DEATH:
            if(ent->move_type != MOVE_ON_WATER)
            {
                pos[2] += (TR_METERING_SECTORSIZE / 4) * engine_frame_time;     // go to the air
            }
            break;


        case TR_STATE_LARA_UNDERWATER_FORWARD:
            if(ent->move_type != MOVE_UNDERWATER && ent->move_type != MOVE_ON_WATER)
            {
                Entity_SetAnimation(ent, 0, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(curr_fc->floor_hit.hit && curr_fc->water && (curr_fc->transition_level - curr_fc->floor_hit.point[2] <= ent->character->max_step_up_height))
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_TO_WADE, 0); // go to the air
                ss_anim->next_state = TR_STATE_LARA_STOP;
                vec3_copy(ent->character->climb.point, curr_fc->floor_hit.point);   ///@FIXME: without it Lara are pulled high up, but this string was not been here.
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
                    ent->linear_speed = 0.0f;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_INERTIA;
            }
            break;

        case TR_STATE_LARA_UNDERWATER_INERTIA:
            if(ent->move_type == MOVE_ON_WATER)
            {
                ent->linear_speed = 0.0f;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_STOP:
            if(clean_action && (cmd->move[0] == 1) && (ent->move_type != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                Character_GetMiddleHandsPos(ent, climb_from);
                climb_from[0] -= ent->character->climb_r * ent->transform[4 + 0];
                climb_from[1] -= ent->character->climb_r * ent->transform[4 + 1];
                climb_from[2] = curr_fc->transition_level;
                climb_to[0] = climb_from[0] + t * ent->transform[4 + 0];
                climb_to[1] = climb_from[1] + t * ent->transform[4 + 1];
                climb_to[2] = climb_from[2] - ent->character->max_step_up_height;
                Character_CheckClimbability(ent, climb, climb_from, climb_to);
                low_vertical_space = climb->edge_hit && climb->edge_point[2] <= curr_fc->transition_level;
                if(climb->edge_hit && (climb->next_z_space >= ent->character->Height - LARA_HANG_VERTICAL_EPSILON))// && (climb->edge_point.m_floats[2] - pos[2] < ent->character->max_step_up_height))   // max_step_up_height is not correct value here
                {
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_CLIMBING;
                    ent->no_fix_all = 0x01;
                    ent->angles[0] = climb->edge_z_ang;
                    Entity_UpdateTransform(ent);
                    vec3_copy(climb->point, climb->edge_point);
                }
            }

            if(ent->move_type == MOVE_CLIMBING)
            {
                vec3_set_zero(ent->speed);
                cmd->rot[0] = 0;
                ent->no_fix_all = 0x01;
                if(low_vertical_space)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW, 0);
                    ent_climb_out_of_water(ent, ss_anim, 0x02);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                    ss_anim->onEndFrame = ent_climb_out_of_water;
                }
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if((cmd->move[0] == 1) || (cmd->jump == 1))                    // dive works correct only after TR_STATE_LARA_ONWATER_FORWARD
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_ONWATER_FORWARD;
            }
            else if(cmd->move[0] ==-1)
            {
                ent->dir_flag = ENT_MOVE_BACKWARD;
                ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
            }
            else if(cmd->move[1] ==-1)
            {
                if(cmd->shift == 1)
                {
                    ent->dir_flag = ENT_MOVE_LEFT;
                    cmd->rot[0] = 0;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
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
                    cmd->rot[0] = 0;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(ent->move_type == MOVE_UNDERWATER)
            {
                ent->move_type = MOVE_ON_WATER;
            }
            break;

        case TR_STATE_LARA_ONWATER_FORWARD:
            ent->move_type = MOVE_ON_WATER;
            if(resp->kill)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->jump == 1)
            {
                t = pos[2];
                Character_GetHeightInfo(pos, &next_fc);
                pos[2] = t;
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
                ss_anim->onEndFrame = ent_set_underwater;                          // dive
            }
            else if((cmd->move[0] == 1) && !clean_action)
            {
                if(!curr_fc->floor_hit.hit || (pos[2] - ent->character->Height > curr_fc->floor_hit.point[2]- ent->character->swim_depth))
                {
                    //ent->last_state = ent->last_state;                          // swim forward
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                    ss_anim->onEndFrame = ent_set_on_floor;                     // to wade
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_BACK:
            if(cmd->move[0] == -1 && cmd->jump == 0)
            {
                if(!curr_fc->floor_hit.hit || (curr_fc->floor_hit.point[2] + ent->character->Height < curr_fc->transition_level))
                {
                    //ent->current_state = TR_STATE_CURRENT;                      // continue swimming
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_LEFT:
            cmd->rot[0] = 0;
            if(cmd->jump == 0)
            {
                if(cmd->move[1] ==-1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit.hit || (pos[2] - ent->character->Height > curr_fc->floor_hit.point[2]))
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
                    }
                    else
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_WALK_LEFT;
                        ss_anim->onEndFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

        case TR_STATE_LARA_ONWATER_RIGHT:
            cmd->rot[0] = 0;
            if(cmd->jump == 0)
            {
                if(cmd->move[1] == 1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit.hit || (pos[2] - ent->character->Height > curr_fc->floor_hit.point[2]))
                    {
                        // swim RIGHT
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                    }
                    else
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_WALK_RIGHT;
                        ss_anim->onEndFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

            /*
             * CROUCH SECTION
             */
        case TR_STATE_LARA_CROUCH_IDLE:
            ent->dir_flag = ENT_MOVE_FORWARD;
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            move[0] = pos[0];
            move[1] = pos[1];
            move[2] = pos[2] + 0.5 * (ent->bf->bb_max[2] - ent->bf->bb_min[2]);
            Character_GetHeightInfo(move, &next_fc);

            Character_Lean(ent, cmd, 0.0);

            if((cmd->crouch == 0) && !low_vertical_space)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // Back to stand
            }
            else if((cmd->move[0] != 0) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE;                  // Both forward & back provoke crawl stage
            }
            else if(cmd->jump == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_ROLL;                 // Crouch roll
            }
            else
            {
                if(World_GetVersion() > TR_III)
                {
                    if(cmd->move[1] == 1)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_RIGHT;
                    }
                    else if(cmd->move[1] == -1)
                    {
                        ent->dir_flag = ENT_MOVE_FORWARD;
                        ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_LEFT;
                    }
                }
                else
                {
                    cmd->rot[0] = 0;
                }
            }
            break;

        case TR_STATE_LARA_CROUCH_ROLL:
        case TR_STATE_LARA_SPRINT_ROLL:
            cmd->rot[0] = 0;
            Character_Lean(ent, cmd, 0.0);
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }

            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            if((Entity_CheckNextPenetration(ent, move) > 0) && (resp->horizontal_collide != 0x00))  // Smash into wall
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_CRAWL_IDLE:
            ent->dir_flag = ENT_MOVE_FORWARD;
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            if(resp->kill == 1)
            {
                ent->dir_flag = ENT_STAY;
                ss_anim->next_state = TR_STATE_LARA_DEATH;
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
                if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                {
                    vec3_mul_scalar(global_offset, ent->transform + 4, CRAWL_FORWARD_OFFSET);
                    global_offset[2] += 0.5 * (ent->bf->bb_max[2] + ent->bf->bb_min[2]);
                    vec3_add(global_offset, global_offset, pos);
                    Character_GetHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_hit.point[2] < pos[2] + ent->character->min_step_up_height) &&
                       (next_fc.floor_hit.point[2] > pos[2] - ent->character->min_step_up_height))
                    {
                        ss_anim->next_state = TR_STATE_LARA_CRAWL_FORWARD;           // In TR4+, first state is crawlspace jump
                    }
                }
            }
            else if(cmd->move[0] == -1)
            {
                vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
                if((Entity_CheckNextPenetration(ent, move) == 0) || (resp->horizontal_collide == 0x00))
                {
                    vec3_mul_scalar(global_offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
                    global_offset[2] += 0.5 * (ent->bf->bb_max[2] + ent->bf->bb_min[2]);
                    vec3_add(global_offset, global_offset, pos);
                    Character_GetHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_hit.point[2] < pos[2] + ent->character->min_step_up_height) &&
                       (next_fc.floor_hit.point[2] > pos[2] - ent->character->min_step_up_height))
                    {
                        ent->dir_flag = ENT_MOVE_BACKWARD;
                        ss_anim->next_state = TR_STATE_LARA_CRAWL_BACK;
                    }
                    else if(clean_action && (resp->horizontal_collide == 0) &&
                       (next_fc.floor_hit.point[2] < pos[2] - ent->character->Height))
                    {
                        climb_from[0] = pos[0] + ent->transform[4 + 0] * ent->bf->bb_min[1];
                        climb_from[1] = pos[1] + ent->transform[4 + 1] * ent->bf->bb_min[1];
                        climb_from[2] = pos[2];
                        climb_to[0] = pos[0];
                        climb_to[1] = pos[1];
                        climb_to[2] = pos[2] - ent->character->max_step_up_height;
                        Character_CheckClimbability(ent, climb, climb_from, climb_to);
                        if(climb->can_hang)
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            ent->dir_flag = ENT_MOVE_BACKWARD;
                            ent->move_type = MOVE_CLIMBING;
                            vec3_copy(climb->point, climb->edge_point);
                            ss_anim->next_state = TR_STATE_LARA_CRAWL_TO_CLIMB;
                        }
                    }
                }
            }
            else if(!cmd->crouch)
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;                // Back to crouch.
            }
            break;

        case TR_STATE_LARA_CRAWL_TO_CLIMB:
            ent->no_fix_all = 0x01;
            ss_anim->onEndFrame = ent_crawl_to_climb;
            break;

        case TR_STATE_LARA_CRAWL_FORWARD:
            ent->dir_flag = ENT_MOVE_FORWARD;
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            ent->character->rotate_speed_mult = 0.5f;
            vec3_mul_scalar(move, ent->transform + 4, PENETRATION_TEST_OFFSET);
            if((Entity_CheckNextPenetration(ent, move) > 0) && (resp->horizontal_collide != 0x00))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            vec3_mul_scalar(global_offset, ent->transform + 4, CRAWL_FORWARD_OFFSET);
            global_offset[2] += 0.5 * (ent->bf->bb_max[2] + ent->bf->bb_min[2]);
            vec3_add(global_offset, global_offset, pos);
            Character_GetHeightInfo(global_offset, &next_fc);

            if((cmd->move[0] != 1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (next_fc.floor_hit.point[2] >= pos[2] + ent->character->min_step_up_height) ||
                     (next_fc.floor_hit.point[2] <= pos[2] - ent->character->min_step_up_height)  )
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_BACK:
            ent->dir_flag = ENT_MOVE_FORWARD;   // Absurd? No, Core Design.
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            ent->character->rotate_speed_mult = 0.5f;
            vec3_mul_scalar(move, ent->transform + 4, -PENETRATION_TEST_OFFSET);
            if((Entity_CheckNextPenetration(ent, move) > 0) && (resp->horizontal_collide != 0x00))
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            vec3_mul_scalar(global_offset, ent->transform + 4, -CRAWL_FORWARD_OFFSET);
            global_offset[2] += 0.5 * (ent->bf->bb_max[2] + ent->bf->bb_min[2]);
            vec3_add(global_offset, global_offset, pos);
            Character_GetHeightInfo(global_offset, &next_fc);
            if((cmd->move[0] != -1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (next_fc.floor_hit.point[2] >= pos[2] + ent->character->min_step_up_height)   ||
                     (next_fc.floor_hit.point[2] <= pos[2] - ent->character->min_step_up_height)    )
            {
                ent->dir_flag = ENT_STAY;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_LEFT:
            ent->dir_flag = ENT_MOVE_FORWARD;
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            ent->character->rotate_speed_mult = ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 14)) ? (1.0f) : (0.0f);

            if((cmd->move[1] != -1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_RIGHT:
            ent->dir_flag = ENT_MOVE_FORWARD;
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            ent->character->rotate_speed_mult = ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 14)) ? (1.0f) : (0.0f);

            if((cmd->move[1] != 1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CROUCH_TURN_LEFT:
        case TR_STATE_LARA_CROUCH_TURN_RIGHT:
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            ent->character->rotate_speed_mult = ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 23)) ? (0.6f) : (0.0f);

            if((cmd->move[1] == 0) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            break;

            /*
             * CLIMB MONKEY
             */
        case TR_STATE_LARA_MONKEYSWING_IDLE:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_STAY;
            ///@FIXME: stick for TR_III+ monkey swing fix... something wrong with anim 150
            if(clean_action && (ent->move_type != MOVE_MONKEYSWING) && (curr_fc->ceiling_climb) && (curr_fc->ceiling_hit.hit) && (pos[2] + ent->bf->bb_max[2] > curr_fc->ceiling_hit.point[2] - 96.0))
            {
                ent->move_type = MOVE_MONKEYSWING;
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_MONKEY_IDLE, 0);
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
            }

            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->shift && (cmd->move[1] ==-1))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_LEFT;
            }
            else if(cmd->shift && (cmd->move[1] == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_RIGHT;
            }
            else if(cmd->move[0] == 1)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_FORWARD;
            }
            else if(cmd->move[1] ==-1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_LEFT;
            }
            else if(cmd->move[1] == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_RIGHT;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_LEFT:
            ent->character->rotate_speed_mult = 0.5f;
            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != -1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_RIGHT:
            ent->character->rotate_speed_mult = 0.5f;
            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->dir_flag = ENT_STAY;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_FORWARD:
            ent->character->rotate_speed_mult = 0.45f;
            ent->dir_flag = ENT_MOVE_FORWARD;

            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_LEFT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_LEFT;

            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_RIGHT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_RIGHT;

            if((ent->move_type != MOVE_MONKEYSWING) || !clean_action)
            {
                Entity_SetAnimation(ent, TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

            /*
             * intermediate animations are processed automatically.
             */
        default:
            cmd->rot[0] = 0;
            if((ent->move_type == MOVE_MONKEYSWING) || (ent->move_type == MOVE_WALLS_CLIMB))
            {
                if(!clean_action)
                {
                    Entity_SetAnimation(ent, TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    ent->dir_flag = ENT_STAY;
                    ent->move_type = MOVE_FREE_FALLING;
                }
            }
            break;
    };

    /*
     * additional animations control
     */
    switch(ss_anim->current_animation)
    {
        case TR_ANIMATION_LARA_STAY_JUMP_SIDES:
            ent->no_fix_skeletal_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3 | BODY_PART_HEAD;
            break;

        case TR_ANIMATION_LARA_TRY_HANG_SOLID:
        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG:
            if((ent->move_type == MOVE_FREE_FALLING) && (ent->character->cmd.action) &&
               (ent->speed[0] * ent->transform[4+0] + ent->speed[1] * ent->transform[4+1] < 0.0))
            {
                ent->speed[0] = -ent->speed[0];
                ent->speed[1] = -ent->speed[1];
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
