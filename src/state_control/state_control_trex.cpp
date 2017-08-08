
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
#include "state_control_trex.h"
#include "state_control.h"


void StateControl_TRexSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TREX_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TREX_DEAD, 0);
            break;
    }
}


int StateControl_TRex(struct entity_s *ent, struct ss_animation_s *ss_anim)
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
        case TR_STATE_TREX_STAY: // -> 2 -> 3 -> 5 -> 6 -> 7
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_TREX_DEAD;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_TREX_ATTACK;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_TREX_ARRRR;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_TREX_WALK) : (TR_STATE_TREX_RUN);
            }
            else
            {
                ss_anim->next_state = TR_STATE_TREX_STAY;
            }
            break;

        case TR_STATE_TREX_WALK:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_TREX_WALK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_TREX_STAY;
            }
            break;

        case TR_STATE_TREX_RUN:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && !cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->next_state = TR_STATE_TREX_RUN;
            }
            else
            {
                ss_anim->next_state = TR_STATE_TREX_STAY;
            }
            break;

        case TR_STATE_TREX_ATTACK:
            cmd->rot[0] = 0;
            state->attack = 0x01;
            /*if(World_GetPlayer() && World_GetPlayer()->character &&  World_GetPlayer()->character->state.dead)
            {
                ss_anim->next_state = TR_STATE_TREX_KILL;
            }
            else*/
            {
                ss_anim->next_state = TR_STATE_TREX_ATTACK;
            }
            break;
            
        case TR_STATE_TREX_DEAD:
            state->dead = 0x02;
            break;
    };

    return 0;
}
