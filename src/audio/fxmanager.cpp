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
        alDeleteAuxiliaryEffectSlots(1, &slot);
    }

    alDeleteFilters(1, &m_filter);
    alDeleteEffects(m_effects.size(), m_effects.data());
}

bool FxManager::loadReverb(loader::ReverbType effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    BOOST_ASSERT(effect_index>=loader::ReverbType::Outside && effect_index < loader::ReverbType::Sentinel);
    ALuint effect = m_effects[static_cast<int>(effect_index)];

    if(alIsEffect(effect))
    {
        alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

        alEffectf(effect, AL_REVERB_DENSITY, reverb->flDensity);
        alEffectf(effect, AL_REVERB_DIFFUSION, reverb->flDiffusion);
        alEffectf(effect, AL_REVERB_GAIN, reverb->flGain);
        alEffectf(effect, AL_REVERB_GAINHF, reverb->flGainHF);
        alEffectf(effect, AL_REVERB_DECAY_TIME, reverb->flDecayTime);
        alEffectf(effect, AL_REVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
        alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
        alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
        alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
        alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
        alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
        alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
        alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);

        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "OpenAL error: no effect " << effect;
        return false;
    }
}

FxManager::FxManager(bool)
{
    m_effects.fill(0);
    m_slots.fill(0);
    alGenAuxiliaryEffectSlots(MaxSlots, m_slots.data());
    alGenEffects(m_effects.size(), m_effects.data());
    alGenFilters(1, &m_filter);

    alFilteri(m_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(m_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(m_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

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

    alListenerfv(AL_POSITION, glm::value_ptr(cam.getPosition()));

    glm::vec3 v2 = cam.getMovement() / util::toSeconds(engine::engine_frame_time);
    alListenerfv(AL_VELOCITY, glm::value_ptr(v2));
    cam.resetMovement();

    if(!cam.getCurrentRoom())
        return;

    if(cam.getCurrentRoom()->m_flags & TR_ROOM_FLAG_WATER)
    {
        m_currentRoomType = loader::ReverbType::Water;
    }
    else
    {
        m_currentRoomType = cam.getCurrentRoom()->m_reverbType;
    }

    if(m_underwater != static_cast<bool>(cam.getCurrentRoom()->m_flags & TR_ROOM_FLAG_WATER))
    {
        m_underwater = (cam.getCurrentRoom()->m_flags & TR_ROOM_FLAG_WATER) != 0;

        if(m_underwater)
        {
            engine::engine_world.audioEngine.send(audio::SoundUnderwater);
        }
        else
        {
            engine::engine_world.audioEngine.kill(audio::SoundUnderwater);
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
        }
        return slot;
    }
    else    // Do not switch audio send.
    {
        return m_slots[m_currentSlot];
    }
}

}
