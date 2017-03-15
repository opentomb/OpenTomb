
#ifndef STATE_CONTROL_NATLA_H
#define STATE_CONTROL_NATLA_H

/*
 *  ====== NATLA'S ANIMATIONS ======
 */
#define TR_ANIMATION_NATLA_STAY 0
#define TR_ANIMATION_NATLA_FLY 5
#define TR_ANIMATION_NATLA_DROP 6
#define TR_ANIMATION_NATLA_DEAD 13

//   ====== NATLA'S STATES ======
#define TR_STATE_NATLA_STAY 1 // -> 2  -> 4 -> 5
#define TR_STATE_NATLA_FLY 2 // -> 1 -> 5 -> 7
#define TR_STATE_NATLA_RUN 3 // -> 8 -> 9
#define TR_STATE_NATLA_STAY_AIM 4 // -> 1 -> 5 -> 6
#define TR_STATE_NATLA_DROPPED 5 // -> 8
#define TR_STATE_NATLA_STAY_SHOOT 6 // -> 5
#define TR_STATE_NATLA_FALL 7 // -> 5
#define TR_STATE_NATLA_STAND_UP 8 // -> 3 -> 9
#define TR_STATE_NATLA_DEAD 9



#endif

