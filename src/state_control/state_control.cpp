
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "state_control.h"

int StateControl_Lara(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LaraSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);
void StateControl_LaraSetWeaponModel(struct entity_s *ent, int weapon_model, int weapon_state);

int StateControl_Bat(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_BatSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Wolf(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_WolfSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Bear(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_BearSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Raptor(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_RaptorSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_TRex(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_TRexSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Larson(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LarsonSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Pierre(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_PierreSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Lion(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LionSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Gorilla(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_GorillaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Crocodile(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_CrocodileSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Rat(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_RatSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Centaur(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_CentaurSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Puma(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_PumaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_WingedMutant(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_WingedMutantSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Cowboy(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_CowboySetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_MrT(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_MrTSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Skateboardist(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_SkateboardistSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_TorsoBoss(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_TorsoBossSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Natla(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_NatlaSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

int StateControl_Tiger(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_TigerSetKeyAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);

void StateControl_SetStateFunctions(struct entity_s *ent, int functions_id)
{
    if(ent && ent->character)
    {
        switch(functions_id)
        {
            case STATE_FUNCTIONS_LARA:
                ent->character->state_func = StateControl_Lara;
                ent->character->set_key_anim_func = StateControl_LaraSetKeyAnim;
                ent->character->set_weapon_model_func = StateControl_LaraSetWeaponModel;
                break;

            case STATE_FUNCTIONS_BAT:
                ent->character->state_func = StateControl_Bat;
                ent->character->set_key_anim_func = StateControl_BatSetKeyAnim;
                break;

            case STATE_FUNCTIONS_WOLF:
                ent->character->state_func = StateControl_Wolf;
                ent->character->set_key_anim_func = StateControl_WolfSetKeyAnim;
                break;

            case STATE_FUNCTIONS_BEAR:
                ent->character->state_func = StateControl_Bear;
                ent->character->set_key_anim_func = StateControl_BearSetKeyAnim;
                break;

            case STATE_FUNCTIONS_RAPTOR:
                ent->character->state_func = StateControl_Raptor;
                ent->character->set_key_anim_func = StateControl_RaptorSetKeyAnim;
                break;

            case STATE_FUNCTIONS_TREX:
                ent->character->state_func = StateControl_TRex;
                ent->character->set_key_anim_func = StateControl_TRexSetKeyAnim;
                break;

            case STATE_FUNCTIONS_LARSON:
                ent->character->state_func = StateControl_Larson;
                ent->character->set_key_anim_func = StateControl_LarsonSetKeyAnim;
                break;

            case STATE_FUNCTIONS_PIERRE:
                ent->character->state_func = StateControl_Pierre;
                ent->character->set_key_anim_func = StateControl_PierreSetKeyAnim;
                break;

            case STATE_FUNCTIONS_LION:
                ent->character->state_func = StateControl_Lion;
                ent->character->set_key_anim_func = StateControl_LionSetKeyAnim;
                break;

            case STATE_FUNCTIONS_GORILLA:
                ent->character->state_func = StateControl_Gorilla;
                ent->character->set_key_anim_func = StateControl_GorillaSetKeyAnim;
                break;

            case STATE_FUNCTIONS_CROCODILE:
                ent->character->state_func = StateControl_Crocodile;
                ent->character->set_key_anim_func = StateControl_CrocodileSetKeyAnim;
                break;

            case STATE_FUNCTIONS_RAT:
                ent->character->state_func = StateControl_Rat;
                ent->character->set_key_anim_func = StateControl_RatSetKeyAnim;
                break;

            case STATE_FUNCTIONS_CENTAUR:
                ent->character->state_func = StateControl_Centaur;
                ent->character->set_key_anim_func = StateControl_CentaurSetKeyAnim;
                break;

            case STATE_FUNCTIONS_PUMA:
                ent->character->state_func = StateControl_Puma;
                ent->character->set_key_anim_func = StateControl_PumaSetKeyAnim;
                break;

            case STATE_FUNCTIONS_WINGED_MUTANT:
                ent->character->state_func = StateControl_WingedMutant;
                ent->character->set_key_anim_func = StateControl_WingedMutantSetKeyAnim;
                break;

            case STATE_FUNCTIONS_COWBOY:
                ent->character->state_func = StateControl_Cowboy;
                ent->character->set_key_anim_func = StateControl_CowboySetKeyAnim;
                break;

            case STATE_FUNCTIONS_MRT:
                ent->character->state_func = StateControl_MrT;
                ent->character->set_key_anim_func = StateControl_MrTSetKeyAnim;
                break;

            case STATE_FUNCTIONS_SKATEBOARDIST:
                ent->character->state_func = StateControl_Skateboardist;
                ent->character->set_key_anim_func = StateControl_SkateboardistSetKeyAnim;
                break;

            case STATE_FUNCTIONS_TORSO_BOSS:
                ent->character->state_func = StateControl_TorsoBoss;
                ent->character->set_key_anim_func = StateControl_TorsoBossSetKeyAnim;
                break;

            case STATE_FUNCTIONS_NATLA:
                ent->character->state_func = StateControl_Natla;
                ent->character->set_key_anim_func = StateControl_NatlaSetKeyAnim;
                break;

            case STATE_FUNCTIONS_TIGER:
                ent->character->state_func = StateControl_Tiger;
                ent->character->set_key_anim_func = StateControl_TigerSetKeyAnim;
                break;
        }
    }
}
