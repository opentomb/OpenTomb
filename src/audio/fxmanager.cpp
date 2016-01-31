#include "fxmanager.h"

#include "alext.h"
#include "audio.h"
#include "engine/engine.h"
#include "world/camera.h"
#include "world/room.h"

#include <glm/gtc/type_ptr.hpp>
#include <boost/log/trivial.hpp>

namespace audio
{
FxManager::~FxManager() noexcept
{
    for(auto slot : m_slots)
    {
        if(slot == 0)
            continue;

        alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
        DEBUG_CHECK_AL_ERROR();
        alDeleteAuxiliaryEffectSlots(1, &slot);
        DEBUG_CHECK_AL_ERROR();
    }

    alDeleteFilters(1, &m_filter);
    DEBUG_CHECK_AL_ERROR();
    alDeleteEffects(m_effects.size(), m_effects.data());
    DEBUG_CHECK_AL_ERROR();
}

bool FxManager::loadReverb(loader::ReverbType effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    BOOST_ASSERT(effect_index >= loader::ReverbType::Outside && effect_index < loader::ReverbType::Sentinel);
    ALuint effect = m_effects[static_cast<int>(effect_index)];

    if(alIsEffect(effect))
    {
        alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
        DEBUG_CHECK_AL_ERROR();

        alEffectf(effect, AL_REVERB_DENSITY, reverb->flDensity);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_DIFFUSION, reverb->flDiffusion);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_GAIN, reverb->flGain);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_GAINHF, reverb->flGainHF);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_DECAY_TIME, reverb->flDecayTime);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
        DEBUG_CHECK_AL_ERROR();
        alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
        DEBUG_CHECK_AL_ERROR();
        alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
        DEBUG_CHECK_AL_ERROR();

        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "OpenAL error: no effect " << effect;
        return false;
    }
}

FxManager::FxManager(Engine* engine)
    : m_engine(engine)
{
    if(!engine->getSettings().use_effects)
        return;

    m_effects.fill(0);
    m_slots.fill(0);
    alGenAuxiliaryEffectSlots(MaxSlots, m_slots.data());
    DEBUG_CHECK_AL_ERROR();
    alGenEffects(m_effects.size(), m_effects.data());
    DEBUG_CHECK_AL_ERROR();
    alGenFilters(1, &m_filter);
    DEBUG_CHECK_AL_ERROR();

    alFilteri(m_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    DEBUG_CHECK_AL_ERROR();
    alFilterf(m_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    DEBUG_CHECK_AL_ERROR();
    alFilterf(m_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.
    DEBUG_CHECK_AL_ERROR();

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    loadReverb(loader::ReverbType::Outside, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    loadReverb(loader::ReverbType::SmallRoom, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    loadReverb(loader::ReverbType::MediumRoom, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    loadReverb(loader::ReverbType::LargeRoom, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    loadReverb(loader::ReverbType::Pipe, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    loadReverb(loader::ReverbType::Water, &reverb6);
}

/**
 * Updates listener parameters by camera structure. For correct speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void FxManager::updateListener(world::Camera& cam)
{
    ALfloat v[6] = {
        cam.getViewDir()[0], cam.getViewDir()[1], cam.getViewDir()[2],
        cam.getUpDir()[0], cam.getUpDir()[1], cam.getUpDir()[2]
    };

    alListenerfv(AL_ORIENTATION, v);
    DEBUG_CHECK_AL_ERROR();

    alListenerfv(AL_POSITION, glm::value_ptr(cam.getPosition()));
    DEBUG_CHECK_AL_ERROR();

    glm::vec3 v2 = cam.getMovement() / m_engine->getEngine()->getFrameTimeSecs();
    alListenerfv(AL_VELOCITY, glm::value_ptr(v2));
    DEBUG_CHECK_AL_ERROR();
    cam.resetMovement();

    if(!cam.getCurrentRoom())
        return;

    if(cam.getCurrentRoom()->getFlags() & TR_ROOM_FLAG_WATER)
    {
        m_currentRoomType = loader::ReverbType::Water;
    }
    else
    {
        m_currentRoomType = cam.getCurrentRoom()->getReverbType();
    }

    if(m_underwater != static_cast<bool>(cam.getCurrentRoom()->getFlags() & TR_ROOM_FLAG_WATER))
    {
        m_underwater = (cam.getCurrentRoom()->getFlags() & TR_ROOM_FLAG_WATER) != 0;

        if(m_underwater)
        {
            m_engine->send(audio::SoundUnderwater);
        }
        else
        {
            m_engine->kill(audio::SoundUnderwater);
        }
    }
}

void FxManager::updateListener(world::Character& /*ent*/)
{
    ///@FIXME: Add entity listener updater here.
}

ALuint FxManager::allocateSlot()
{
    if(m_currentRoomType != m_lastRoomType)  // Switch audio send.
    {
        m_lastRoomType = m_currentRoomType;
        ++m_currentSlot;
        if(m_currentSlot >= m_slots.size())
            m_currentSlot = 0;

        ALuint effect = m_effects[static_cast<int>(m_currentRoomType)];
        ALuint slot = m_slots[m_currentSlot];

        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
            DEBUG_CHECK_AL_ERROR();
        }
        return slot;
    }
    else    // Do not switch audio send.
    {
        return m_slots[m_currentSlot];
    }
}
}
