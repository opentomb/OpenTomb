#pragma once

#include <AL/al.h>

#include "loader/datatypes.h"

namespace audio
{
// Effect structure.
// Contains all global effect parameters.
struct Effect
{
    // General sound source parameters (similar to TR sound info).

    ALfloat     pitch = 0;          // [PIT in TR] Global pitch shift.
    ALfloat     gain = 0;           // [VOL in TR] Global gain (volume).
    ALfloat     range = 0;          // [RAD in TR] Range (radius).
    ALuint      chance = 0;         // [CH  in TR] Chance to play.
    loader::LoopType loop = loader::LoopType::None;
    ALboolean   rand_pitch = AL_FALSE;     // Similar to flag 0x200000 (P) in native TRs.
    ALboolean   rand_gain = AL_FALSE;      // Similar to flag 0x400000 (V) in native TRs.

    // Additional sound source parameters.
    // These are not natively in TR engines, but can be later assigned by
    // external script.

    ALboolean   rand_freq = false;          // Slightly randomize frequency.
    ALuint      rand_pitch_var = 0;     // Pitch randomizer bounds.
    ALuint      rand_gain_var = 0;      // Gain  randomizer bounds.
    ALuint      rand_freq_var = 0;      // Frequency randomizer bounds.

    // Sample reference parameters.

    ALuint      sample_index = 0;       // First (or only) sample (buffer) index.
    ALuint      sample_count = 0;       // Sample amount to randomly select from.
};
} // namespace audio
