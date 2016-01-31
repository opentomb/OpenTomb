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
Source::Source(audio::Engine* engine)
    : m_audioEngine(engine)
{
    alGenSources(1, &m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();

    if(!alIsSource(m_sourceIndex))
        return;

    alSourcef(m_sourceIndex, AL_MIN_GAIN, 0.0);
    DEBUG_CHECK_AL_ERROR();
    alSourcef(m_sourceIndex, AL_MAX_GAIN, 1.0);
    DEBUG_CHECK_AL_ERROR();

    if(m_audioEngine->getSettings().use_effects)
    {
        alSourcef(m_sourceIndex, AL_ROOM_ROLLOFF_FACTOR, 1.0);
        DEBUG_CHECK_AL_ERROR();
        alSourcei(m_sourceIndex, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_TRUE);
        DEBUG_CHECK_AL_ERROR();
        alSourcei(m_sourceIndex, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
        DEBUG_CHECK_AL_ERROR();
        alSourcef(m_sourceIndex, AL_AIR_ABSORPTION_FACTOR, 0.1f);
        DEBUG_CHECK_AL_ERROR();
    }
    else
    {
        alSourcef(m_sourceIndex, AL_AIR_ABSORPTION_FACTOR, 0.0f);
        DEBUG_CHECK_AL_ERROR();
    }
}

Source::~Source()
{
    if(!alIsSource(m_sourceIndex))
        return;

    alSourceStop(m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();
    alDeleteSources(1, &m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();
}

bool Source::isActive() const
{
    return m_active;
}

bool Source::isLooping() const
{
    if(!alIsSource(m_sourceIndex))
        return false;

    ALint looping;
    alGetSourcei(m_sourceIndex, AL_LOOPING, &looping);
    DEBUG_CHECK_AL_ERROR();
    return looping != AL_FALSE;
}

bool Source::isPlaying() const
{
    if(!alIsSource(m_sourceIndex))
        return false;

    ALenum state = AL_STOPPED;
    alGetSourcei(m_sourceIndex, AL_SOURCE_STATE, &state);
    DEBUG_CHECK_AL_ERROR();

    // Paused state and existing file pointers also counts as playing.
    return state == AL_PLAYING || state == AL_PAUSED;
}

void Source::play(FxManager& manager, const world::World& world)
{
    if(!alIsSource(m_sourceIndex))
        return;

    if(m_emitterType == EmitterType::Global)
    {
        alSourcei(m_sourceIndex, AL_SOURCE_RELATIVE, AL_TRUE);
        DEBUG_CHECK_AL_ERROR();
        alSource3f(m_sourceIndex, AL_POSITION, 0.0f, 0.0f, 0.0f);
        DEBUG_CHECK_AL_ERROR();
        alSource3f(m_sourceIndex, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        DEBUG_CHECK_AL_ERROR();

        if(m_audioEngine->getSettings().use_effects)
        {
            unsetFX();
        }
    }
    else
    {
        alSourcei(m_sourceIndex, AL_SOURCE_RELATIVE, AL_FALSE);
        DEBUG_CHECK_AL_ERROR();
        linkEmitter(world);

        if(m_audioEngine->getSettings().use_effects)
        {
            setFX(manager);
            setUnderwater(manager);
        }
    }

    alSourcePlay(m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();
    m_active = true;
}

void Source::pause()
{
    if(!alIsSource(m_sourceIndex))
        return;

    alSourcePause(m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();
}

void Source::stop()
{
    if(!alIsSource(m_sourceIndex))
        return;

    alSourceStop(m_sourceIndex);
    DEBUG_CHECK_AL_ERROR();
}

void Source::update(const FxManager& manager, const world::World& world)
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
    DEBUG_CHECK_AL_ERROR();
    alGetSourcef(m_sourceIndex, AL_MAX_DISTANCE, &range);
    DEBUG_CHECK_AL_ERROR();

    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.

    if(m_audioEngine->isInRange(m_emitterType, m_emitterId, range, gain))
    {
        linkEmitter(world);

        if(m_audioEngine->getSettings().use_effects && m_underwater != manager.isUnderwater())
        {
            setUnderwater(manager);
        }
    }
    else
    {
        // Immediately stop source only if track is looped. It allows sounds which
        // were activated for already destroyed entities to finish (e.g. grenade
        // explosions, ricochets, and so on).

        if(isLooping())
            stop();
    }
}

void Source::setBuffer(ALint buffer)
{
    ALuint buffer_index = m_audioEngine->getBuffer(buffer);

    if(!alIsSource(m_sourceIndex) || !alIsBuffer(buffer_index))
        return;

    alSourcei(m_sourceIndex, AL_BUFFER, buffer_index);
    DEBUG_CHECK_AL_ERROR();

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

void Source::setLooping(ALboolean is_looping)
{
    alSourcei(m_sourceIndex, AL_LOOPING, is_looping);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setGain(ALfloat gain_value)
{
    alSourcef(m_sourceIndex, AL_GAIN, glm::clamp(gain_value, 0.0f, 1.0f) * m_audioEngine->getSettings().sound_volume);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    alSourcef(m_sourceIndex, AL_PITCH, glm::clamp(pitch_value, 0.1f, 2.0f));
    DEBUG_CHECK_AL_ERROR();
}

void Source::setRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(m_sourceIndex, AL_REFERENCE_DISTANCE, range_value / 6.0f);
    DEBUG_CHECK_AL_ERROR();
    alSourcef(m_sourceIndex, AL_MAX_DISTANCE, range_value);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setPosition(const ALfloat pos_vector[])
{
    alSourcefv(m_sourceIndex, AL_POSITION, pos_vector);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setVelocity(const ALfloat vel_vector[])
{
    alSourcefv(m_sourceIndex, AL_VELOCITY, vel_vector);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setFX(FxManager& manager)
{
    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    ALuint slot = manager.allocateSlot();

    // Assign global reverb FX to channel.

    alSource3i(m_sourceIndex, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
    DEBUG_CHECK_AL_ERROR();
}

void Source::unsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(m_sourceIndex, AL_DIRECT_FILTER, AL_FILTER_NULL);
    DEBUG_CHECK_AL_ERROR();
    alSource3i(m_sourceIndex, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
    DEBUG_CHECK_AL_ERROR();
}

void Source::setUnderwater(const FxManager& fxManager)
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(fxManager.isUnderwater())
    {
        alSourcei(m_sourceIndex, AL_DIRECT_FILTER, fxManager.getFilter());
        DEBUG_CHECK_AL_ERROR();
        m_underwater = true;
    }
    else
    {
        alSourcei(m_sourceIndex, AL_DIRECT_FILTER, AL_FILTER_NULL);
        DEBUG_CHECK_AL_ERROR();
        m_underwater = false;
    }
}

void Source::linkEmitter(const world::World& world)
{
    BOOST_ASSERT(m_emitterId.is_initialized());

    switch(m_emitterType)
    {
        case EmitterType::Any:
            BOOST_ASSERT(false);
            break;

        case EmitterType::Global:
            break;

        case EmitterType::Entity:
            if(std::shared_ptr<world::Entity> ent = world.getEntityByID(*m_emitterId))
            {
                setPosition(glm::value_ptr(ent->m_transform[3]));
                setVelocity(glm::value_ptr(ent->m_speed));
            }
            break;

        case EmitterType::SoundSource:
            setPosition(glm::value_ptr(m_audioEngine->getEmitter(*m_emitterId).position));
            break;
    }
}
} // namespace audio