
#ifndef AUDIO_FX_H
#define AUDIO_FX_H

#include <stdint.h>


// MAX_SLOTS specifies amount of FX slots used to apply environmental
// effects to sounds. We need at least two of them to prevent glitches
// at environment transition (slots are cyclically changed, leaving
// previously played samples at old slot). Maximum amount is 4, but
// it's not recommended to set it more than 2.

#define TR_AUDIO_MAX_SLOTS 2

// In TR3-5, there were 5 reverb / echo effect flags for each
// room, but they were never used in PC versions - however, level
// files still contain this info, so we now can re-use these flags
// to assign reverb/echo presets to each room.
// Also, underwater environment can be considered as additional
// reverb flag, so overall amount is 6.

enum TR_AUDIO_FX {

    TR_AUDIO_FX_OUTSIDE,         // EFX_REVERB_PRESET_CITY
    TR_AUDIO_FX_SMALLROOM,       // EFX_REVERB_PRESET_LIVINGROOM
    TR_AUDIO_FX_MEDIUMROOM,      // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
    TR_AUDIO_FX_LARGEROOM,       // EFX_REVERB_PRESET_DOME_TOMB
    TR_AUDIO_FX_PIPE,            // EFX_REVERB_PRESET_PIPE_LARGE
    TR_AUDIO_FX_WATER,           // EFX_REVERB_PRESET_UNDERWATER
    TR_AUDIO_FX_LASTINDEX
};

// Sound flags are found at offset 7 of SoundDetail unit and specify
// certain sound modifications.

/*#define TR_AUDIO_FLAG_RAND_PITCH  0x20 // P flag. Slight random pitch shift.
#define TR_AUDIO_FLAG_RAND_VOLUME 0x40 // V flag. Slight random gain shift.
#define TR_AUDIO_FLAG_UNKNOWN     0x10 // N flag. UNKNOWN MEANING!
*/

void Audio_InitFX();
void Audio_DeinitFX();

void Audio_SetFX(uint32_t source);
void Audio_UnsetFX(uint32_t source);


void Audio_SetFXWaterStateForSource(uint32_t source);
void Audio_SetFXRoomType(int value);
void Audio_SetFXWaterState(bool state);
bool Audio_GetFXWaterState();

#endif // AUDIO_FX_H
