
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
#include "state_control_bat.h"
#include "state_control.h"


void StateControl_BatSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            if(ent->move_type == MOVE_STATIC_POS)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_BAT_START, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_BAT_FLY, 0);
            }
            break;

        case ANIMATION_KEY_DEAD:
            if(ent->move_type == MOVE_ON_FLOOR)
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_BAT_DEAD, 0);
            }
            else
            {
                Anim_SetAnimation(ss_anim, TR_ANIMATION_BAT_DROP, 0);
            }
            break;
    }
}


int StateControl_Bat(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->attack = 0x00;

    switch(current_state)
    {
        case TR_STATE_BAT_START:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_STAY;
            ent->move_type = MOVE_FLY;
            ss_anim->next_state = TR_STATE_BAT_FLY;
            break;

        case TR_STATE_BAT_ATTACK:
            state->attack = 0x01;
        case TR_STATE_BAT_FLY:
            if(state->dead)
            {
                ss_anim->next_state = TR_STATE_BAT_DROP;
                ent->move_type = MOVE_FREE_FALLING;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_BAT_ATTACK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_BAT_FLY;
            }
            break;

        case TR_STATE_BAT_DROP:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_STAY;
            ent->speed[0] = 0.0f;
            ent->speed[1] = 0.0f;

            if(ent->move_type == MOVE_ON_FLOOR)
            {
                ss_anim->next_state = TR_STATE_BAT_DEAD;
            }
            break;

        case TR_STATE_BAT_DEAD:
            cmd->rot[0] = 0;
            ent->dir_flag = ENT_STAY;
            state->dead = 0x02;
            if(ent->move_type == MOVE_FREE_FALLING)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_BAT_DROP, 0, NULL);
                state->dead = 0x01;
            }
            break;
    };
    
    return 0;
}
