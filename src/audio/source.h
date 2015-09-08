#pragma once

#include "audio/audio.h"

#include <AL/al.h>

#include <cstdint>

namespace audio
{
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

    void Play();    // Make source active and play it.
    void Pause();   // Pause source (leaving it active).
    void Stop();    // Stop and destroy source.
    void Update();  // Update source parameters.

    void SetBuffer(ALint buffer);           // Assign buffer to source.
    void SetLooping(ALboolean is_looping);  // Set looping flag.
    void SetPitch(ALfloat pitch_value);     // Set pitch shift.
    void SetGain(ALfloat gain_value);       // Set gain (volume).
    void SetRange(ALfloat range_value);     // Set max. audible distance.
    void SetFX();                           // Set reverb FX, according to room flag.
    void UnsetFX();                         // Remove any reverb FX from source.
    void SetUnderwater();                   // Apply low-pass underwater filter.

    bool IsLooping();           // Check if source is looping;
    bool IsPlaying();           // Check if source is currently playing.
    bool IsActive();            // Check if source is active.

    int32_t     emitter_ID;     // Entity of origin. -1 means no entity (hence - empty source).
    EmitterType emitter_type;   // 0 - ordinary entity, 1 - sound source, 2 - global sound.
    uint32_t    effect_index;   // Effect index. Used to associate effect with entity for R/W flags.
    uint32_t    sample_index;   // OpenAL sample (buffer) index. May be the same for different sources.
    uint32_t    sample_count;   // How many buffers to use, beginning with sample_index.

    friend int isEffectPlaying(int effect_ID, int entity_type, int entity_ID);

private:
    bool        active;         // Source gets autostopped and destroyed on next frame, if it's not set.
    bool        is_water;       // Marker to define if sample is in underwater state or not.
    ALuint      source_index = 0;   // Source index. Should be unique for each source.

    void LinkEmitter();                             // Link source to parent emitter.
    void SetPosition(const ALfloat pos_vector[]);   // Set source position.
    void SetVelocity(const ALfloat vel_vector[]);   // Set source velocity (speed).
};

int  getFreeSource();

bool isInRange(EmitterType entity_type, int entity_ID, float range, float gain);
int  isEffectPlaying(int effect_ID = -1, EmitterType entity_type = EmitterType::Any, int entity_ID = -1);

Error send(int effect_ID, EmitterType entity_type = EmitterType::Global, int entity_ID = 0);    // Send to play effect with given parameters.
Error kill(int effect_ID, EmitterType entity_type = EmitterType::Global, int entity_ID = 0);    // If exist, immediately stop and destroy all effects with given parameters.
} // namespace audio
