
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
#include "../room.h"
#include "../world.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "state_control_bear.h"
#include "state_control.h"


void StateControl_BearSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_BEAR_STAY, 0);
            break;
    }
}


int StateControl_Bear(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ent->dir_flag = ENT_STAY;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;
    state->attack = 0x00;

    switch(current_state)
    {
        case TR_STATE_BEAR_STAY:    // -> 0 -> 3 -> 4 -> 5 -> 8 -> 9
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_BEAR_DEAD;
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_BEAR_WALK) : (TR_STATE_BEAR_RUN);
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY_HIGH;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_BEAR_ATTACK;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_BEAR_EAT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY;
            }
            break;

        case TR_STATE_BEAR_STAY_HIGH: // -> 1 -> 2 -> 5 -> 7 -> 9
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_BEAR_DEAD;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY;
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_BEAR_WALK_HIGH;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = (cmd->jump) ? (TR_STATE_BEAR_ATTACK_HIGH) : (TR_STATE_BEAR_ATTACK);
            }
            else
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY_HIGH;
            }
            break;

        case TR_STATE_BEAR_WALK:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_BEAR_WALK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY;
            }
            break;

        case TR_STATE_BEAR_WALK_HIGH:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_BEAR_WALK_HIGH;
            }
            else
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY_HIGH;
            }
            break;

        case TR_STATE_BEAR_ATTACK:
            cmd->rot[0] = 0;
            state->attack = 0x01;
        case TR_STATE_BEAR_RUN:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_BEAR_RUN_ATTACK;
            }
            else if(!cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_BEAR_RUN;
            }
            else
            {
                ss_anim->next_state = TR_STATE_BEAR_STAY;
            }
            break;

        case TR_STATE_BEAR_ATTACK_HIGH:
        case TR_STATE_BEAR_RUN_ATTACK:
            state->attack = 0x01;
            break;

        default:
        case TR_STATE_BEAR_DEAD:
            cmd->rot[0] = 0;
            break;
    };

    return 0;
}
