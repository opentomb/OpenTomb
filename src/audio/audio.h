#pragma once

namespace audio
{

// Define some common samples across ALL TR versions.

#define TR_AUDIO_SOUND_NO          2
#define TR_AUDIO_SOUND_SLIDING     3
#define TR_AUDIO_SOUND_LANDING     4
#define TR_AUDIO_SOUND_HOLSTEROUT  6
#define TR_AUDIO_SOUND_HOLSTERIN   7
#define TR_AUDIO_SOUND_SHOTPISTOLS 8
#define TR_AUDIO_SOUND_RELOAD      9
#define TR_AUDIO_SOUND_RICOCHET    10
#define TR_AUDIO_SOUND_LARASCREAM  30
#define TR_AUDIO_SOUND_LARAINJURY  31
#define TR_AUDIO_SOUND_SPLASH      33
#define TR_AUDIO_SOUND_FROMWATER   34
#define TR_AUDIO_SOUND_SWIM        35
#define TR_AUDIO_SOUND_LARABREATH  36
#define TR_AUDIO_SOUND_BUBBLE      37
#define TR_AUDIO_SOUND_USEKEY      39
#define TR_AUDIO_SOUND_SHOTUZI     43
#define TR_AUDIO_SOUND_SHOTSHOTGUN 45
#define TR_AUDIO_SOUND_UNDERWATER  60
#define TR_AUDIO_SOUND_PUSHABLE    63
#define TR_AUDIO_SOUND_MENUROTATE  108
#define TR_AUDIO_SOUND_MENUSELECT  109
#define TR_AUDIO_SOUND_MENUOPEN    111
#define TR_AUDIO_SOUND_MENUCLOSE   112  // Only used in TR1-3.
#define TR_AUDIO_SOUND_MENUCLANG   114
#define TR_AUDIO_SOUND_MENUPAGE    115
#define TR_AUDIO_SOUND_MEDIPACK    116

// Error handling routines.

bool checkALError(const char *error_marker = "");    // AL-specific error handler.
void logSndfileError(int code);           // Sndfile-specific error handler.

} // namespace audio
