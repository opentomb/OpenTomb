#pragma once

#include <cstdint>

namespace world
{
namespace animation
{
using AnimationEffect = uint32_t;
//   ====== ANIMATION EFFECTS / FLIPEFFECTS ======

constexpr AnimationEffect TR_EFFECT_CHANGEDIRECTION = 0;
constexpr AnimationEffect TR_EFFECT_SHAKESCREEN = 1;
constexpr AnimationEffect TR_EFFECT_PLAYFLOODSOUND = 2;
constexpr AnimationEffect TR_EFFECT_BUBBLE = 3;
constexpr AnimationEffect TR_EFFECT_ENDLEVEL = 4;
constexpr AnimationEffect TR_EFFECT_ACTIVATECAMERA = 5;
constexpr AnimationEffect TR_EFFECT_ACTIVATEKEY = 6;
constexpr AnimationEffect TR_EFFECT_ENABLEEARTHQUAKES = 7;
constexpr AnimationEffect TR_EFFECT_GETCROWBAR = 8;
constexpr AnimationEffect TR_EFFECT_CURTAINFX = 9;  // Effect 9 is empty in TR4.
constexpr AnimationEffect TR_EFFECT_PLAYSOUND_TIMERFIELD = 10;
constexpr AnimationEffect TR_EFFECT_PLAYEXPLOSIONSOUND = 11;
constexpr AnimationEffect TR_EFFECT_DISABLEGUNS = 12;
constexpr AnimationEffect TR_EFFECT_ENABLEGUNS = 13;
constexpr AnimationEffect TR_EFFECT_GETRIGHTGUN = 14;
constexpr AnimationEffect TR_EFFECT_GETLEFTGUN = 15;
constexpr AnimationEffect TR_EFFECT_FIRERIGHTGUN = 16;
constexpr AnimationEffect TR_EFFECT_FIRELEFTGUN = 17;
constexpr AnimationEffect TR_EFFECT_MESHSWAP1 = 18;
constexpr AnimationEffect TR_EFFECT_MESHSWAP2 = 19;
constexpr AnimationEffect TR_EFFECT_MESHSWAP3 = 20;
constexpr AnimationEffect TR_EFFECT_INV_ON = 21; // Effect 21 is unknown at offset 4376F0.
constexpr AnimationEffect TR_EFFECT_INV_OFF = 22; // Effect 22 is unknown at offset 437700.
constexpr AnimationEffect TR_EFFECT_HIDEOBJECT = 23;
constexpr AnimationEffect TR_EFFECT_SHOWOBJECT = 24;
constexpr AnimationEffect TR_EFFECT_STATUEFX = 25; // Effect 25 is empty in TR4.
constexpr AnimationEffect TR_EFFECT_RESETHAIR = 26;
constexpr AnimationEffect TR_EFFECT_BOILERFX = 27; // Effect 27 is empty in TR4.
constexpr AnimationEffect TR_EFFECT_SETFOGCOLOUR = 28;
constexpr AnimationEffect TR_EFFECT_GHOSTTRAP = 29; // Effect 29 is unknown at offset 4372F0
constexpr AnimationEffect TR_EFFECT_LARALOCATION = 30;
constexpr AnimationEffect TR_EFFECT_CLEARSCARABS = 31;
constexpr AnimationEffect TR_EFFECT_PLAYSTEPSOUND = 32; // Also called FOOTPRINT_FX in TR4 source code.

                                                             // Effects 33 - 42 are assigned to FLIP_MAP0-FLIP_MAP9 in TR4 source code,
                                                             // but are empty in TR4 binaries.

constexpr AnimationEffect TR_EFFECT_GETWATERSKIN = 43;
constexpr AnimationEffect TR_EFFECT_REMOVEWATERSKIN = 44;
constexpr AnimationEffect TR_EFFECT_LARALOCATIONPAD = 45;
constexpr AnimationEffect TR_EFFECT_KILLALLENEMIES = 46;
}
}
