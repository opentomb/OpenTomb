#pragma once

#include "loader/datatypes.h"

#include <array>

#include <AL/al.h>
#include <AL/efx-presets.h>

namespace world
{
class Camera;
class Character;
}

namespace engine
{
class Engine;
}

namespace audio
{
class Engine;

// FX manager structure.
// It contains all necessary info to process sample FX (reverb and echo).
class FxManager
{
    DISABLE_COPY(FxManager);
    TRACK_LIFETIME();

    // MAX_SLOTS specifies amount of FX slots used to apply environmental
    // effects to sounds. We need at least two of them to prevent glitches
    // at environment transition (slots are cyclically changed, leaving
    // previously played samples at old slot). Maximum amount is 4, but
    // it's not recommended to set it more than 2.

    static constexpr int MaxSlots = 2;

    Engine* m_engine;
    ALuint m_filter = 0;
    std::array<ALuint, static_cast<int>(loader::ReverbType::Sentinel)> m_effects;
    std::array<ALuint, MaxSlots> m_slots;
    ALuint m_currentSlot = 0;
    loader::ReverbType m_currentRoomType = loader::ReverbType::Outside;
    loader::ReverbType m_lastRoomType = loader::ReverbType::Outside;
    bool m_underwater = false;    // If listener is underwater, all samples will damp.

public:
    explicit FxManager(Engine* engine);
    
    ~FxManager() noexcept;

    bool loadReverb(loader::ReverbType effect_index, const EFXEAXREVERBPROPERTIES *reverb);
    void updateListener(world::Camera& cam);
    void updateListener(world::Character& ent);
    ALuint allocateSlot();

    bool isUnderwater() const noexcept
    {
        return m_underwater;
    }

    void resetLastRoomType() noexcept
    {
        m_lastRoomType = loader::ReverbType::Sentinel;
    }

    ALuint getFilter() const noexcept
    {
        return m_filter;
    }
};
} // namespace audio
