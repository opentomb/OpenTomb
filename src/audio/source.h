#pragma once

#include "world/object.h"

#include <AL/al.h>

#include <boost/optional.hpp>

#include <cstdint>

namespace audio
{
class FxManager;

// Entity types are used to identify different sound emitter types. Since
// sounds in TR games could be emitted either by entities, sound sources
// or global events, we have defined these three types of emitters.
enum class EmitterType
{
    Any,
    Entity,   // Entity (movable)
    SoundSource,   // Sound source (static)
    Global   // Global sound (menus, secret, etc.)
};

// Sound source is a complex class, each member of which is linked with
// certain in-game entity or sound source, but also a kind of entity by itself.
// Number of simultaneously existing sound sources is fixed, and can't be more than
// MAX_CHANNELS global constant.
class Source
{
public:
    Source();  // Audio source constructor.
    ~Source();  // Audio source destructor.

    void play(FxManager& manager);    // Make source active and play it.
    void pause();   // Pause source (leaving it active).
    void stop();    // Stop and destroy source.
    void update(const FxManager &manager);  // Update source parameters.

    void setBuffer(ALint buffer);           // Assign buffer to source.
    void setLooping(ALboolean is_looping);  // Set looping flag.
    void setPitch(ALfloat pitch_value);     // Set pitch shift.
    void setGain(ALfloat gain_value);       // Set gain (volume).
    void setRange(ALfloat range_value);     // Set max. audible distance.
    void setFX(FxManager &manager);                           // Set reverb FX, according to room flag.
    void unsetFX();                         // Remove any reverb FX from source.
    void setUnderwater(const FxManager &fxManager);                   // Apply low-pass underwater filter.

    bool isLooping() const;           // Check if source is looping;
    bool isPlaying() const;           // Check if source is currently playing.
    bool isActive() const;            // Check if source is active.

    boost::optional<world::ObjectId> m_emitterID = boost::none;     // Entity of origin. -1 means no entity (hence - empty source).
    EmitterType m_emitterType = EmitterType::Entity;   // 0 - ordinary entity, 1 - sound source, 2 - global sound.
    uint32_t    m_effectIndex = 0;   // Effect index. Used to associate effect with entity for R/W flags.
    uint32_t    m_sampleIndex = 0;   // OpenAL sample (buffer) index. May be the same for different sources.
    uint32_t    m_sampleCount = 0;   // How many buffers to use, beginning with sample_index.

    friend int isEffectPlaying(int effect_ID, int entity_type, int entity_ID);

private:
    bool        m_active = false;         // Source gets autostopped and destroyed on next frame, if it's not set.
    bool        m_underwater = false;       // Marker to define if sample is in underwater state or not.
    ALuint      m_sourceIndex = 0;   // Source index. Should be unique for each source.

    void linkEmitter();                             // Link source to parent emitter.
    void setPosition(const ALfloat pos_vector[]);   // Set source position.
    void setVelocity(const ALfloat vel_vector[]);   // Set source velocity (speed).
};

} // namespace audio
