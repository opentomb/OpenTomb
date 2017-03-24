
#ifndef STATE_CONTROL_RAT_H
#define STATE_CONTROL_RAT_H

#define TR_MODEL_RAT_OF_TR1 16
#define TR_MODEL_RAT_OW_TR1 17

/*
 *  ====== RAT'S ANIMATIONS ======
 */
#define TR_ANIMATION_RAT_OW_FLOW 0
#define TR_ANIMATION_RAT_OW_DEAD 2
#define TR_ANIMATION_RAT_OF_STAY 0
#define TR_ANIMATION_RAT_OF_DEAD 8

//   ====== RAT'S STATES ======
#define TR_STATE_RAT_OW_FLOW 1     // -> 2
#define TR_STATE_RAT_OW_ATTACK 2
#define TR_STATE_RAT_OW_DEAD 3

#define TR_STATE_RAT_STAY 1 // -> 3 -> 4 -> 6
#define TR_STATE_RAT_RUN_ATTACK 2
#define TR_STATE_RAT_RUN 3 // -> 1 -> 2
#define TR_STATE_RAT_ATTACK 4
#define TR_STATE_RAT_DEAD 5
#define TR_STATE_RAT_STAY_HIGH 6 // -> 1

#endif

