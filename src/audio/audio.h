#pragma once

#include <cstdio>

#include <AL/al.h>
#include <AL/alext.h>
#include <AL/efx-presets.h>
#include <AL/efx-creative.h>

#include <sndfile.h>

#include "engine/game.h"
#include "loader/level.h"

namespace world
{
class Camera;
struct Entity;
} // namespace world

namespace audio
{

namespace
{
// AL_UNITS constant is used to translate native TR coordinates into
// OpenAL coordinates. By default, it's the same as geometry grid
// resolution (1024).

constexpr float ALUnits = 1024.0;

// MAX_CHANNELS defines maximum amount of sound sources (channels)
// that can play at the same time. Contemporary devices can play
// up to 256 channels, but we set it to 32 for compatibility
// reasons.

constexpr int MaxChannels = 32;

// NUMBUFFERS is a number of buffers cyclically used for each stream.
// Double is enough, but we use quad for further stability.

constexpr int StreamBufferCount = 4;

// NUMSOURCES tells the engine how many sources we should reserve for
// in-game music and BGMs, considering crossfades. By default, it's 6,
// as it's more than enough for typical TR audio setup (one BGM track
// plus one one-shot track or chat track in TR5).

constexpr int StreamSourceCount = 6;

// MAP_SIZE is similar to sound map size, but it is used to mark
// already played audiotracks. Note that audiotracks CAN play several
// times, if they were consequently called with increasing activation
// flags (e.g., at first we call it with 00001 flag, then with 00101,
// and so on). If all activation flags were set, including only once
// flag, audiotrack won't play anymore.

constexpr int StreamMapSize = 256;

// CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
// info is placed. Also CDAUDIO count specifies static amount of tracks existing
// in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.

constexpr int WADStride     = 268;
constexpr int WADNameLength = 260;
constexpr int WADCount      = 130;

// Sound flags are found at offset 7 of SoundDetail unit and specify
// certain sound modifications.

constexpr int AudioFlagUnknown = 0x10; // N flag. UNKNOWN MEANING!

// Sample number mask is a mask value used in bitwise operation with
// "num_samples_and_flags_1" field to extract amount of samples per
// effect.

constexpr int SampleNumberMask = 0x0F;

// Crossfades for different track types are also different,
// since background ones tend to blend in smoothly, while one-shot
// tracks should be switched fastly.

constexpr float CrossfadeOneshot = GAME_LOGIC_REFRESH_INTERVAL / 0.3f;
constexpr float CrossfadeBackground = GAME_LOGIC_REFRESH_INTERVAL / 1.0f;
constexpr float CrossfadeChat = GAME_LOGIC_REFRESH_INTERVAL / 0.1f;

// Damp coefficient specifies target volume level on a tracks
// that are being silenced (background music). The larger it is, the bigger
// silencing is.

constexpr float StreamDampLevel = 0.6f;

// Damp fade speed is used when dampable track is either being
// damped or un-damped.

constexpr float StreamDampSpeed = GAME_LOGIC_REFRESH_INTERVAL / 1.0f;

// Audio de-initialization delay gives some time to OpenAL to shut down its
// currently active sources. If timeout is reached, it means that something is
// really wrong with audio subsystem; usually five seconds is enough.

constexpr float AudioDeinitDelay = 5.0f;
} // anonymous namespace



// Possible types of errors returned by Audio_Send / Audio_Kill functions.
enum class Error
{
    NoSample,
    NoChannel,
    Ignored,
    Processed
};

// Possible errors produced by Audio_StreamPlay / Audio_StreamStop functions.
enum class StreamError
{
    PlayError,
    LoadError,
    WrongTrack,
    NoFreeStream,
    Ignored,
    Processed
};

// Audio map size is a size of effect ID array, which is used to translate
// global effect IDs to level effect IDs. If effect ID in audio map is -1
// (0xFFFF), it means that this effect is absent in current level.
// Normally, audio map size is a constant for each TR game version and
// won't change from level to level.


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

// General audio routines.

void initGlobals();

void init(uint32_t num_Sources = MaxChannels);
void deInit();

// Audio source (samples) routines.
void updateListenerByCamera(world::Camera *cam);
void updateListenerByEntity(std::shared_ptr<world::Entity> ent);

bool fillALBuffer(ALuint buf_number, SNDFILE *wavFile, Uint32 buffer_size, SF_INFO *sfInfo);
int  loadALbufferFromMem(ALuint buf_number, uint8_t *sample_pointer, size_t sample_size, size_t uncomp_sample_size = 0);
int  loadALbufferFromFile(ALuint buf_number, const char *fname);
void loadOverridedSamples(const world::World *world);

// Error handling routines.

bool logALError(int error_marker = 0);    // AL-specific error handler.
void logSndfileError(int code);           // Sndfile-specific error handler.

// Helper functions.

float   getByteDepth(SF_INFO sfInfo);
void    loadALExtFunctions(ALCdevice* device);

} // namespace audio
