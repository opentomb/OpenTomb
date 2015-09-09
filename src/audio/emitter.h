#pragma once

#include <cstdint>

#include <AL/al.h>
#include <LinearMath/btVector3.h>

namespace audio
{

// Audio emitter (aka SoundSource) structure.
struct Emitter
{
    ALuint      emitter_index;  // Unique emitter index.
    ALuint      sound_index;    // Sound index.
    btVector3   position;    // Vector coordinate.
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
};

} // namespace audio
