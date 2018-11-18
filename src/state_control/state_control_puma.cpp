
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
#include "state_control_puma.h"
#include "state_control.h"


void StateControl_PumaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_PUMA_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            if(current_state == TR_STATE_PUMA_STAY)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_PUMA_DEAD1, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_PUMA_DEAD2, 0);
            }
            break;
    }
}


int StateControl_Puma(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;
    state->attack = 0x00;

    switch(current_state)
    {
        case TR_STATE_PUMA_STAY: // -> 2 -> 3 -> 4 -> 6 -> 7
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_PUMA_DEAD1, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->target_state = TR_STATE_PUMA_STAY_ATTACK;
            }
            else if(cmd->jump)
            {
                ss_anim->target_state = TR_STATE_PUMA_JUMP_ATTACK;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->target_state = TR_STATE_PUMA_ARRRR;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->target_state = (cmd->shift) ? (TR_STATE_PUMA_WALK) : (TR_STATE_PUMA_RUN);
            }
            else
            {
                ss_anim->target_state = TR_STATE_PUMA_STAY;
            }
            break;

        case TR_STATE_PUMA_WALK: // -> 1
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->target_state = TR_STATE_PUMA_WALK;
            }
            else
            {
                ss_anim->target_state = TR_STATE_PUMA_STAY;
            }
            break;

        case TR_STATE_PUMA_RUN: // -> 1
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_PUMA_DEAD2, 0, NULL);
            }
            else if(!cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->target_state = TR_STATE_PUMA_RUN;
            }
            else
            {
                ss_anim->target_state = TR_STATE_PUMA_STAY;
            }
            break;

        case TR_STATE_PUMA_JUMP_ATTACK:
        case TR_STATE_PUMA_STAY_ATTACK:
            state->attack = 0x01;
            break;
            
        case TR_STATE_PUMA_DEAD:
            state->dead = 0x02;
            break;
    };

    return 0;
}
