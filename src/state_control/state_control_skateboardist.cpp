
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
#include "state_control_skateboardist.h"
#include "state_control.h"


void StateControl_SkateboardistSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim)
{
    switch(key_anim)
    {
        case ANIMATION_KEY_INIT:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_SKATEBOARDIST_STAY, 0);
            break;

        case ANIMATION_KEY_DEAD:
            Anim_SetAnimation(ss_anim, TR_ANIMATION_SKATEBOARDIST_DEAD, 0);
            break;
    }
}


int StateControl_Skateboardist(struct entity_s *ent, struct ss_animation_s *ss_anim)
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
        case TR_STATE_SKATEBOARDIST_INERTIA: // -> 0 -> 3 -> 4
            if(state->dead)
            {
                Entity_SetAnimation(ent, ss_anim->type, TR_ANIMATION_SKATEBOARDIST_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_SHOOTING2;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_RIDE;
            }
            else if(cmd->move[0] < 0)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_STOP;
            }
            else
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_INERTIA;
            }
            break;

        case TR_STATE_SKATEBOARDIST_RIDE:   // -> 2
            ent->dir_flag = ENT_MOVE_FORWARD;
            if(state->dead)
            {
                Entity_SetAnimation(ent, ss_anim->type, TR_ANIMATION_SKATEBOARDIST_DEAD, 0, NULL);
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_RIDE;
            }
            else
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_INERTIA;
            }
            break;

        case TR_STATE_SKATEBOARDIST_STOP:   // -> 1 -> 2
            if(state->dead)
            {
                Entity_SetAnimation(ent, ss_anim->type, TR_ANIMATION_SKATEBOARDIST_DEAD, 0, NULL);
            }
            else if(cmd->action)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_SHOOTING1;
            }
            else if(cmd->move[0] > 0)
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_INERTIA;
            }
            else
            {
                ss_anim->target_state = TR_STATE_SKATEBOARDIST_STOP;
            }
            break;

        case TR_STATE_SKATEBOARDIST_SHOOTING1:
        case TR_STATE_SKATEBOARDIST_SHOOTING2:
            {
                entity_p target = World_GetEntityByID(ent->character->target_id);
                if((ss_anim->frame_changing_state >= 0x02) && target && Character_IsTargetAccessible(ent, target))
                {
                    Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                }
            }
            break;
            
        case TR_STATE_SKATEBOARDIST_DEAD:
            state->dead = 0x02;
            break;
    };

    if(ent->character->state.slide == CHARACTER_SLIDE_BACK)
    {
        ent->dir_flag = ENT_MOVE_BACKWARD;
        ent->anim_linear_speed = 64;
    }
    else if(ent->character->state.slide == CHARACTER_SLIDE_FRONT)
    {
        ent->dir_flag = ENT_MOVE_FORWARD;
        ent->anim_linear_speed = 64;
    }

    return 0;
}
