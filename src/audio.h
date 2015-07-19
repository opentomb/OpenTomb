
#ifndef AUDIO_H
#define AUDIO_H

#include <cstdio>
#include <cstdlib>

#include <AL/al.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#include <AL/efx-creative.h>

#include <sndfile.h>

#include "vt/vt_level.h"
#include "game.h"
#include "script.h"
#include "system.h"

// AL_UNITS constant is used to translate native TR coordinates into
// OpenAL coordinates. By default, it's the same as geometry grid
// resolution (1024).

#define TR_AUDIO_AL_UNITS 1024.0

// MAX_CHANNELS defines maximum amount of sound sources (channels)
// that can play at the same time. Contemporary devices can play
// up to 256 channels, but we set it to 32 for compatibility
// reasons.

#define TR_AUDIO_MAX_CHANNELS 32

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

// Audio map size is a size of effect ID array, which is used to translate
// global effect IDs to level effect IDs. If effect ID in audio map is -1
// (0xFFFF), it means that this effect is absent in current level.
// Normally, audio map size is a constant for each TR game version and
// won't change from level to level.

#define TR_AUDIO_MAP_SIZE_NONE (-1)
#define TR_AUDIO_MAP_SIZE_TR1  256
#define TR_AUDIO_MAP_SIZE_TR2  370
#define TR_AUDIO_MAP_SIZE_TR3  370
#define TR_AUDIO_MAP_SIZE_TR4  370
#define TR_AUDIO_MAP_SIZE_TR5  450

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

// Default range and pitch values are required for compatibility with
// TR1 and TR2 levels, as there is no such parameters in SoundDetails
// structures.

#define TR_AUDIO_DEFAULT_RANGE 8
#define TR_AUDIO_DEFAULT_PITCH 1.0       // 0.0 - only noise

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


// NUMBUFFERS is a number of buffers cyclically used for each stream.
// Double is enough, but we use quad for further stability.

#define TR_AUDIO_STREAM_NUMBUFFERS 4

// NUMSOURCES tells the engine how many sources we should reserve for
// in-game music and BGMs, considering crossfades. By default, it's 6,
// as it's more than enough for typical TR audio setup (one BGM track
// plus one one-shot track or chat track in TR5).

#define TR_AUDIO_STREAM_NUMSOURCES 6

// MAP_SIZE is similar to sound map size, but it is used to mark
// already played audiotracks. Note that audiotracks CAN play several
// times, if they were consequently called with increasing activation
// flags (e.g., at first we call it with 00001 flag, then with 00101,
// and so on). If all activation flags were set, including only once
// flag, audiotrack won't play anymore.

#define TR_AUDIO_STREAM_MAP_SIZE 256

// CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
// info is placed. Also CDAUDIO count specifies static amount of tracks existing
// in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.

#define TR_AUDIO_STREAM_WAD_STRIDE     268
#define TR_AUDIO_STREAM_WAD_NAMELENGTH 260
#define TR_AUDIO_STREAM_WAD_COUNT      130

// Stream loading method describes the way audiotracks are loaded.
// There are either seperate track files or single CDAUDIO.WAD file.

enum TR_AUDIO_STREAM_METHOD
{
    TR_AUDIO_STREAM_METHOD_TRACK,  // Separate tracks. Used in TR 1, 2, 4, 5.
    TR_AUDIO_STREAM_METHOD_WAD,    // WAD file.  Used in TR3.
    TR_AUDIO_STREAM_METHOD_LASTINDEX

};

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

// Crossfades for different track types are also different,
// since background ones tend to blend in smoothly, while one-shot
// tracks should be switched fastly.

#define TR_AUDIO_STREAM_CROSSFADE_ONESHOT (GAME_LOGIC_REFRESH_INTERVAL    / 0.3f)
#define TR_AUDIO_STREAM_CROSSFADE_BACKGROUND (GAME_LOGIC_REFRESH_INTERVAL / 1.0f)
#define TR_AUDIO_STREAM_CROSSFADE_CHAT (GAME_LOGIC_REFRESH_INTERVAL       / 0.1f)

// Damp coefficient specifies target volume level on a tracks
// that are being silenced (background music). The larger it is, the bigger
// silencing is.

#define TR_AUDIO_STREAM_DAMP_LEVEL 0.6f

// Damp fade speed is used when dampable track is either being
// damped or un-damped.

#define TR_AUDIO_STREAM_DAMP_SPEED (GAME_LOGIC_REFRESH_INTERVAL / 1.0f)

// Possible errors produced by Audio_StreamPlay / Audio_StreamStop functions.

#define TR_AUDIO_STREAMPLAY_PLAYERROR    (-4)
#define TR_AUDIO_STREAMPLAY_LOADERROR    (-3)
#define TR_AUDIO_STREAMPLAY_WRONGTRACK   (-2)
#define TR_AUDIO_STREAMPLAY_NOFREESTREAM (-1)
#define TR_AUDIO_STREAMPLAY_IGNORED        0
#define TR_AUDIO_STREAMPLAY_PROCESSED      1

// Audio de-initialization delay gives some time to OpenAL to shut down its
// currently active sources. If timeout is reached, it means that something is
// really wrong with audio subsystem; usually five seconds is enough.

#define TR_AUDIO_DEINIT_DELAY 5.0


struct Camera;
struct Entity;

// Audio settings structure.

struct AudioSettings
{
    ALfloat     music_volume = 0;
    ALfloat     sound_volume = 0;
    bool        use_effects = false;
    bool        effects_initialized = false;
    bool        listener_is_player = false; // RESERVED FOR FUTURE USE
    int         stream_buffer_size = 0;
};

// FX manager structure.
// It contains all necessary info to process sample FX (reverb and echo).

struct AudioFxManager
{
    ALuint      al_filter;
    ALuint      al_effect[TR_AUDIO_FX_LASTINDEX];
    ALuint      al_slot[TR_AUDIO_MAX_SLOTS];
    ALuint      current_slot;
    ALuint      current_room_type;
    ALuint      last_room_type;
    int8_t      water_state;    // If listener is underwater, all samples will damp.
};

// Effect structure.
// Contains all global effect parameters.

struct AudioEffect
{
    // General sound source parameters (similar to TR sound info).

    ALfloat     pitch = 0;          // [PIT in TR] Global pitch shift.
    ALfloat     gain = 0;           // [VOL in TR] Global gain (volume).
    ALfloat     range = 0;          // [RAD in TR] Range (radius).
    ALuint      chance = 0;         // [CH  in TR] Chance to play.
    ALuint      loop = 0;           // 0 = none, 1 = W, 2 = R, 3 = L.
    ALboolean   rand_pitch = false;     // Similar to flag 0x200000 (P) in native TRs.
    ALboolean   rand_gain = false;      // Similar to flag 0x400000 (V) in native TRs.

    // Additional sound source parameters.
    // These are not natively in TR engines, but can be later assigned by
    // external script.

    ALboolean   rand_freq = false;          // Slightly randomize frequency.
    ALuint      rand_pitch_var = 0;     // Pitch randomizer bounds.
    ALuint      rand_gain_var = 0;      // Gain  randomizer bounds.
    ALuint      rand_freq_var = 0;      // Frequency randomizer bounds.

    // Sample reference parameters.

    ALuint      sample_index = 0;       // First (or only) sample (buffer) index.
    ALuint      sample_count = 0;       // Sample amount to randomly select from.
};

// Audio emitter (aka SoundSource) structure.

struct AudioEmitter
{
    ALuint      emitter_index;  // Unique emitter index.
    ALuint      sound_index;    // Sound index.
    btVector3   position;    // Vector coordinate.
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
};


// Main audio source class.

// Sound source is a complex class, each member of which is linked with
// certain in-game entity or sound source, but also a kind of entity by itself.
// Number of simultaneously existing sound sources is fixed, and can't be more than
// MAX_CHANNELS global constant.

class AudioSource
{
public:
    AudioSource();  // Audio source constructor.
   ~AudioSource();  // Audio source destructor.

    void Play();    // Make source active and play it.
    void Pause();   // Pause source (leaving it active).
    void Stop();    // Stop and destroy source.
    void Update();  // Update source parameters.

    void SetBuffer(ALint buffer);           // Assign buffer to source.
    void SetLooping(ALboolean is_looping);  // Set looping flag.
    void SetPitch(ALfloat pitch_value);     // Set pitch shift.
    void SetGain(ALfloat gain_value);       // Set gain (volume).
    void SetRange(ALfloat range_value);     // Set max. audible distance.
    void SetFX();                           // Set reverb FX, according to room flag.
    void UnsetFX();                         // Remove any reverb FX from source.
    void SetUnderwater();                   // Apply low-pass underwater filter.

    bool IsPlaying();           // Check if source is currently playing.
    bool IsActive();            // Check if source is active.

    int32_t     emitter_ID;     // Entity of origin. -1 means no entity (hence - empty source).
    uint32_t    emitter_type;   // 0 - ordinary entity, 1 - sound source, 2 - global sound.
    uint32_t    effect_index;   // Effect index. Used to associate effect with entity for R/W flags.
    uint32_t    sample_index;   // OpenAL sample (buffer) index. May be the same for different sources.
    uint32_t    sample_count;   // How many buffers to use, beginning with sample_index.

    friend int Audio_IsEffectPlaying(int effect_ID, int entity_type, int entity_ID);

private:
    bool        active;         // Source gets autostopped and destroyed on next frame, if it's not set.
    bool        is_water;       // Marker to define if sample is in underwater state or not.
    ALuint      source_index;   // Source index. Should be unique for each source.

    void LinkEmitter();                             // Link source to parent emitter.
    void SetPosition(const ALfloat pos_vector[]);   // Set source position.
    void SetVelocity(const ALfloat vel_vector[]);   // Set source velocity (speed).
};


// Main stream track class is used to create multi-channel soundtrack player,
// which differs from classic TR scheme, where each new soundtrack interrupted
// previous one. With flexible class handling, we now can implement multitrack
// player with automatic channel and crossfade management.

class StreamTrack
{
public:
    StreamTrack();      // Stream track constructor.
   ~StreamTrack();      // Stream track destructor.

    // Load routine prepares track for playing. Arguments are track index,
    // stream type (background, one-shot or chat) and load method, which
    // differs for TR1-2, TR3 and TR4-5.

    bool Load(const char *path, const int index, const int type, const int load_method);
    bool Unload();

    bool Play(bool fade_in = false);     // Begins to play track.
    void Pause();                        // Pauses track, preserving position.
    void End();                          // End track with fade-out.
    void Stop();                         // Immediately stop track.
    bool Update();                       // Update track and manage streaming.

    bool IsTrack(const int track_index); // Checks desired track's index.
    bool IsType(const int track_type);   // Checks desired track's type.
    bool IsPlaying();                    // Checks if track is playing.
    bool IsActive();                     // Checks if track is still active.
    bool IsDampable();                   // Checks if track is dampable.

    void SetFX();                        // Set reverb FX, according to room flag.
    void UnsetFX();                      // Remove any reverb FX from source.

    static bool damp_active;             // Global flag for damping BGM tracks.

private:
    bool Load_Track(const char *path);                     // Track loading.
    bool Load_Wad(uint8_t index, const char *filename);    // Wad loading.

    bool Stream(ALuint buffer);          // General stream routine.

    FILE*           wad_file;   // General handle for opened wad file.
    SNDFILE*        snd_file;   // Sndfile file reader needs its own handle.
    SF_INFO         sf_info;

    // General OpenAL fields

    ALuint          source;
    ALuint          buffers[TR_AUDIO_STREAM_NUMBUFFERS];
    ALenum          format;
    ALsizei         rate;
    ALfloat         current_volume;     // Stream volume, considering fades.
    ALfloat         damped_volume;      // Additional damp volume multiplier.

    bool            active;             // If track is active or not.
    bool            ending;             // Used when track is being faded by other one.
    bool            dampable;           // Specifies if track can be damped by others.
    int             stream_type;        // Either BACKGROUND, ONESHOT or CHAT.
    int             current_track;      // Needed to prevent same track sending.
    int             method;             // TRACK (TR1-2/4-5) or WAD (TR3).
};

// General audio routines.

void Audio_InitGlobals();
void Audio_InitFX();

void Audio_Init(uint32_t num_Sources = TR_AUDIO_MAX_CHANNELS);
int  Audio_DeInit();
void Audio_Update();

// Audio source (samples) routines.

int  Audio_GetFreeSource();
bool Audio_IsInRange(int entity_type, int entity_ID, float range, float gain);
int  Audio_IsEffectPlaying(int effect_ID = -1, int entity_type = -1, int entity_ID = -1);

int  Audio_Send(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);    // Send to play effect with given parameters.
int  Audio_Kill(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);    // If exist, immediately stop and destroy all effects with given parameters.

void Audio_PauseAllSources();    // Used to pause all effects currently playing.
void Audio_StopAllSources();     // Used in audio deinit.
void Audio_ResumeAllSources();   // Used to resume all effects currently paused.
void Audio_UpdateSources();      // Main sound loop.
void Audio_UpdateListenerByCamera(Camera *cam);
void Audio_UpdateListenerByEntity(std::shared_ptr<Entity> ent);

bool Audio_FillALBuffer(ALuint buf_number, SNDFILE *wavFile, Uint32 buffer_size, SF_INFO *sfInfo);
int  Audio_LoadALbufferFromMem(ALuint buf_number, uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size = 0);
int  Audio_LoadALbufferFromFile(ALuint buf_number, const char *fname);
void Audio_LoadOverridedSamples(World *world);

int  Audio_LoadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb);

// Stream tracks (music / BGM) routines.

int  Audio_GetFreeStream();                          // Get free (stopped) stream.
bool Audio_IsTrackPlaying(int32_t track_index = -1); // See if track is already playing.
bool Audio_TrackAlreadyPlayed(uint32_t track_index,
                              int8_t mask = 0);      // Check if track played with given activation mask.
void Audio_UpdateStreams();                          // Update all streams.
void Audio_UpdateStreamsDamping();                   // See if there any damping tracks playing.
void Audio_PauseStreams(int stream_type = -1);       // Pause ALL streams (of specified type).
void Audio_ResumeStreams(int stream_type = -1);      // Resume ALL streams.
bool Audio_EndStreams(int stream_type = -1);         // End ALL streams (with crossfade).
bool Audio_StopStreams(int stream_type = -1);        // Immediately stop ALL streams.

// Generally, you need only this function to trigger any track.
int Audio_StreamPlay(const uint32_t track_index, const uint8_t mask = 0);

// Error handling routines.

bool Audio_LogALError(int error_marker = 0);    // AL-specific error handler.
void Audio_LogSndfileError(int code);           // Sndfile-specific error handler.

// Helper functions.

float   Audio_GetByteDepth(SF_INFO sfInfo);
void    Audio_LoadALExtFunctions(ALCdevice* device);
bool    Audio_DeInitDelay();

#endif // AUDIO_H
