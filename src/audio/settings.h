#pragma once

#include <AL/al.h>

namespace audio
{

// Audio settings structure.
struct Settings
{
    ALfloat     music_volume = 0.7f;
    ALfloat     sound_volume = 0.8f;
    bool        use_effects = true;
    bool        listener_is_player = false; // RESERVED FOR FUTURE USE
    int         stream_buffer_size = 32;
};

} // namespace audio
