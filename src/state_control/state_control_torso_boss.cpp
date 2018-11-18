
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
#include "state_control_torso_boss.h"
#include "state_control.h"


void StateControl_TorsoBossSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TORSO_BOSS_INIT, 0);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TORSO_BOSS_DEAD, 0);
            break;
    }
}


int StateControl_TorsoBoss(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;
    state->attack = 0x00;

    ent->character->rotate_speed_mult = 0.33;
    switch(current_state)
    {
        case TR_STATE_TORSO_BOSS_STAY: // -> 2 -> 3 -> 4 -> 5 -> 6 -> 7
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_TORSO_BOSS_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->target_state = (cmd->shift) ? (TR_STATE_TORSO_BOSS_ATTACK_BIG) : (TR_STATE_TORSO_BOSS_ATTACK);
            }
            else if(cmd->jump)
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_ATTACK_KILL;
            }
            else if(cmd->move[1] < 0)
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_TURN_RIGHT;
            }
            else if(cmd->move[1] > 0)
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_TURN_LEFT;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_MOVE;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_STAY;
            }
            break;

        case TR_STATE_TORSO_BOSS_MOVE: // -> 1
            if(!state->dead && (cmd->move[1] > 0))
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_MOVE;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_STAY;
            }
            break;

        case TR_STATE_TORSO_BOSS_TURN_RIGHT: // -> 1
            cmd->rot[0] = 1;
            if(!state->dead && (cmd->move[1] < 0))
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_TURN_RIGHT;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_STAY;
            }
            break;

        case TR_STATE_TORSO_BOSS_TURN_LEFT: // -> 1
            cmd->rot[0] = -1;
            if(!state->dead && (cmd->move[1] > 0))
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_TURN_LEFT;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TORSO_BOSS_STAY;
            }
            break;

        case TR_STATE_TORSO_BOSS_ATTACK_KILL:
        case TR_STATE_TORSO_BOSS_ATTACK:
        case TR_STATE_TORSO_BOSS_ATTACK_BIG:
        case TR_STATE_TORSO_BOSS_ATTACK_JUMP:
            state->attack = 0x01;
            cmd->rot[0] = 0;
            break;

        case TR_STATE_TORSO_BOSS_DEAD:
            state->dead = 0x02;
            break;
            
        default:
            cmd->rot[0] = 0;
            break;
    };

    return 0;
}
