
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
#include "state_control_crocodile.h"
#include "state_control.h"


void StateControl_CrocodileSetIdleAnim(struct entity_s *ent, int anim_type, int move_type)
{
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type);
    switch(move_type)
    {
        case MOVE_ON_FLOOR:
            if(ss_anim)
            {
                ss_anim->model = World_GetModelByID(TR_MODEL_CROCODILE_TR1);
            }
            Entity_SetAnimation(ent, anim_type, TR_STATE_CROCODILE_STAY, 0, NULL);
            ent->move_type = MOVE_ON_FLOOR;
            break;

        case MOVE_UNDERWATER:
            if(ss_anim)
            {
                ss_anim->model = World_GetModelByID(TR_MODEL_CROCODILE_UW_TR1);
            }
            Entity_SetAnimation(ent, anim_type, TR_STATE_CROCODILE_UW_FLOW, 0, NULL);
            ent->move_type = MOVE_UNDERWATER;
            break;
    }
}


int StateControl_Crocodile(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    float *pos = ent->transform + 12;

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;

    if(ent->self->room && (ent->self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(ss_anim->model->id != TR_MODEL_CROCODILE_UW_TR1)
        {
            StateControl_CrocodileSetIdleAnim(ent, ss_anim->type, MOVE_UNDERWATER);
        }
        ent->character->parameters.param[PARAM_AIR] = 100;
        ent->character->parameters.maximum[PARAM_AIR] = 100;
        uint16_t current_state = Anim_GetCurrentState(ss_anim);
        switch(current_state)
        {
            case TR_STATE_CROCODILE_UW_FLOW: // -> 2
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_CROCODILE_UW_DEAD, 0, NULL);
                }
                else if(cmd->action)
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_UW_ATTACK;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_UW_FLOW;
                }
                break;

            case TR_STATE_CROCODILE_UW_ATTACK: // -> 1
                ent->dir_flag = ENT_MOVE_FORWARD;
                if(!state->dead && cmd->action)
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_UW_ATTACK;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_UW_FLOW;
                }
                break;
        };
    }
    else
    {
        if(ss_anim->model->id == TR_MODEL_CROCODILE_UW_TR1)
        {
            StateControl_CrocodileSetIdleAnim(ent, ss_anim->type, MOVE_ON_FLOOR);
        }

        uint16_t current_state = Anim_GetCurrentState(ss_anim);
        switch(current_state)
        {
            case TR_STATE_CROCODILE_STAY: // -> 2 -> 3 -> 4 -> 5
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_CROCODILE_DEAD, 0, NULL);
                }
                else if(cmd->action)
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_ATTACK;
                }
                else if(cmd->move[0] > 0)
                {
                    ss_anim->next_state = (cmd->shift) ? (TR_STATE_CROCODILE_WALK) : (TR_STATE_CROCODILE_RUN);
                }
                else if(cmd->roll)
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_RUN_RIGHT;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_STAY;
                }
                break;

            case TR_STATE_CROCODILE_WALK: // -> 1 -> 2
                ent->dir_flag = ENT_MOVE_FORWARD;
                if(!state->dead && (cmd->move[0] > 0))
                {
                    ss_anim->next_state = (cmd->shift) ? (TR_STATE_CROCODILE_WALK) : (TR_STATE_CROCODILE_RUN);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_STAY;
                }
                break;

            case TR_STATE_CROCODILE_RUN: // -> 1 -> 3
                ent->dir_flag = ENT_MOVE_FORWARD;
                if(!state->dead && (cmd->move[0] > 0))
                {
                    ss_anim->next_state = (cmd->shift) ? (TR_STATE_CROCODILE_WALK) : (TR_STATE_CROCODILE_RUN);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_STAY;
                }
                break;

            case TR_STATE_CROCODILE_RUN_RIGHT:
                ent->dir_flag = ENT_MOVE_FORWARD;
                cmd->rot[0] = -1;
                if(!state->dead && cmd->roll)
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_RUN_RIGHT;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_CROCODILE_WALK;
                }
                break;
        };
    }

    return 0;
}
