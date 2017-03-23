
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
#include "state_control_Pierre.h"
#include "state_control.h"


void StateControl_PierreSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_PIERRE_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_PIERRE_DEAD, 0);
            break;
    }
}


int StateControl_Pierre(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;

    switch(current_state)
    {
        case TR_STATE_PIERRE_STAY:
            if(state->dead || cmd->action)
            {
                ss_anim->next_state = TR_STATE_PIERRE_AIM;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY_QUEUED;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_PIERRE_WALK) : (TR_STATE_PIERRE_RUN);
            }
            else
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY;
            }
            break;

        case TR_STATE_PIERRE_STAY_QUEUED:
            if(state->dead || cmd->action || (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY;
            }
            else
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY_QUEUED;
            }
            break;

        case TR_STATE_PIERRE_WALK:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_PIERRE_WALK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY;
            }
            break;

        case TR_STATE_PIERRE_RUN:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && !cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_PIERRE_RUN;
            }
            else
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY;
            }
            break;

        case TR_STATE_PIERRE_AIM:
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_PIERRE_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_PIERRE_SHOOT;
            }
            else
            {
                ss_anim->next_state = TR_STATE_PIERRE_STAY;
            }
            break;

        case TR_STATE_PIERRE_SHOOT:
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_PIERRE_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_PIERRE_SHOOT;
            }
            else
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_PIERRE_STOP_SHOOTING, 0, NULL);
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
