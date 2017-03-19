
#include <stdlib.h>
#include <stdio.h>

#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"

#include "../render/camera.h"
#include "../script/script.h"
#include "../vt/tr_versions.h"
#include "../engine.h"
#include "../audio.h"
#include "../controls.h"
#include "../physics.h"
#include "../room.h"
#include "../world.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "state_control_Natla.h"
#include "state_control.h"


void StateControl_NatlaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            if(ent->move_type == MOVE_FLY)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_NATLA_FLY, 0);
            }
            else if(ent->move_type == MOVE_FREE_FALLING)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_NATLA_DROP, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_NATLA_STAY, 0);
            }
            break;

        case ANIMATION_KEY_DEAD:
            if(ent->move_type == MOVE_ON_FLOOR)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_NATLA_DEAD, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_NATLA_DROP, 0);
            }
            break;
    }
}


int StateControl_Natla(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);
    height_info_p hi = &ent->character->height_info;
    float *pos = ent->transform + 12;

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;

    switch(current_state)
    {
        case TR_STATE_NATLA_STAY:
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_NATLA_DROPPED;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_NATLA_STAY_AIM;
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_NATLA_FLY;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_STAY;
            }
            break;

        case TR_STATE_NATLA_STAY_AIM: // -> 1 -> 5 -> 6
            ent->move_type = MOVE_ON_FLOOR;
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_NATLA_DROPPED;
            }
            if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_NATLA_STAY_SHOOT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_STAY;
            }
            break;

        case TR_STATE_NATLA_STAY_SHOOT: // -> 5
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_NATLA_DROPPED;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_STAY_SHOOT;
            }
            break;

        case TR_STATE_NATLA_RUN: // -> 8 -> 9
            ent->dir_flag = ENT_MOVE_FORWARD;
        case TR_STATE_NATLA_STAND_UP: // -> 3 -> 9
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_NATLA_DEAD;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->next_state = TR_STATE_NATLA_RUN;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_STAND_UP;
            }
            break;

        case TR_STATE_NATLA_FALL: // -> 5
            cmd->rot[0] = 0;
            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ss_anim->next_state = TR_STATE_NATLA_DROPPED;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_FALL;
            }
            break;

        case TR_STATE_NATLA_DROPPED: // -> 8
            cmd->rot[0] = 0;
            if(cmd->action || (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_NATLA_STAND_UP;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_DROPPED;
            }
            break;

        case TR_STATE_NATLA_FLY: // -> 1 -> 5 -> 7
            ent->move_type = MOVE_FLY;
            if(state->dead)
            {
                ent->move_type = MOVE_FREE_FALLING;
                ss_anim->next_state = (ss_anim->current_animation == TR_ANIMATION_NATLA_FLY) ? TR_STATE_NATLA_FALL : TR_STATE_NATLA_DROPPED;
            }
            else if((ent->move_type != MOVE_FLY) || cmd->crouch && hi->floor_hit.hit && (pos[2] < hi->floor_hit.point[2] + 256.0f))
            {
                ent->move_type = MOVE_ON_FLOOR;
                ss_anim->next_state = TR_STATE_NATLA_STAY;
            }
            else
            {
                ss_anim->next_state = TR_STATE_NATLA_FLY;
            }
            break;
    };

    if(state->slide == CHARACTER_SLIDE_BACK)
    {
        ent->dir_flag = ENT_MOVE_BACKWARD;
        ent->anim_linear_speed = 64;
    }
    else if(state->slide == CHARACTER_SLIDE_FRONT)
    {
        ent->dir_flag = ENT_MOVE_FORWARD;
        ent->anim_linear_speed = 64;
    }

    return 0;
}
