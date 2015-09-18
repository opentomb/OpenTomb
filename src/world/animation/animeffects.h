#pragma once

//   ====== ANIMATION EFFECTS / FLIPEFFECTS ======

#define TR_EFFECT_CHANGEDIRECTION       0
#define TR_EFFECT_SHAKESCREEN           1
#define TR_EFFECT_PLAYFLOODSOUND        2
#define TR_EFFECT_BUBBLE                3
#define TR_EFFECT_ENDLEVEL              4
#define TR_EFFECT_ACTIVATECAMERA        5
#define TR_EFFECT_ACTIVATEKEY           6
#define TR_EFFECT_ENABLEEARTHQUAKES     7
#define TR_EFFECT_GETCROWBAR            8
#define TR_EFFECT_CURTAINFX             9  // Effect 9 is empty in TR4.
#define TR_EFFECT_PLAYSOUND_TIMERFIELD  10
#define TR_EFFECT_PLAYEXPLOSIONSOUND    11
#define TR_EFFECT_DISABLEGUNS           12
#define TR_EFFECT_ENABLEGUNS            13
#define TR_EFFECT_GETRIGHTGUN           14
#define TR_EFFECT_GETLEFTGUN            15
#define TR_EFFECT_FIRERIGHTGUN          16
#define TR_EFFECT_FIRELEFTGUN           17
#define TR_EFFECT_MESHSWAP1             18
#define TR_EFFECT_MESHSWAP2             19
#define TR_EFFECT_MESHSWAP3             20
#define TR_EFFECT_INV_ON                21 // Effect 21 is unknown at offset 4376F0.
#define TR_EFFECT_INV_OFF               22 // Effect 22 is unknown at offset 437700.
#define TR_EFFECT_HIDEOBJECT            23
#define TR_EFFECT_SHOWOBJECT            24
#define TR_EFFECT_STATUEFX              25 // Effect 25 is empty in TR4.
#define TR_EFFECT_RESETHAIR             26
#define TR_EFFECT_BOILERFX              27 // Effect 27 is empty in TR4.
#define TR_EFFECT_SETFOGCOLOUR          28
#define TR_EFFECT_GHOSTTRAP             29 // Effect 29 is unknown at offset 4372F0
#define TR_EFFECT_LARALOCATION          30
#define TR_EFFECT_CLEARSCARABS          31
#define TR_EFFECT_PLAYSTEPSOUND         32 // Also called FOOTPRINT_FX in TR4 source code.

// Effects 33 - 42 are assigned to FLIP_MAP0-FLIP_MAP9 in TR4 source code,
// but are empty in TR4 binaries.

#define TR_EFFECT_GETWATERSKIN          43
#define TR_EFFECT_REMOVEWATERSKIN       44
#define TR_EFFECT_LARALOCATIONPAD       45
#define TR_EFFECT_KILLALLENEMIES        46
