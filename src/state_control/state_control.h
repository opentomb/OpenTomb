
#ifndef STATE_CONTROL_H
#define STATE_CONTROL_H

struct entity_s;

#define STATE_FUNCTIONS_LARA                (0x01)
#define STATE_FUNCTIONS_BAT                 (0x02)
#define STATE_FUNCTIONS_WOLF                (0x03)
#define STATE_FUNCTIONS_BEAR                (0x04)
#define STATE_FUNCTIONS_RAPTOR              (0x05)
#define STATE_FUNCTIONS_TREX                (0x06)
#define STATE_FUNCTIONS_LARSON              (0x07)
#define STATE_FUNCTIONS_PIERRE              (0x08)
#define STATE_FUNCTIONS_LION                (0x09)
#define STATE_FUNCTIONS_GORILLA             (0x0A)
#define STATE_FUNCTIONS_CROCODILE           (0x0B)
#define STATE_FUNCTIONS_RAT                 (0x0C)
#define STATE_FUNCTIONS_CENTAUR             (0x0D)
#define STATE_FUNCTIONS_PUMA                (0x0E)
#define STATE_FUNCTIONS_WINGED_MUTANT       (0x0F)
#define STATE_FUNCTIONS_COWBOY              (0x10)
#define STATE_FUNCTIONS_SKATEBOARDIST       (0x11)
#define STATE_FUNCTIONS_MRT                 (0x12)
#define STATE_FUNCTIONS_TORSO_BOSS          (0x13)
#define STATE_FUNCTIONS_NATLA               (0x14)

#define TR_STATE_CURRENT (-1)

#define ANIMATION_KEY_INIT          (0)
#define ANIMATION_KEY_DEAD          (1)

void StateControl_SetStateFunctions(struct entity_s *ent, int functions_id);

#endif
