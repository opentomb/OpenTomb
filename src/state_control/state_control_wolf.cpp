
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
#include "../world.h"
#include "state_control_wolf.h"
#include "state_control.h"


void StateControl_WolfSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_WOLF_STAY_TO_IDLE, -1);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_WOLF_DEAD1, 0);
            break;
    }
}


int StateControl_Wolf(struct entity_s *ent, struct ss_animation_s *ss_anim)
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
    state->can_attack = 0x00;

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
        case TR_STATE_WOLF_STAY:// 1    // -> 2 -> 7 -> 8 -> 9
            cmd->rot[0] = 0;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->next_state = TR_STATE_WOLF_WALK;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(cmd->shift)
            {
                ss_anim->next_state = TR_STATE_WOLF_STAY_CROUCH;
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_WOLF_WOOOO;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->next_state = TR_STATE_WOLF_IDLE;
            }
            else
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            break;

        case TR_STATE_WOLF_STAY_CROUCH:// 9 // -> 1 -> 3 -> 5 -> 7 -> 12
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_WOLF_CROUCH) : (TR_STATE_WOLF_RUN);
            }
            else if(cmd->action)
            {
                cmd->rot[0] = 0;
                ss_anim->next_state = TR_STATE_WOLF_STAY_ATTACK;
            }
            else if(cmd->jump)
            {
                cmd->rot[0] = 0;
                ss_anim->next_state = TR_STATE_WOLF_WOOOO;
            }
            else if(cmd->shift)
            {
                ss_anim->next_state = TR_STATE_WOLF_STAY_CROUCH;
            }
            else
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_STAY;
            }
            break;

        case TR_STATE_WOLF_WALK:// 2    // -> 1 -> 5
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_WOLF_WALK) : (TR_STATE_WOLF_CROUCH);
            }
            else
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_STAY;
            }
            break;

        case TR_STATE_WOLF_RUN:// 3     // -> 6 -> 9 -> 10
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_WOLF_JUMP_ATTACK;
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_WOLF_RUN) : (TR_STATE_WOLF_CROUCH);
            }
            else if(cmd->roll)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_RUN_RIGHT;
            }
            else
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_STAY_CROUCH;
            }
            break;

        case TR_STATE_WOLF_CROUCH:// 5  // -> 3 -> 9 -> 12
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->action)
            {
                cmd->rot[0] = 0;
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_JUMP_ATTACK;
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = (cmd->shift) ? (TR_STATE_WOLF_CROUCH) : (TR_STATE_WOLF_RUN);
            }
            else
            {
                cmd->rot[0] = 0;
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_STAY_CROUCH;
            }
            break;

        case TR_STATE_WOLF_STAY_ATTACK:// 12
            state->attack = 0x01;
            break;

        case TR_STATE_WOLF_JUMP_ATTACK:// 6   // -> 3
            ent->dir_flag = ENT_MOVE_FORWARD;
            state->attack = 0x01;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD3, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_WOLF_JUMP_ATTACK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_WOLF_RUN;
            }
            break;

        case TR_STATE_WOLF_WOOOO:// 7
            cmd->rot[0] = 0;
            break;

        case TR_STATE_WOLF_IDLE:// 8    // -> 1
            cmd->rot[0] = 0;
            ent->no_move = 0x01;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD1, 0, NULL);
            }
            else if(cmd->move[0] > 0)
            {
                ent->dir_flag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_WOLF_STAY;
            }
            else
            {
                ss_anim->next_state = TR_STATE_WOLF_IDLE;
            }
            break;

        case TR_STATE_WOLF_RUN_RIGHT:// 10
            if(state->dead)
            {
                Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_WOLF_DEAD2, 0, NULL);
            }
            else
            {
                cmd->rot[0] = -1;
                ent->dir_flag = ENT_MOVE_FORWARD;
            }
            break;

        case TR_STATE_WOLF_DEAD:// 11
            state->dead = 0x01;
            if(ss_anim->model->animations[ss_anim->current_animation].max_frame <= ss_anim->current_frame + 1)
            {
                state->dead = 0x02;
                state->ragdoll = 0x01;
            }
            break;

        default:
            cmd->rot[0] = 0;
            break;
    };

    return 0;
}
