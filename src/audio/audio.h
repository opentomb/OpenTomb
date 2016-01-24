#pragma once

#include <cstdint>

namespace audio
{
using SoundId = uint32_t;

// Define some common samples across ALL TR versions.

constexpr SoundId SoundBubble = 37;
constexpr SoundId SoundCurtainMove = 191;
constexpr SoundId SoundDiscBladeHit = 258;
constexpr SoundId SoundDiscgunShoot = 251;
constexpr SoundId SoundDoorbell = 334;
constexpr SoundId SoundDoorOpen = 64;
constexpr SoundId SoundEagleDying = 131;
constexpr SoundId SoundExplosion = 105;
constexpr SoundId SoundFromWater = 34;
constexpr SoundId SoundGenDeath = 41;
constexpr SoundId SoundHelicopter = 297;
constexpr SoundId SoundHolsterIn = 7;
constexpr SoundId SoundHolsterOut = 6;
constexpr SoundId SoundLanding = 4;
constexpr SoundId SoundLaraBreath = 36;
constexpr SoundId SoundLaraInjury = 31;
constexpr SoundId SoundLaraScream = 30;
constexpr SoundId SoundMedipack = 116;
constexpr SoundId SoundMenuRotate = 108;
constexpr SoundId SoundNo = 2;
constexpr SoundId SoundPushable = 63;
constexpr SoundId SoundReload = 9;
constexpr SoundId SoundRicochet = 10;
constexpr SoundId SoundShotPistols = 8;
constexpr SoundId SoundShotShotgun = 45;
constexpr SoundId SoundShotUzi = 43;
constexpr SoundId SoundSliding = 3;
constexpr SoundId SoundSpike = 343;
constexpr SoundId SoundSpikedMetalDoorSlide = 127;
constexpr SoundId SoundSplash = 33;
constexpr SoundId SoundSwim = 35;
constexpr SoundId SoundTigerGrowl = 103;
constexpr SoundId SoundTR123MenuClose = 112;
constexpr SoundId SoundTR123MenuOpen = 111;
constexpr SoundId SoundTR123MenuPage = 115;
constexpr SoundId SoundTR123MenuWeapon = 114;
constexpr SoundId SoundTR13MenuSelect = 109;
constexpr SoundId SoundTR1DartShoot = 151;
constexpr SoundId SoundTR1MenuClang = 113;
constexpr SoundId SoundTR2345MenuClang = 114;
constexpr SoundId SoundTR2MenuSelect = 112;
constexpr SoundId SoundTR2MovingWall = 204;
constexpr SoundId SoundTR2SpikeHit = 205;
constexpr SoundId SoundTR3DartgunShoot = 325;
constexpr SoundId SoundTR3MovingWall = 147;
constexpr SoundId SoundTR3SpikeHit = 56;
constexpr SoundId SoundTR45MenuOpenClose = 109;
constexpr SoundId SoundTR45MenuPage = 111;
constexpr SoundId SoundTR45MenuSelect = 111;
constexpr SoundId SoundTR45MenuWeapon = 9;
constexpr SoundId SoundUnderwater = 60;
constexpr SoundId SoundUseKey = 39;
constexpr SoundId SoundWadeShallow = 18;

// Certain sound effect indexes were changed across different TR
// versions, despite remaining the same - mostly, it happened with
// menu sounds and some general sounds. For such effects, we specify
// additional remap enumeration list, which is fed into Lua script
// to get actual effect ID for current game version.

enum class GlobalSoundId
{
    MenuOpen,
    MenuClose,
    MenuPage,
    MenuSelect,
    MenuWeapon,
    MenuClang,
    MovingWall,
    SpikeHit
};

// Error handling routines.

bool checkALError(const char *error_marker = "");    // AL-specific error handler.
void logSndfileError(int code);           // Sndfile-specific error handler.
} // namespace audio
