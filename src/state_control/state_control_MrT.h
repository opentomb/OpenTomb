
#ifndef STATE_CONTROL_MRT_H
#define STATE_CONTROL_MRT_H

/*
 *  ====== MRT'S ANIMATIONS ======
 */
#define TR_ANIMATION_MRT_STAY 7
#define TR_ANIMATION_MRT_SHOOTING 11
#define TR_ANIMATION_MRT_DEAD 14

//   ====== MRT'S STATES ======

#define TR_STATE_MRT_STAY 1  // -> 2 -> 3 -> 4
#define TR_STATE_MRT_WALK 2  // -> 1
#define TR_STATE_MRT_RUN 3   // -> 1
#define TR_STATE_MRT_AIM 4   // -> 1 -> 6
#define TR_STATE_MRT_DEAD 5
#define TR_STATE_MRT_SHOOT 6

#endif

