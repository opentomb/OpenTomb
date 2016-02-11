#pragma once

#include "util/helpers.h"

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

    explicit Settings(boost::property_tree::ptree& config)
    {
        music_volume = util::getSetting(config, "musicVolume", 0.7f);
        sound_volume = util::getSetting(config, "soundVolume", 0.8f);
        use_effects = util::getSetting(config, "useEffects", true);
        listener_is_player = util::getSetting(config, "listenerIsPlayer", false);
        stream_buffer_size = util::getSetting(config, "streamBufferSize", 32) * 1024;
        if(stream_buffer_size <= 0)
            stream_buffer_size = 128 * 1024;
    }
};
} // namespace audio
