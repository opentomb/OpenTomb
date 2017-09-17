
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <stdint.h>

// NUMBUFFERS is a number of buffers cyclically used for each stream.
// Double is enough, but we use quad for further stability, because
// OGG codec seems to be very sensitive to buffering.

#define TR_AUDIO_STREAM_NUMBUFFERS 4

// MAP_SIZE is similar to sound map size, but it is used to mark
// already played audiotracks. Note that audiotracks CAN play several
// times, if they were consequently called with increasing activation
// flags (e.g., at first we call it with 00001 flag, then with 00101,
// and so on). If all activation flags were set, including only once
// flag, audiotrack won't play anymore.

#define TR_AUDIO_STREAM_MAP_SIZE 256

// Stream loading method describes the way audiotracks are loaded.
// There are either seperate OGG files, single CDAUDIO.WAD file or
// seperate ADPCM WAV files.
// You can add extra types with implementation of extra audio codecs,
// only thing to do is to add corresponding stream and load routines
// into class' private section.

enum TR_AUDIO_STREAM_METHOD
{
    TR_AUDIO_STREAM_METHOD_OGG,    // OGG files. Used in TR1-2 (replaces CD audio).
    TR_AUDIO_STREAM_METHOD_WAD,    // WAD file.  Used in TR3.
    TR_AUDIO_STREAM_METHOD_WAV,    // WAV files. Used in TR4-5.
    TR_AUDIO_STREAM_METHOD_LASTINDEX
};

// Crossfades for different track types are also different,
// since background ones tend to blend in smoothly, while one-shot
// tracks should be switched fastly.

#define TR_AUDIO_STREAM_CROSSFADE_ONESHOT       (60.0f / 0.3f)
#define TR_AUDIO_STREAM_CROSSFADE_BACKGROUND    (60.0f / 2.0f)
#define TR_AUDIO_STREAM_CROSSFADE_CHAT          (60.0f / 0.1f)

// Damp coefficient specifies target volume level on a tracks
// that are being silenced (background music). The larger it is, the bigger
// silencing is.

#define TR_AUDIO_STREAM_DAMP_LEVEL 0.6f

// Damp fade speed is used when dampable track is either being
// damped or un-damped.

#define TR_AUDIO_STREAM_DAMP_SPEED (GAME_LOGIC_REFRESH_INTERVAL / 1.0f)


// Main stream track struct is used to create multi-channel soundtrack player,
// which differs from classic TR scheme, where each new soundtrack interrupted
// previous one. With flexible class handling, we now can implement multitrack
// player with automatic channel and crossfade management.

#define TR_AUDIO_STREAM_STOPPED     (0)
#define TR_AUDIO_STREAM_PLAYING     (1)
#define TR_AUDIO_STREAM_PAUSED      (2)
#define TR_AUDIO_STREAM_STOPPING    (3)

typedef struct stream_track_s
{
    int32_t         track;
    uint16_t        type; // Either BACKGROUND, ONESHOT or CHAT.
    uint16_t        state;
    uint32_t        linked_buffers;
    uint32_t        buffer_offset;
    ALuint          source;
    ALuint          buffers[TR_AUDIO_STREAM_NUMBUFFERS];
    ALfloat         current_volume;     // Stream volume, considering fades.
}stream_track_t, *stream_track_p;


void StreamTrack_Init(stream_track_p s);
void StreamTrack_Clear(stream_track_p s);
int StreamTrack_Play(stream_track_p s);
int StreamTrack_Stop(stream_track_p s);
int StreamTrack_Pause(stream_track_p s);
int StreamTrack_CheckForEnd(stream_track_p s);

int StreamTrack_IsNeedUpdateBuffer(stream_track_p s);
int StreamTrack_UpdateBuffer(stream_track_p s, uint8_t *buff, size_t size, int sample_bitsize, int channels, int frequency);


#endif // AUDIO_STREAM_H
