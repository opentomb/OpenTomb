#pragma once

#include "loader/datatypes.h"

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
    std::array<ALuint, static_cast<int>(loader::ReverbType::Sentinel)> al_effect;
    std::array<ALuint,MaxSlots> al_slot;
    ALuint      current_slot = 0;
    loader::ReverbType current_room_type = loader::ReverbType::Outside;
    loader::ReverbType last_room_type = loader::ReverbType::Outside;
    bool        water_state = false;    // If listener is underwater, all samples will damp.

    ~FxManager();

    bool loadReverb(loader::ReverbType effect_index, const EFXEAXREVERBPROPERTIES *reverb);
    void updateListener(world::Camera& cam);
    void updateListener(world::Character& ent);

    explicit FxManager() = default;
    explicit FxManager(bool); // Bool param only used for distinguishing from default constructor
};

} // namespace audio
