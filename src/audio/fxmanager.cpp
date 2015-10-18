#include "fxmanager.h"

#include "alext.h"
#include "audio.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "world/camera.h"
#include "world/room.h"

#include <glm/gtc/type_ptr.hpp>
#include <boost/log/trivial.hpp>

namespace audio
{

FxManager::~FxManager()
{
    for(int i = 0; i < FxManager::MaxSlots; i++)
    {
        if(al_slot[i])
        {
            alAuxiliaryEffectSloti(al_slot[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
            alDeleteAuxiliaryEffectSlots(1, &al_slot[i]);
        }
    }

    alDeleteFilters(1, &al_filter);
    alDeleteEffects(al_effect.size(), al_effect.data());
}

bool FxManager::loadReverb(loader::ReverbInfo effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    BOOST_ASSERT(effect_index>=loader::ReverbInfo::Outside && effect_index < loader::ReverbInfo::Sentinel);
    ALuint effect = al_effect[static_cast<int>(effect_index)];

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
    al_effect.fill(0);
    al_slot.fill(0);
    alGenAuxiliaryEffectSlots(MaxSlots, al_slot.data());
    alGenEffects(al_effect.size(), al_effect.data());
    alGenFilters(1, &al_filter);

    alFilteri(al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    loadReverb(loader::ReverbInfo::Outside, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    loadReverb(loader::ReverbInfo::SmallRoom, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    loadReverb(loader::ReverbInfo::MediumRoom, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    loadReverb(loader::ReverbInfo::LargeRoom, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    loadReverb(loader::ReverbInfo::Pipe, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    loadReverb(loader::ReverbInfo::Water, &reverb6);
}

/**
 * Updates listener parameters by camera structure. For correct speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void FxManager::updateListener(world::Camera *cam)
{
    ALfloat v[6] = {
        cam->getViewDir()[0], cam->getViewDir()[1], cam->getViewDir()[2],
        cam->getUpDir()[0], cam->getUpDir()[1], cam->getUpDir()[2]
    };

    alListenerfv(AL_ORIENTATION, v);

    alListenerfv(AL_POSITION, glm::value_ptr(cam->getPosition()));

    glm::vec3 v2 = cam->getMovement() / util::toSeconds(engine::engine_frame_time);
    alListenerfv(AL_VELOCITY, glm::value_ptr(v2));
    cam->resetMovement();

    if(!cam->getCurrentRoom())
        return;

    if(cam->getCurrentRoom()->flags & TR_ROOM_FLAG_WATER)
    {
        current_room_type = loader::ReverbInfo::Water;
    }
    else
    {
        current_room_type = cam->getCurrentRoom()->reverb_info;
    }

    if(water_state != static_cast<bool>(cam->getCurrentRoom()->flags & TR_ROOM_FLAG_WATER))
    {
        water_state = (cam->getCurrentRoom()->flags & TR_ROOM_FLAG_WATER) != 0;

        if(water_state)
        {
            engine::engine_world.audioEngine.send(TR_AUDIO_SOUND_UNDERWATER);
        }
        else
        {
            engine::engine_world.audioEngine.kill(TR_AUDIO_SOUND_UNDERWATER);
        }
    }
}

void FxManager::updateListener(world::Character* /*ent*/)
{
    ///@FIXME: Add entity listener updater here.
}

}
