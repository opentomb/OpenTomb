#include "source.h"

#include "alext.h"
#include "engine/engine.h"
#include "fxmanager.h"
#include "settings.h"
#include "util/helpers.h"
#include "world/entity.h"

#include <glm/gtc/type_ptr.hpp>

namespace audio
{

Source::Source()
{
    m_active = false;
    m_emitterID = -1;
    m_emitterType = EmitterType::Entity;
    m_effectIndex = 0;
    m_sampleIndex = 0;
    m_sampleCount = 0;
    m_isWater = false;
    alGenSources(1, &m_sourceIndex);

    if(alIsSource(m_sourceIndex))
    {
        alSourcef(m_sourceIndex, AL_MIN_GAIN, 0.0);
        alSourcef(m_sourceIndex, AL_MAX_GAIN, 1.0);

        if(engine::engine_world.audioEngine.getSettings().use_effects)
        {
            alSourcef(m_sourceIndex, AL_ROOM_ROLLOFF_FACTOR, 1.0);
            alSourcei(m_sourceIndex, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_TRUE);
            alSourcei(m_sourceIndex, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
            alSourcef(m_sourceIndex, AL_AIR_ABSORPTION_FACTOR, 0.1f);
        }
        else
        {
            alSourcef(m_sourceIndex, AL_AIR_ABSORPTION_FACTOR, 0.0f);
        }
    }
}

Source::~Source()
{
    if(alIsSource(m_sourceIndex))
    {
        alSourceStop(m_sourceIndex);
        alDeleteSources(1, &m_sourceIndex);
    }
}

bool Source::isActive() const
{
    return m_active;
}

bool Source::isLooping() const
{
    if(alIsSource(m_sourceIndex))
    {
        ALint looping;
        alGetSourcei(m_sourceIndex, AL_LOOPING, &looping);
        return (looping != AL_FALSE);
    }
    else
    {
        return false;
    }
}

bool Source::isPlaying() const
{
    if(alIsSource(m_sourceIndex))
    {
        ALenum state = AL_STOPPED;
        alGetSourcei(m_sourceIndex, AL_SOURCE_STATE, &state);

        // Paused state and existing file pointers also counts as playing.
        return ((state == AL_PLAYING) || (state == AL_PAUSED));
    }
    else
    {
        return false;
    }
}

void Source::play(FxManager& manager)
{
    if(alIsSource(m_sourceIndex))
    {
        if(m_emitterType == EmitterType::Global)
        {
            alSourcei(m_sourceIndex, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(m_sourceIndex, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(m_sourceIndex, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            if(engine::engine_world.audioEngine.getSettings().use_effects)
            {
                unsetFX();
            }
        }
        else
        {
            alSourcei(m_sourceIndex, AL_SOURCE_RELATIVE, AL_FALSE);
            linkEmitter();

            if(engine::engine_world.audioEngine.getSettings().use_effects)
            {
                setFX(manager);
                setUnderwater(manager);
            }
        }

        alSourcePlay(m_sourceIndex);
        m_active = true;
    }
}

void Source::pause()
{
    if(alIsSource(m_sourceIndex))
    {
        alSourcePause(m_sourceIndex);
    }
}

void Source::stop()
{
    if(alIsSource(m_sourceIndex))
    {
        alSourceStop(m_sourceIndex);
    }
}

void Source::update(const FxManager& manager)
{
    // Bypass any non-active source.
    if(!m_active)
        return;

    // Disable and bypass source, if it is stopped.
    if(!isPlaying())
    {
        m_active = false;
        return;
    }

    // Bypass source, if it is global.
    if(m_emitterType == EmitterType::Global)
        return;

    ALfloat range, gain;

    alGetSourcef(m_sourceIndex, AL_GAIN, &gain);
    alGetSourcef(m_sourceIndex, AL_MAX_DISTANCE, &range);

    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.

    if(engine::engine_world.audioEngine.isInRange(m_emitterType, m_emitterID, range, gain))
    {
        linkEmitter();

        if(engine::engine_world.audioEngine.getSettings().use_effects && m_isWater != manager.water_state)
        {
            setUnderwater(manager);
        }
    }
    else
    {
        // Immediately stop source only if track is looped. It allows sounds which
        // were activated for already destroyed entities to finish (e.g. grenade
        // explosions, ricochets, and so on).

        if(isLooping()) stop();
    }
}

void Source::setBuffer(ALint buffer)
{
    ALuint buffer_index = engine::engine_world.audioEngine.getBuffer(buffer);

    if(alIsSource(m_sourceIndex) && alIsBuffer(buffer_index))
    {
        alSourcei(m_sourceIndex, AL_BUFFER, buffer_index);

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

void Source::setLooping(ALboolean is_looping)
{
    alSourcei(m_sourceIndex, AL_LOOPING, is_looping);
}

void Source::setGain(ALfloat gain_value)
{
    alSourcef(m_sourceIndex, AL_GAIN, glm::clamp(gain_value, 0.0f, 1.0f) * engine::engine_world.audioEngine.getSettings().sound_volume);
}

void Source::setPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    alSourcef(m_sourceIndex, AL_PITCH, glm::clamp(pitch_value, 0.1f, 2.0f));
}

void Source::setRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(m_sourceIndex, AL_REFERENCE_DISTANCE, range_value / 6.0f);
    alSourcef(m_sourceIndex, AL_MAX_DISTANCE, range_value);
}

void Source::setPosition(const ALfloat pos_vector[])
{
    alSourcefv(m_sourceIndex, AL_POSITION, pos_vector);
}

void Source::setVelocity(const ALfloat vel_vector[])
{
    alSourcefv(m_sourceIndex, AL_VELOCITY, vel_vector);
}

void Source::setFX(FxManager& manager)
{
    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    ALuint slot;
    if(manager.current_room_type != manager.last_room_type)  // Switch audio send.
    {
        manager.last_room_type = manager.current_room_type;
        manager.current_slot = (++manager.current_slot > (FxManager::MaxSlots - 1)) ? (0) : (manager.current_slot);

        ALuint effect = manager.al_effect[static_cast<int>(manager.current_room_type)];
        slot = manager.al_slot[manager.current_slot];

        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = manager.al_slot[manager.current_slot];
    }

    // Assign global reverb FX to channel.

    alSource3i(m_sourceIndex, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}

void Source::unsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(m_sourceIndex, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(m_sourceIndex, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
}

void Source::setUnderwater(const FxManager& fxManager)
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(fxManager.water_state)
    {
        alSourcei(m_sourceIndex, AL_DIRECT_FILTER, fxManager.al_filter);
        m_isWater = true;
    }
    else
    {
        alSourcei(m_sourceIndex, AL_DIRECT_FILTER, AL_FILTER_NULL);
        m_isWater = false;
    }
}

void Source::linkEmitter()
{
    switch(m_emitterType)
    {
        case EmitterType::Any:
            BOOST_ASSERT(false);
            break;

        case EmitterType::Global:
            break;

        case EmitterType::Entity:
            if(std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(m_emitterID))
            {
                setPosition(glm::value_ptr(ent->m_transform[3]));
                setVelocity(glm::value_ptr(ent->m_speed));
            }
            break;

        case EmitterType::SoundSource:
            setPosition(glm::value_ptr(engine::engine_world.audioEngine.getEmitter(m_emitterID).position));
            break;
    }
}

} // namespace audio
