#include "alext.h"

#ifndef AL_ALEXT_PROTOTYPES

#include <cassert>
#include <iostream>

// Effect objects
LPALGENEFFECTS alGenEffects = nullptr;
LPALDELETEEFFECTS alDeleteEffects = nullptr;
LPALISEFFECT alIsEffect = nullptr;
LPALEFFECTI alEffecti = nullptr;
LPALEFFECTIV alEffectiv = nullptr;
LPALEFFECTF alEffectf = nullptr;
LPALEFFECTFV alEffectfv = nullptr;
LPALGETEFFECTI alGetEffecti = nullptr;
LPALGETEFFECTIV alGetEffectiv = nullptr;
LPALGETEFFECTF alGetEffectf = nullptr;
LPALGETEFFECTFV alGetEffectfv = nullptr;

//Filter objects
LPALGENFILTERS alGenFilters = nullptr;
LPALDELETEFILTERS alDeleteFilters = nullptr;
LPALISFILTER alIsFilter = nullptr;
LPALFILTERI alFilteri = nullptr;
LPALFILTERIV alFilteriv = nullptr;
LPALFILTERF alFilterf = nullptr;
LPALFILTERFV alFilterfv = nullptr;
LPALGETFILTERI alGetFilteri = nullptr;
LPALGETFILTERIV alGetFilteriv = nullptr;
LPALGETFILTERF alGetFilterf = nullptr;
LPALGETFILTERFV alGetFilterfv = nullptr;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;

void audio::loadALExtFunctions(ALCdevice* device)
{
    static bool isLoaded = false;
    if(isLoaded)
        return;

    std::cout << "\nOpenAL device extensions: " << alcGetString(device, ALC_EXTENSIONS) << "\n";
    assert(alcIsExtensionPresent(device, ALC_EXT_EFX_NAME) == ALC_TRUE);

    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
    alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
    alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
    alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
    alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
    alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
    alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
    alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
    alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
    alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
    alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
    alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
    alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
    alGetFilteri = (LPALGETFILTERI)alGetProcAddress("alGetFilteri");
    alGetFilteriv = (LPALGETFILTERIV)alGetProcAddress("alGetFilteriv");
    alGetFilterf = (LPALGETFILTERF)alGetProcAddress("alGetFilterf");
    alGetFilterfv = (LPALGETFILTERFV)alGetProcAddress("alGetFilterfv");
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
    alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
    alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
    alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
    alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
    alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
    alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

    isLoaded = true;
}

#endif // ifndef AL_ALEXT_PROTOTYPES
