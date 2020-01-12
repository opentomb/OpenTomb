
#include <stdlib.h>
#include <stdio.h>

#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/obb.h"

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
#include "state_control_tiger.h"
#include "state_control.h"


void StateControl_TigerSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TIGER_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_TIGER_DEAD, 0);
            break;
    }
}


int StateControl_Tiger(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    uint16_t current_state = Anim_GetCurrentState(ss_anim);

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;
    state->attack = 0x00;
    state->can_attack = 0x00;

    // TODO taken from wolf state control, needs adjustments for tiger
    if(!state->dead && (ent->character->target_id != ENTITY_ID_NONE))
    {
        entity_p target = World_GetEntityByID(ent->character->target_id);
        if(target && Room_IsInNearRoomsList(ent->self->room, target->self->room))
        {
            float pos[3], *v = ent->bf->bone_tags[ent->character->bone_head].current_transform + 12;
            Mat4_vec3_mul_macro(pos, ent->transform.M4x4, v);
            pos[0] -= target->obb->centre[0];
            pos[1] -= target->obb->centre[1];
            pos[2] -= target->obb->centre[2];
            if((pos[2] > -target->obb->extent[2]) && (pos[2] < target->obb->extent[2]) &&
               (pos[0] * pos[0] + pos[1] * pos[1] < 400.0f * 400.0f))
            {
                state->can_attack = 0x01;
            }
        }
    }

    switch(current_state)
    {
        case TR_STATE_TIGER_STAY:
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_TIGER_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->target_state = TR_STATE_TIGER_STAY_ATTACK;
            }
            else if(cmd->jump)
            {
                ss_anim->target_state = TR_STATE_TIGER_JUMP_ATTACK;
            }
            else if(cmd->move[0] < 0)
            {
                if(rand() % 2 == 0)
                {
                    ss_anim->target_state = TR_STATE_TIGER_ROAR_1;
                }
                else
                {
                    ss_anim->target_state = TR_STATE_TIGER_ROAR_2;
                }
            }
            else if(cmd->move[0] > 0)
            {
                if(cmd->shift)
                {
                    ss_anim->target_state = TR_STATE_TIGER_WALK;
                }
                else
                {
                    ss_anim->target_state = TR_STATE_TIGER_RUN;
                }
            }
            else
            {
                ss_anim->target_state = TR_STATE_TIGER_STAY;
            }
            break;

        case TR_STATE_TIGER_WALK:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(!state->dead && cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->target_state = TR_STATE_TIGER_WALK;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TIGER_STAY;
            }
            break;

        case TR_STATE_TIGER_RUN:
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_TIGER_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                if((rand() % 2) == 0)
                {
                    ss_anim->target_state = TR_STATE_TIGER_JUMP_ATTACK;
                }
                else
                {
                    ss_anim->target_state = TR_STATE_TIGER_STAY_ATTACK;
                }
            }
            else if(cmd->jump)
            {
                ss_anim->target_state = TR_STATE_TIGER_JUMP_ATTACK;
            }
            else if(!cmd->shift && (cmd->move[0] > 0))
            {
                ss_anim->target_state = TR_STATE_TIGER_RUN;
            }
            else
            {
                ss_anim->target_state = TR_STATE_TIGER_STAY;
            }
            break;

        case TR_STATE_TIGER_STAY_ATTACK:
        case TR_STATE_TIGER_JUMP_ATTACK:
            state->attack = 0x01;
            break;
            
        case TR_STATE_TIGER_DEAD:
            state->dead = 0x02;
            break;
    };

    return 0;
}
