#include "fxmanager.h"

#include "alext.h"
#include "audio.h"
#include "engine/system.h"

namespace audio
{

FxManager::~FxManager()
{
    for(int i = 0; i < FxManager::MaxSlots; i++)
    {
        if(FxManager::instance()->al_slot[i])
        {
            alAuxiliaryEffectSloti(FxManager::instance()->al_slot[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
            alDeleteAuxiliaryEffectSlots(1, &FxManager::instance()->al_slot[i]);
        }
    }

    alDeleteFilters(1, &FxManager::instance()->al_filter);
    alDeleteEffects(TR_AUDIO_FX_LASTINDEX, FxManager::instance()->al_effect.data());
}

bool FxManager::loadReverb(int effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    ALuint effect = FxManager::instance()->al_effect[effect_index];

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
        engine::Sys_DebugLog(LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return false;
    }
}

FxManager::FxManager(bool)
{
    al_effect.fill(0);
    al_slot.fill(0);
    alGenAuxiliaryEffectSlots(MaxSlots, al_slot.data());
    alGenEffects(TR_AUDIO_FX_LASTINDEX, al_effect.data());
    alGenFilters(1, &al_filter);

    alFilteri(al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    loadReverb(TR_AUDIO_FX_OUTSIDE, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    loadReverb(TR_AUDIO_FX_SMALLROOM, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    loadReverb(TR_AUDIO_FX_MEDIUMROOM, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    loadReverb(TR_AUDIO_FX_LARGEROOM, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    loadReverb(TR_AUDIO_FX_PIPE, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    loadReverb(TR_AUDIO_FX_WATER, &reverb6);
}

}
