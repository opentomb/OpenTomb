
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// AL_UNITS constant is used to translate native TR coordinates into
// OpenAL coordinates. By default, it's the same as geometry grid
// resolution (1024).

#define TR_AUDIO_AL_UNITS 1024.0

// MAX_CHANNELS defines maximum amount of sound sources (channels)
// that can play at the same time. Contemporary devices can play
// up to 256 channels, but we set it to 32 for compatibility
// reasons.

#define TR_AUDIO_MAX_CHANNELS 32

// NUMSOURCES tells the engine how many sources we should reserve for
// in-game music and BGMs, considering crossfades. By default, it's 6,
// as it's more than enough for typical TR audio setup (one BGM track
// plus one one-shot track or chat track in TR5).

#define TR_AUDIO_STREAM_NUMSOURCES 6


// Sound flags are found at offset 7 of SoundDetail unit and specify
// certain sound modifications.

#define TR_AUDIO_FLAG_RAND_PITCH  0x20 // P flag. Slight random pitch shift.
#define TR_AUDIO_FLAG_RAND_VOLUME 0x40 // V flag. Slight random gain shift.
#define TR_AUDIO_FLAG_UNKNOWN     0x10 // N flag. UNKNOWN MEANING!

// Looped field is located at offset 6 of SoundDetail structure and
// combined with SampleIndexes value. This field is responsible for
// looping behaviour of each sound.
// L flag sets sound to continous looped state, while W flag waits
// for any sound with similar ID to finish, and only then plays it
// again. R flag rewinds sound, if sound with similar ID is being
// sent to sources.

#define TR_AUDIO_LOOP_LOOPED  0x03  // L flag. Normal looped.
#define TR_AUDIO_LOOP_REWIND  0x02  // R flag. Rewind on re-send.
#define TR_AUDIO_LOOP_WAIT    0x01  // W flag. Wait until play is over.
#define TR_AUDIO_LOOP_NONE    0x00  // No looping.

// Sample number mask is a mask value used in bitwise operation with
// "num_samples_and_flags_1" field to extract amount of samples per
// effect.

#define TR_AUDIO_SAMPLE_NUMBER_MASK 0x0F

// Entity types are used to identify different sound emitter types. Since
// sounds in TR games could be emitted either by entities, sound sources
// or global events, we have defined these three types of emitters.

#define TR_AUDIO_EMITTER_ENTITY      0   // Entity (movable)
#define TR_AUDIO_EMITTER_SOUNDSOURCE 1   // Sound source (static)
#define TR_AUDIO_EMITTER_GLOBAL      2   // Global sound (menus, secret, etc.)

// Possible types of errors returned by Audio_Send / Audio_Kill functions.

#define TR_AUDIO_SEND_NOSAMPLE  (-2)
#define TR_AUDIO_SEND_NOCHANNEL (-1)
#define TR_AUDIO_SEND_IGNORED     0
#define TR_AUDIO_SEND_PROCESSED   1

// Define some common samples across ALL TR versions.

#define TR_AUDIO_SOUND_NO          2
#define TR_AUDIO_SOUND_SLIDING     3
#define TR_AUDIO_SOUND_LANDING     4
#define TR_AUDIO_SOUND_HOLSTEROUT  6
#define TR_AUDIO_SOUND_HOLSTERIN   7
#define TR_AUDIO_SOUND_SHOTPISTOLS 8
#define TR_AUDIO_SOUND_RELOAD      9
#define TR_AUDIO_SOUND_RICOCHET    10

#define TR_AUDIO_SOUND_SHOTHARPOON_G   15
#define TR_AUDIO_SOUND_RELOADHARPOON_G 16
#define TR_AUDIO_SOUND_SHOTHARPOON_W   23
#define TR_AUDIO_SOUND_RELOADHARPOON_W 22
#define TR_AUDIO_SOUND_SHOTAUTOMAGS    21

#define TR_AUDIO_SOUND_LARASCREAM  30
#define TR_AUDIO_SOUND_LARAINJURY  31
#define TR_AUDIO_SOUND_SPLASH      33
#define TR_AUDIO_SOUND_FROMWATER   34
#define TR_AUDIO_SOUND_SWIM        35
#define TR_AUDIO_SOUND_LARABREATH  36
#define TR_AUDIO_SOUND_BUBBLE      37
#define TR_AUDIO_SOUND_USEKEY      39
#define TR_AUDIO_SOUND_SHOTUZI     43
#define TR_AUDIO_SOUND_SHOTUZI_END 44   // After TR1
#define TR_AUDIO_SOUND_SHOTMAGNUM  44   // Only on TR1 !
#define TR_AUDIO_SOUND_SHOTSHOTGUN 45
#define TR_AUDIO_SOUND_UNDERWATER  60
#define TR_AUDIO_SOUND_PUSHABLE    63
#define TR_AUDIO_SOUND_SHOTM16     78   // only TR2
#define TR_AUDIO_SOUND_SHOTMP5     78   // only TR3
#define TR_AUDIO_SOUND_SHOTM16_END 103  // only TR2
#define TR_AUDIO_SOUND_RELOADROCKETGUN  104
#define TR_AUDIO_SOUND_SHOTROCKETGUN    105
#define TR_AUDIO_SOUND_MENUROTATE  108
#define TR_AUDIO_SOUND_MENUSELECT  109
#define TR_AUDIO_SOUND_MENUOPEN    111
#define TR_AUDIO_SOUND_MENUCLOSE   112  // Only used in TR1-3.
#define TR_AUDIO_SOUND_MENUCLANG   114
#define TR_AUDIO_SOUND_MENUPAGE    115
#define TR_AUDIO_SOUND_MEDIPACK    116
#define TR_AUDIO_SOUND_SHOTDESERTEAGLE  121 // TR3 only
#define TR_AUDIO_SOUND_SHOTREVOLVER     121 // TR4 and TR5
#define TR_AUDIO_SOUND_SHOTGRENADEGUN   125
#define TR_AUDIO_SOUND_RELOADGRENADEGUN 126
#define TR_AUDIO_SOUND_SHOTCROSSBOW 235

// Certain sound effect indexes were changed across different TR
// versions, despite remaining the same - mostly, it happened with
// menu sounds and some general sounds. For such effects, we specify
// additional remap enumeration list, which is fed into Lua script
// to get actual effect ID for current game version.

enum TR_AUDIO_SOUND_GLOBALID
{
    TR_AUDIO_SOUND_GLOBALID_MENUOPEN,
    TR_AUDIO_SOUND_GLOBALID_MENUCLOSE,
    TR_AUDIO_SOUND_GLOBALID_MENUROTATE,
    TR_AUDIO_SOUND_GLOBALID_MENUPAGE,
    TR_AUDIO_SOUND_GLOBALID_MENUSELECT,
    TR_AUDIO_SOUND_GLOBALID_MENUWEAPON,
    TR_AUDIO_SOUND_GLOBALID_MENUCLANG,
    TR_AUDIO_SOUND_GLOBALID_MENUAUDIOTEST,
    TR_AUDIO_SOUND_GLOBALID_LASTINDEX
};


// Possible errors produced by Audio_StreamPlay / Audio_StreamStop functions.
#define TR_AUDIO_STREAMPLAY_PLAYERROR    (-4)
#define TR_AUDIO_STREAMPLAY_LOADERROR    (-3)
#define TR_AUDIO_STREAMPLAY_WRONGTRACK   (-2)
#define TR_AUDIO_STREAMPLAY_NOFREESTREAM (-1)
#define TR_AUDIO_STREAMPLAY_IGNORED        0
#define TR_AUDIO_STREAMPLAY_PROCESSED      1

// Audio stream type defines stream behaviour. While background track
// loops forever until interrupted by other background track, one-shot
// and chat tracks doesn't interrupt them, playing in parallel instead.
// However, all stream types could be interrupted by next pending track
// with same type.

enum TR_AUDIO_STREAM_TYPE
{
    TR_AUDIO_STREAM_TYPE_BACKGROUND,    // BGM tracks.
    TR_AUDIO_STREAM_TYPE_ONESHOT,       // One-shot music pieces.
    TR_AUDIO_STREAM_TYPE_CHAT,          // Chat tracks.
    TR_AUDIO_STREAM_TYPE_LASTINDEX
};

// Audio settings structure.

typedef struct audio_settings_s
{
    float       music_volume;
    float       sound_volume;
    uint32_t    use_effects : 1;
    uint32_t    listener_is_player : 1; // RESERVED FOR FUTURE USE
}audio_settings_t, *audio_settings_p;


extern struct audio_settings_s audio_settings;

// General audio routines.

void Audio_InitGlobals();

void Audio_CoreInit();
void Audio_CoreDeinit();
void Audio_Init(uint32_t num_Sources = TR_AUDIO_MAX_CHANNELS);
void Audio_GenSamples(class VT_Level *tr);
void Audio_CacheTrack(int id);
int  Audio_DeInit();
void Audio_Update(float time);

// Audio source (samples) routines.
int  Audio_IsEffectPlaying(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);

int  Audio_Send(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);    // Send to play effect with given parameters.
int  Audio_Kill(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);    // If exist, immediately stop and destroy all effects with given parameters.

// Stream tracks (music / BGM) routines.
int  Audio_EndStreams(int stream_type = -1);        // End ALL streams (with crossfade).
int  Audio_StopStreams(int stream_type = -1);       // Immediately stop ALL streams.
int  Audio_PauseStreams(int stream_type = -1);      // Pause ALL streams (of specified type).
int  Audio_ResumeStreams(int stream_type = -1);     // Resume ALL streams.

// Generally, you need only this function to trigger any track.
int Audio_StreamPlay(const uint32_t track_index, const uint8_t mask = 0);


struct stream_track_s *Audio_GetStreamExternal();

#endif // AUDIO_H
