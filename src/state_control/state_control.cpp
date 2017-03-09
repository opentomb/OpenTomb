
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "state_control.h"

int StateControl_Lara(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LaraSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Bat(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_BatSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Wolf(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_WolfSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Bear(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_BearSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Raptor(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_RaptorSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_TRex(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_TRexSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Larson(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LarsonSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Lion(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LionSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Gorilla(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_GorillaSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Crocodile(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_CrocodileSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Rat(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_RatSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

int StateControl_Centaur(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_CentaurSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

void StateControl_SetStateFunctions(struct entity_s *ent, int functions_id)
{
    if(ent && ent->character)
    {
        switch(functions_id)
        {
            case STATE_FUNCTIONS_LARA:
                ent->character->state_func = StateControl_Lara;
                ent->character->set_idle_anim_func = StateControl_LaraSetIdleAnim;
                break;

            case STATE_FUNCTIONS_BAT:
                ent->character->state_func = StateControl_Bat;
                ent->character->set_idle_anim_func = StateControl_BatSetIdleAnim;
                break;

            case STATE_FUNCTIONS_WOLF:
                ent->character->state_func = StateControl_Wolf;
                ent->character->set_idle_anim_func = StateControl_WolfSetIdleAnim;
                break;

            case STATE_FUNCTIONS_BEAR:
                ent->character->state_func = StateControl_Bear;
                ent->character->set_idle_anim_func = StateControl_BearSetIdleAnim;
                break;

            case STATE_FUNCTIONS_RAPTOR:
                ent->character->state_func = StateControl_Raptor;
                ent->character->set_idle_anim_func = StateControl_RaptorSetIdleAnim;
                break;

            case STATE_FUNCTIONS_TREX:
                ent->character->state_func = StateControl_TRex;
                ent->character->set_idle_anim_func = StateControl_TRexSetIdleAnim;
                break;

            case STATE_FUNCTIONS_LARSON:
                ent->character->state_func = StateControl_Larson;
                ent->character->set_idle_anim_func = StateControl_LarsonSetIdleAnim;
                break;

            case STATE_FUNCTIONS_LION:
                ent->character->state_func = StateControl_Lion;
                ent->character->set_idle_anim_func = StateControl_LionSetIdleAnim;
                break;

            case STATE_FUNCTIONS_GORILLA:
                ent->character->state_func = StateControl_Gorilla;
                ent->character->set_idle_anim_func = StateControl_GorillaSetIdleAnim;
                ent->character->Height = 512.0f;
                break;

            case STATE_FUNCTIONS_CROCODILE:
                ent->character->state_func = StateControl_Crocodile;
                ent->character->set_idle_anim_func = StateControl_CrocodileSetIdleAnim;
                break;

            case STATE_FUNCTIONS_RAT:
                ent->character->state_func = StateControl_Rat;
                ent->character->set_idle_anim_func = StateControl_RatSetIdleAnim;
                break;

            case STATE_FUNCTIONS_CENTAUR:
                ent->character->state_func = StateControl_Centaur;
                ent->character->set_idle_anim_func = StateControl_CentaurSetIdleAnim;
                break;
        }
    }
}
