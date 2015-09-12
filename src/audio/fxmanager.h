#pragma once

#include <array>

#include <AL/al.h>
#include <AL/efx-presets.h>

namespace world
{
class Camera;
struct Character;
}

namespace audio
{

// In TR3-5, there were 5 reverb / echo effect flags for each
// room, but they were never used in PC versions - however, level
// files still contain this info, so we now can re-use these flags
// to assign reverb/echo presets to each room.
// Also, underwater environment can be considered as additional
// reverb flag, so overall amount is 6.

enum TR_AUDIO_FX
{
    TR_AUDIO_FX_OUTSIDE,         // EFX_REVERB_PRESET_CITY
    TR_AUDIO_FX_SMALLROOM,       // EFX_REVERB_PRESET_LIVINGROOM
    TR_AUDIO_FX_MEDIUMROOM,      // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
    TR_AUDIO_FX_LARGEROOM,       // EFX_REVERB_PRESET_DOME_TOMB
    TR_AUDIO_FX_PIPE,            // EFX_REVERB_PRESET_PIPE_LARGE
    TR_AUDIO_FX_WATER,           // EFX_REVERB_PRESET_UNDERWATER
    TR_AUDIO_FX_LASTINDEX
};

// FX manager structure.
// It contains all necessary info to process sample FX (reverb and echo).
struct FxManager
{
    // MAX_SLOTS specifies amount of FX slots used to apply environmental
    // effects to sounds. We need at least two of them to prevent glitches
    // at environment transition (slots are cyclically changed, leaving
    // previously played samples at old slot). Maximum amount is 4, but
    // it's not recommended to set it more than 2.

    static constexpr int MaxSlots = 2;

    ALuint      al_filter = 0;
    std::array<ALuint,TR_AUDIO_FX_LASTINDEX> al_effect;
    std::array<ALuint,MaxSlots> al_slot;
    ALuint      current_slot = 0;
    ALuint      current_room_type = 0;
    ALuint      last_room_type = 0;
    bool        water_state = false;    // If listener is underwater, all samples will damp.

    ~FxManager();

    bool loadReverb(int effect_index, const EFXEAXREVERBPROPERTIES *reverb);
    void updateListener(world::Camera *cam);
    void updateListener(world::Character* ent);

    explicit FxManager() = default;
    explicit FxManager(bool); // Bool param only used for distinguishing from default constructor
};

} // namespace audio
