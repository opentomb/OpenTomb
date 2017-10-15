
#include "../config-opentomb.h"

#include <string.h>

#include "audio_al.h"
#include "audio.h"
#include "audio_fx.h"

// FX manager structure.
// It contains all necessary info to process sample FX (reverb and echo).
typedef struct audio_fxmanager_s
{
    ALuint      al_filter;
    ALuint      al_effect[TR_AUDIO_FX_LASTINDEX];
    ALuint      al_slot[TR_AUDIO_MAX_SLOTS];
    ALuint      current_slot;
    ALuint      current_room_type;
    ALuint      last_room_type;
    bool        water_state;    // If listener is underwater, all samples will damp.
}audio_fxmanager_t, *audio_fxmanager_p;



struct audio_fxmanager_s    fxManager = {0};

#ifdef HAVE_ALEXT_H
int Audio_LoadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb);
#endif


void Audio_InitFX()
{
#ifdef HAVE_ALEXT_H
    memset(&fxManager, 0, sizeof(audio_fxmanager_s));

    // Set up effect slots, effects and filters.

    alGenAuxiliaryEffectSlots(TR_AUDIO_MAX_SLOTS, fxManager.al_slot);
    alGenEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    alGenFilters(1, &fxManager.al_filter);

    alFilteri(fxManager.al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    Audio_LoadReverbToFX(TR_AUDIO_FX_OUTSIDE, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    Audio_LoadReverbToFX(TR_AUDIO_FX_SMALLROOM, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    Audio_LoadReverbToFX(TR_AUDIO_FX_MEDIUMROOM, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    Audio_LoadReverbToFX(TR_AUDIO_FX_LARGEROOM, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    Audio_LoadReverbToFX(TR_AUDIO_FX_PIPE, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    Audio_LoadReverbToFX(TR_AUDIO_FX_WATER, &reverb6);
#endif
    fxManager.last_room_type = TR_AUDIO_FX_LASTINDEX;
}


void Audio_DeinitFX()
{
#ifdef HAVE_ALEXT_H
    for(int i = 0; i < TR_AUDIO_MAX_SLOTS; i++)
    {
        if(alIsAuxiliaryEffectSlot(fxManager.al_slot[i]))
        {
            alAuxiliaryEffectSloti(fxManager.al_slot[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
            alDeleteAuxiliaryEffectSlots(1, &fxManager.al_slot[i]);
        }
    }

    if(alIsFilter(fxManager.al_filter))
    {
        alDeleteFilters(1, &fxManager.al_filter);
        alDeleteEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    }
#endif
}

#ifdef HAVE_ALEXT_H
int Audio_LoadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    ALuint effect = fxManager.al_effect[effect_index];

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
    }
    else
    {
        //Sys_DebugLog(SYS_LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return 0;
    }

    return 1;
}
#endif


void Audio_SetFX(ALuint source)
{
#ifdef HAVE_ALEXT_H
    ALuint effect;
    ALuint slot;

    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    if(fxManager.current_room_type != fxManager.last_room_type)  // Switch audio send.
    {
        fxManager.last_room_type = fxManager.current_room_type;
        fxManager.current_slot   = (++fxManager.current_slot > (TR_AUDIO_MAX_SLOTS - 1)) ? (0) : (fxManager.current_slot);

        effect = fxManager.al_effect[fxManager.current_room_type];
        slot   = fxManager.al_slot[fxManager.current_slot];

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
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
#endif
}


void Audio_UnsetFX(ALuint source)
{
#ifdef HAVE_ALEXT_H
    // Remove any audio sends and direct filters from channel.
    alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
#endif
}


void Audio_SetFXWaterStateForSource(ALuint source)
{
#ifdef HAVE_ALEXT_H
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.
    if(fxManager.water_state)
    {
        alSourcei(source, AL_DIRECT_FILTER, fxManager.al_filter);
    }
    else
    {
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    }
#endif
}


void Audio_SetFXRoomType(int value)
{
    fxManager.current_room_type = value;
}


void Audio_SetFXWaterState(bool state)
{
    fxManager.water_state = state;
}


bool Audio_GetFXWaterState()
{
    return fxManager.water_state;
}
