
#ifndef STATE_CONTROL_BEAR_H
#define STATE_CONTROL_BEAR_H

/*
 *  ====== BEAR'S ANIMATIONS ======
 */
#define TR_ANIMATION_BEAR_STAY 0
#define TR_ANIMATION_BEAR_RUN 2
#define TR_ANIMATION_BEAR_ATTACK_HIGH 5
#define TR_ANIMATION_BEAR_STAY_HIGH 7
#define TR_ANIMATION_BEAR_ATTACK 11
#define TR_ANIMATION_BEAR_EAT 15
#define TR_ANIMATION_BEAR_WALK 17

//   ====== BEAR'S STATES ======
#define TR_STATE_BEAR_WALK 0            // -> 1
#define TR_STATE_BEAR_STAY 1            // -> 0 -> 3 -> 4 -> 5 -> 8 -> 9
#define TR_STATE_BEAR_WALK_HIGH 2       // -> 4
#define TR_STATE_BEAR_RUN 3             // -> 1 -> 6
#define TR_STATE_BEAR_STAY_HIGH 4       // -> 1 -> 2 -> 5 -> 7 -> 9 
#define TR_STATE_BEAR_ATTACK 5          //
#define TR_STATE_BEAR_RUN_ATTACK 6      //
#define TR_STATE_BEAR_ATTACK_HIGH 7
#define TR_STATE_BEAR_EAT 8
#define TR_STATE_BEAR_DEAD 9

#endif

