
#include <stdlib.h>
#include <stdio.h>

#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"

#include "../render/camera.h"
#include "../script/script.h"
#include "../vt/tr_versions.h"
#include "../engine.h"
#include "../controls.h"
#include "../room.h"
#include "../world.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "state_control_gorilla.h"
#include "state_control.h"


void ent_gorilla_fix_strafe(entity_p ent, ss_animation_p ss_anim)
{
    if(ss_anim->frame_changing_state >= 0x02)
    {
        uint16_t curr_st = ss_anim->model->animations[ss_anim->current_animation].state_id;
        uint16_t next_st = ss_anim->model->animations[ss_anim->next_animation].state_id;
        if((curr_st == TR_STATE_GORILLA_STRAFE_LEFT) && (next_st == TR_STATE_GORILLA_STAY))
        {
            ss_anim->current_animation = ss_anim->next_animation;
            ss_anim->current_frame = ss_anim->next_frame;
            ent->angles[0] -= 90.0f;
            Entity_UpdateTransform(ent);
        }
        else if((curr_st == TR_STATE_GORILLA_STRAFE_RIGHT) && (next_st == TR_STATE_GORILLA_STAY))
        {
            ss_anim->current_animation = ss_anim->next_animation;
            ss_anim->current_frame = ss_anim->next_frame;
            ent->angles[0] += 90.0f;
            Entity_UpdateTransform(ent);
        }
        else if((curr_st == TR_STATE_GORILLA_STAY) && (next_st == TR_STATE_GORILLA_STRAFE_LEFT))
        {
            ss_anim->current_animation = ss_anim->next_animation;
            ss_anim->current_frame = ss_anim->next_frame;
            ent->angles[0] += 90.0f;
            Entity_UpdateTransform(ent);
        }
        else if((curr_st == TR_STATE_GORILLA_STAY) && (next_st == TR_STATE_GORILLA_STRAFE_RIGHT))
        {
            ss_anim->current_animation = ss_anim->next_animation;
            ss_anim->current_frame = ss_anim->next_frame;
            ent->angles[0] -= 90.0f;
            Entity_UpdateTransform(ent);
        }
    }
}


void StateControl_GorillaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_GORILLA_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            if(current_state == TR_STATE_GORILLA_STAY)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_GORILLA_DEAD1, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_GORILLA_DEAD2, 0);
            }
            break;
    }
}


int StateControl_Gorilla(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);
    float *pos = ent->transform + 12;

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;
    ss_anim->onEndFrame = ent_gorilla_fix_strafe;
    state->sprint = 0x00;
    state->crouch = 0x00;
    state->attack = 0x00;

    if(ent->move_type == MOVE_CLIMBING)
    {
        ent->move_type = MOVE_ON_FLOOR;
    }

    switch(current_state)
    {
        case TR_STATE_GORILLA_STAY: // -> 3 -> 4 -> 6 -> 7 -> 8 -> 9 -> 10
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_GORILLA_DEAD1, 0, NULL);
            }
            else if(cmd->action && (cmd->move[0] == 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_STAY_ATTACK;
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_GORILLA_FORWARD_ATTACK;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_GORILLA_ARRRR;
            }
            else if(cmd->move[0] > 0)
            {
                if(cmd->shift)
                {
                    climb_info_t *climb = &ent->character->climb;
                    height_info_p curr_fc = &ent->character->height_info;
                    float t = ent->character->forvard_size + 320;
                    float climb_from[3], climb_to[3];
                    climb_from[0] = pos[0];
                    climb_from[1] = pos[1];
                    climb_from[2] = pos[2] + 1024.0f;
                    climb_to[0] = pos[0] + t * ent->transform[4 + 0];
                    climb_to[1] = pos[1] + t * ent->transform[4 + 1];
                    climb_to[2] = pos[2] + ent->character->max_step_up_height;
                    if(curr_fc->ceiling_hit.hit && (climb_from[2] >= curr_fc->ceiling_hit.point[2] - ent->character->climb_r))
                    {
                        climb_from[2] = curr_fc->ceiling_hit.point[2] - ent->character->climb_r;
                    }
                    Character_CheckClimbability(ent, climb, climb_from, climb_to);
                    if(  climb->edge_hit                                                                &&
                        (climb->next_z_space >= ent->character->height)    &&
                        (pos[2] + ent->character->max_step_up_height < climb->edge_point[2]))
                    {
                        if(pos[2] + 800.0f >= climb->edge_point[2])
                        {
                            ent->angles[0] = climb->edge_z_ang;
                            Entity_UpdateTransform(ent);
                            vec3_copy(climb->point, climb->edge_point);
                            Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_GORILLA_CLIMB, 0);
                            ent->no_fix_all = 0x01;
                            pos[0] = climb->point[0] - ent->transform[4 + 0] * (80.0f);
                            pos[1] = climb->point[1] - ent->transform[4 + 1] * (80.0f);
                            pos[2] = climb->edge_point[2] - 512.0;
                            Entity_UpdateRigidBody(ent, 1);
                            break;
                        }
                    }
                }
                ss_anim->next_state = TR_STATE_GORILLA_RUN;
            }
            else if(cmd->shift && (cmd->move[1] > 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_STRAFE_RIGHT;
            }
            else if(cmd->shift && (cmd->move[1] < 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_STRAFE_LEFT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_GORILLA_STAY;
            }
            break;

        case TR_STATE_GORILLA_STRAFE_RIGHT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_STRAFE_RIGHT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_GORILLA_STAY;
            }
            break;

        case TR_STATE_GORILLA_STRAFE_LEFT:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] < 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_STRAFE_LEFT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_GORILLA_STAY;
            }
            break;

        case TR_STATE_GORILLA_RUN: // -> 1
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_GORILLA_DEAD2, 0, NULL);
            }
            else if(!cmd->action && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_GORILLA_RUN;
            }
            else
            {
                ss_anim->next_state = TR_STATE_GORILLA_STAY;
            }
            break;

        case TR_STATE_GORILLA_CLIMB:
            cmd->rot[0] = 0;
            ent->move_type = MOVE_CLIMBING;
            break;

        case TR_STATE_GORILLA_FORWARD_ATTACK:
        case TR_STATE_GORILLA_STAY_ATTACK:
            state->attack = 0x01;
            break;
            
        case TR_STATE_GORILLA_DEAD:
            state->dead = 0x02;
            break;
    };

    if(ent->character->state.slide == CHARACTER_SLIDE_BACK)
    {
        ent->dir_flag = ENT_MOVE_BACKWARD;
        ent->anim_linear_speed = 64;
    }
    else if(ent->character->state.slide == CHARACTER_SLIDE_FRONT)
    {
        ent->dir_flag = ENT_MOVE_FORWARD;
        ent->anim_linear_speed = 64;
    }

    return 0;
}
