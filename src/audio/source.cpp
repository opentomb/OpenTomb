#include "source.h"

#include "engine/engine.h"
#include "util/helpers.h"
#include "world/entity.h"

namespace audio
{

// FIXME Shouldn't be global
extern Settings audio_settings;
// FIXME This shouldn't be global
extern FxManager fxManager;

Source::~Source()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
        alDeleteSources(1, &source_index);
    }
}

bool Source::IsActive()
{
    return active;
}

bool Source::IsLooping()
{
    if(alIsSource(source_index))
    {
        ALint looping;
        alGetSourcei(source_index, AL_LOOPING, &looping);
        return (looping != AL_FALSE);
    }
    else
    {
        return false;
    }
}

bool Source::IsPlaying()
{
    if(alIsSource(source_index))
    {
        ALenum state = AL_STOPPED;
        alGetSourcei(source_index, AL_SOURCE_STATE, &state);

        // Paused state and existing file pointers also counts as playing.
        return ((state == AL_PLAYING) || (state == AL_PAUSED));
    }
    else
    {
        return false;
    }
}

void Source::Play()
{
    if(alIsSource(source_index))
    {
        if(emitter_type == EmitterType::Global)
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(source_index, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(source_index, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            if(audio_settings.use_effects)
            {
                UnsetFX();
            }
        }
        else
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_FALSE);
            LinkEmitter();

            if(audio_settings.use_effects)
            {
                SetFX();
                SetUnderwater();
            }
        }

        alSourcePlay(source_index);
        active = true;
    }
}

void Source::Pause()
{
    if(alIsSource(source_index))
    {
        alSourcePause(source_index);
    }
}

void Source::Stop()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
    }
}

void Source::Update()
{
    // Bypass any non-active source.
    if(!active) return;

    // Disable and bypass source, if it is stopped.
    if(!IsPlaying())
    {
        active = false;
        return;
    }

    // Bypass source, if it is global.
    if(emitter_type == EmitterType::Global) return;

    ALfloat range, gain;

    alGetSourcef(source_index, AL_GAIN, &gain);
    alGetSourcef(source_index, AL_MAX_DISTANCE, &range);

    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.

    if(isInRange(emitter_type, emitter_ID, range, gain))
    {
        LinkEmitter();

        if(audio_settings.use_effects && is_water != fxManager.water_state)
        {
            SetUnderwater();
        }
    }
    else
    {
        // Immediately stop source only if track is looped. It allows sounds which
        // were activated for already destroyed entities to finish (e.g. grenade
        // explosions, ricochets, and so on).

        if(IsLooping()) Stop();
    }
}

void Source::SetBuffer(ALint buffer)
{
    ALint buffer_index = engine::engine_world.audio_buffers[buffer];

    if(alIsSource(source_index) && alIsBuffer(buffer_index))
    {
        alSourcei(source_index, AL_BUFFER, buffer_index);

        // For some reason, OpenAL sometimes produces "Invalid Operation" error here,
        // so there's extra debug info - maybe it'll help some day.

        /*
        if(Audio_LogALError(1))
        {
            int channels, bits, freq;

            alGetBufferi(buffer_index, AL_CHANNELS,  &channels);
            alGetBufferi(buffer_index, AL_BITS,      &bits);
            alGetBufferi(buffer_index, AL_FREQUENCY, &freq);

            Sys_DebugLog(LOG_FILENAME, "Faulty buffer %d info: CH%d, B%d, F%d", buffer_index, channels, bits, freq);
        }
        */
    }
}

void Source::SetLooping(ALboolean is_looping)
{
    alSourcei(source_index, AL_LOOPING, is_looping);
}

void Source::SetGain(ALfloat gain_value)
{
    alSourcef(source_index, AL_GAIN, util::clamp(gain_value, 0.0f, 1.0f) * audio_settings.sound_volume);
}

void Source::SetPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    alSourcef(source_index, AL_PITCH, util::clamp(pitch_value, 0.1f, 2.0f));
}

void Source::SetRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(source_index, AL_REFERENCE_DISTANCE, range_value / 6.0f);
    alSourcef(source_index, AL_MAX_DISTANCE, range_value);
}

void Source::SetPosition(const ALfloat pos_vector[])
{
    alSourcefv(source_index, AL_POSITION, pos_vector);
}

void Source::SetVelocity(const ALfloat vel_vector[])
{
    alSourcefv(source_index, AL_VELOCITY, vel_vector);
}

void Source::SetFX()
{
    ALuint effect;
    ALuint slot;

    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    if(fxManager.current_room_type != fxManager.last_room_type)  // Switch audio send.
    {
        fxManager.last_room_type = fxManager.current_room_type;
        fxManager.current_slot = (++fxManager.current_slot > (MaxSlots - 1)) ? (0) : (fxManager.current_slot);

        effect = fxManager.al_effect[fxManager.current_room_type];
        slot = fxManager.al_slot[fxManager.current_slot];

        assert(alIsAuxiliaryEffectSlot != nullptr);
        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = fxManager.al_slot[fxManager.current_slot];
    }

    // Assign global reverb FX to channel.

    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}

void Source::UnsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
}

void Source::SetUnderwater()
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(fxManager.water_state)
    {
        alSourcei(source_index, AL_DIRECT_FILTER, fxManager.al_filter);
        is_water = true;
    }
    else
    {
        alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
        is_water = false;
    }
}

void Source::LinkEmitter()
{
    switch(emitter_type)
    {
        case EmitterType::Entity:
            if(std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(emitter_ID))
            {
                SetPosition(ent->m_transform.getOrigin());
                SetVelocity(ent->m_speed);
            }
            return;

        case EmitterType::SoundSource:
            SetPosition(engine::engine_world.audio_emitters[emitter_ID].position);
            return;
    }
}

} // namespace audio
