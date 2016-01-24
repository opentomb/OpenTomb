#pragma once

#include "audio.h"

#include <AL/al.h>
#include <glm/glm.hpp>

namespace audio
{
// Audio emitter (aka SoundSource) structure.
struct Emitter
{
    ALuint      emitter_index;  // Unique emitter index.
    SoundId     soundId;
    glm::vec3   position;    // Vector coordinate.
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
};
} // namespace audio
