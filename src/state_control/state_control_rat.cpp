
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
#include "state_control_rat.h"
#include "state_control.h"


void StateControl_RatSetIdleAnim(struct entity_s *ent, int anim_type, int move_type)
{
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type);
    switch(move_type)
    {
        case MOVE_ON_FLOOR:
            if(ss_anim)
            {
                ss_anim->model = World_GetModelByID(TR_MODEL_RAT_TR1);
            }
            Entity_SetAnimation(ent, anim_type, TR_STATE_RAT_STAY, 0, NULL);
            ent->move_type = MOVE_ON_FLOOR;
            break;

        case MOVE_ON_WATER:
            if(ss_anim)
            {
                ss_anim->model = World_GetModelByID(TR_MODEL_RAT_OW_TR1);
            }
            Entity_SetAnimation(ent, anim_type, TR_STATE_RAT_OW_FLOW, 0, NULL);
            ent->move_type = MOVE_ON_WATER;
            break;
    }
}


int StateControl_Rat(struct entity_s *ent, struct ss_animation_s *ss_anim)
{
    character_command_p cmd = &ent->character->cmd;
    character_state_p state = &ent->character->state;
    float *pos = ent->transform + 12;

    ent->character->rotate_speed_mult = 1.0f;
    ss_anim->anim_frame_flags = ANIM_NORMAL_CONTROL;

    state->sprint = 0x00;
    state->crouch = 0x00;

    if(ent->character->height_info.water && (pos[2] - ent->character->height_info.transition_level < 16.0f))
    {
        if(ss_anim->model->id != TR_MODEL_RAT_OW_TR1)
        {
            StateControl_RatSetIdleAnim(ent, ss_anim->type, MOVE_ON_WATER);
        }
        ent->character->parameters.param[PARAM_AIR] = 100;
        ent->character->parameters.maximum[PARAM_AIR] = 100;
        uint16_t current_state = Anim_GetCurrentState(ss_anim);
        switch(current_state)
        {
            ent->dir_flag = ENT_MOVE_FORWARD;
            case TR_STATE_RAT_OW_FLOW: // -> 2
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_RAT_OW_DEAD, 0, NULL);
                }
                else if(cmd->action)
                {
                    ss_anim->next_state = TR_STATE_RAT_OW_ATTACK;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_RAT_OW_FLOW;
                }
                break;

            case TR_STATE_RAT_OW_DEAD:
            case TR_STATE_RAT_OW_ATTACK:
                cmd->rot[0] = 0;
                break;
        };
    }
    else
    {
        if(ss_anim->model->id == TR_MODEL_RAT_OW_TR1)
        {
            StateControl_RatSetIdleAnim(ent, ss_anim->type, MOVE_ON_FLOOR);
        }

        uint16_t current_state = Anim_GetCurrentState(ss_anim);
        switch(current_state)
        {
            case TR_STATE_RAT_STAY: // -> 3 -> 4 -> 6
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_RAT_DEAD, 0, NULL);
                }
                else if(cmd->action)
                {
                    ss_anim->next_state = TR_STATE_RAT_ATTACK;
                }
                else if(cmd->move[0] > 0)
                {
                    ss_anim->next_state = TR_STATE_RAT_RUN;
                }
                else if(cmd->move[0] < 0)
                {
                    ss_anim->next_state = TR_STATE_RAT_STAY_HIGH;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_RAT_STAY;
                }
                break;

            case TR_STATE_RAT_STAY_HIGH: // -> 1
                cmd->rot[0] = 0;
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_RAT_DEAD, 0, NULL);
                }
                else if(cmd->move[0] > 0)
                {
                    ss_anim->next_state = TR_STATE_RAT_STAY;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_RAT_STAY_HIGH;
                }

            case TR_STATE_RAT_RUN: // -> 1 -> 2
                ent->dir_flag = ENT_MOVE_FORWARD;
                if(state->dead)
                {
                    Entity_SetAnimation(ent, ANIM_TYPE_BASE, TR_ANIMATION_RAT_DEAD, 0, NULL);
                }
                if(cmd->action)
                {
                    ss_anim->next_state = TR_STATE_RAT_RUN_ATTACK;
                }
                else if(cmd->move[0] > 0)
                {
                    ss_anim->next_state = TR_STATE_RAT_RUN;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_RAT_STAY;
                }
                break;
        };
    }

    return 0;
}
