
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <math.h>

#include "config-opentomb.h"

extern "C" {
#include <al.h>
#include <alc.h>
#ifdef HAVE_ALEXT_H
#include <alext.h>
#endif
#ifdef HAVE_EFX_H
#include <efx.h>
#endif
#ifdef HAVE_EFX_PRESETS_H
#include <efx-presets.h>
#endif

#include <codec.h>
#include <ogg.h>
#include <os_types.h>
#include <vorbisfile.h>
}

#include "core/system.h"
#include "core/gl_text.h"
#include "core/console.h"
#include "core/vmath.h"
#include "render/camera.h"
#include "render/render.h"
#include "vt/vt_level.h"
#include "game.h"
#include "audio.h"
#include "script.h"
#include "engine.h"
#include "entity.h"
#include "character_controller.h"
#include "engine_string.h"
#include "room.h"
#include "world.h"
#include "trigger.h"


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
    int8_t      water_state;    // If listener is underwater, all samples will damp.
}audio_fxmanager_t, *audio_fxmanager_p;

// Effect structure.
// Contains all global effect parameters.

typedef struct audio_effect_s
{
    // General sound source parameters (similar to TR sound info).

    ALfloat     pitch;          // [PIT in TR] Global pitch shift.
    ALfloat     gain;           // [VOL in TR] Global gain (volume).
    ALfloat     range;          // [RAD in TR] Range (radius).
    ALuint      chance;         // [CH  in TR] Chance to play.
    ALuint      loop;           // 0 = none, 1 = W, 2 = R, 3 = L.
    ALboolean   rand_pitch;     // Similar to flag 0x200000 (P) in native TRs.
    ALboolean   rand_gain;      // Similar to flag 0x400000 (V) in native TRs.

    // Additional sound source parameters.
    // These are not natively in TR engines, but can be later assigned by
    // external script.

    ALboolean   rand_freq;          // Slightly randomize frequency.
    ALuint      rand_pitch_var;     // Pitch randomizer bounds.
    ALuint      rand_gain_var;      // Gain  randomizer bounds.
    ALuint      rand_freq_var;      // Frequency randomizer bounds.

    // Sample reference parameters.

    ALuint      sample_index;       // First (or only) sample (buffer) index.
    ALuint      sample_count;       // Sample amount to randomly select from.
}audio_effect_t, *audio_effect_p;

// Audio emitter (aka SoundSource) structure.

typedef struct audio_emitter_s
{
    ALuint      emitter_index;  // Unique emitter index.
    ALuint      sound_index;    // Sound index.
    float       position[3];    // Vector coordinate.
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
}audio_emitter_t, *audio_emitter_p;

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


class StreamTrackBuffer
{
public:
    StreamTrackBuffer();
   ~StreamTrackBuffer();

    bool Load(int track_index);

    uint8_t *GetBuffer()
    {
        return buffer;
    }

    size_t GetBufferSize()
    {
        return buffer_size;
    }

    ALenum GetFormat()
    {
        return format;
    }

    ALsizei GetRate()
    {
        return rate;
    }

    int GetType()
    {
        return stream_type;
    }

private:
    bool Load_Ogg(const char *path);     // Ogg file loading routine.
    bool Load_Wad(const char *path);     // Wad file loading routine.
    bool Load_Wav(const char *path);     // Wav file loading routine.

    int             track_index;
    uint32_t        buffer_size;
    uint8_t        *buffer;
    int             stream_type;         // Either BACKGROUND, ONESHOT or CHAT.
    ALenum          format;
    ALsizei         rate;
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

    void SetTrackInfo(int track_index, int stream_type_in)
    {
        current_track = track_index;
        stream_type   = stream_type_in;
        dampable      = (stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND);       // Damp only looped (BGM) tracks.
        buffer_offset = 0;
    }
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
    bool Stream(ALuint al_buffer);       // General stream routine.

    uint32_t        buffer_offset;
    // General OpenAL fields
    ALuint          source;
    ALuint          buffers[TR_AUDIO_STREAM_NUMBUFFERS];
    ALfloat         current_volume;     // Stream volume, considering fades.
    ALfloat         damped_volume;      // Additional damp volume multiplier.

    bool            active;             // Used when track is being faded by other one.
    bool            dampable;           // Specifies if track can be damped by others.
    int             stream_type;        // Either BACKGROUND, ONESHOT or CHAT.
    int             current_track;      // Needed to prevent same track sending.
};



// ========== GLOBALS ==============
ALfloat                     listener_position[3];
struct audio_settings_s     audio_settings = {0};
struct audio_fxmanager_s    fxManager = {0};
bool                        StreamTrack::damp_active = false;


size_t sdl_ov_fread(void *data, size_t size, size_t n, void *ctx)
{
    return SDL_RWread((SDL_RWops*)ctx, data, size, n);
}

int sdl_ov_fseek(void *ctx, ogg_int64_t offset, int whence)
{
    return SDL_RWseek((SDL_RWops*)ctx, offset, whence);
}

int sdl_ov_fclose(void *ctx)
{
    return SDL_RWclose((SDL_RWops*)ctx);
}

long sdl_ov_ftell(void *ctx)
{
    return SDL_RWtell((SDL_RWops*)ctx);
}

static ov_callbacks ov_sdl_callbacks =
{
    sdl_ov_fread,
    sdl_ov_fseek,
    sdl_ov_fclose,
    sdl_ov_ftell
};


struct audio_world_data_s
{
    uint32_t                        audio_emitters_count;   // Amount of audio emitters in level.
    struct audio_emitter_s         *audio_emitters;         // Audio emitters.

    uint32_t                        audio_map_count;        // Amount of overall effects in engine.
    int16_t                        *audio_map;              // Effect indexes.
    uint32_t                        audio_effects_count;    // Amount of available effects in level.
    struct audio_effect_s          *audio_effects;          // Effects and their parameters.

    uint32_t                        audio_buffers_count;    // Amount of samples.
    ALuint                         *audio_buffers;          // Samples.
    uint32_t                        audio_sources_count;    // Amount of runtime channels.
    AudioSource                    *audio_sources;          // Channels.

    uint32_t                        stream_tracks_count;    // Amount of stream track channels.
    StreamTrack                    *stream_tracks;          // Stream tracks.

    uint32_t                        stream_buffers_count;    // Amount of stream track source buffers.
    StreamTrackBuffer             **stream_buffers;

    uint32_t                        stream_track_map_count; // Stream track flag map count.
    uint8_t                        *stream_track_map;       // Stream track flag map.
} audio_world_data;


// ======== PRIVATE PROTOTYPES =============
void Audio_InitFX();
#ifdef HAVE_ALEXT_H
int  Audio_LoadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb);
#endif
bool Audio_FillALBuffer(ALuint buf_number, Uint8* buffer_data, Uint32 buffer_size, SDL_AudioSpec wav_spec, bool use_SDL_resampler = false);
int  Audio_LoadALbufferFromWAV_Mem(ALuint buf_number, uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size = 0);
int  Audio_LoadALbufferFromWAV_File(ALuint buf_number, const char *fname);
void Audio_LoadOverridedSamples();

int  Audio_GetFreeSource();
int  Audio_IsInRange(int entity_type, int entity_ID, float range, float gain);

void Audio_PauseAllSources();    // Used to pause all effects currently playing.
void Audio_StopAllSources();     // Used in audio deinit.
void Audio_ResumeAllSources();   // Used to resume all effects currently paused.
void Audio_UpdateSources();      // Main sound loop.
void Audio_UpdateListenerByCamera(struct camera_s *cam);
void Audio_UpdateListenerByEntity(struct entity_s *ent);

int  Audio_GetFreeStream();                         // Get free (stopped) stream.
int  Audio_IsTrackPlaying(uint32_t track_index);    // See if track is already playing.
int  Audio_TrackAlreadyPlayed(uint32_t track_index, int8_t mask = 0);     // Check if track played with given activation mask.
void Audio_UpdateStreams();                         // Update all streams.
void Audio_UpdateStreamsDamping();                  // See if there any damping tracks playing.
void Audio_PauseStreams(int stream_type = -1);      // Pause ALL streams (of specified type).
void Audio_ResumeStreams(int stream_type = -1);     // Resume ALL streams.

// Error handling routines.
int  Audio_LogALError(int error_marker = 0);    // AL-specific error handler.
void Audio_LogOGGError(int code);               // Ogg-specific error handler.


// ======== AUDIOSOURCE CLASS IMPLEMENTATION ========
AudioSource::AudioSource()
{
    active = false;
    emitter_ID =  -1;
    emitter_type = TR_AUDIO_EMITTER_ENTITY;
    effect_index = 0;
    sample_index = 0;
    sample_count = 0;
    is_water     = false;
    alGenSources(1, &source_index);

    if(alIsSource(source_index))
    {
        alSourcef(source_index, AL_MIN_GAIN, 0.0);
        alSourcef(source_index, AL_MAX_GAIN, 1.0);

#ifdef HAVE_ALEXT_H
        if(audio_settings.use_effects)
        {
            alSourcef(source_index, AL_ROOM_ROLLOFF_FACTOR, 1.0);
            alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,   AL_TRUE);
            alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
            alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 0.1);
        }
        else
        {
            alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 0.0);
        }
#endif
    }
}


AudioSource::~AudioSource()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
        alDeleteSources(1, &source_index);
    }
}


bool AudioSource::IsActive()
{
    return active;
}


void AudioSource::Play()
{
    if(alIsSource(source_index))
    {
        if(emitter_type == TR_AUDIO_EMITTER_GLOBAL)
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(source_index, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(source_index, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            if(audio_settings.use_effects)
            {
                UnsetFX();
            }
        }
        else
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_FALSE);
            LinkEmitter();

            if(audio_settings.use_effects)
            {
                SetFX();
                SetUnderwater();
            }
        }

        alSourcePlay(source_index);
        active = true;
    }
}


void AudioSource::Pause()
{
    if(alIsSource(source_index))
    {
        alSourcePause(source_index);
    }
}


void AudioSource::Stop()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
        active = false;
    }
}


void AudioSource::Update()
{
    ALint   state;
    ALfloat range, gain;

    alGetSourcei(source_index, AL_SOURCE_STATE, &state);

    // Disable and bypass source, if it is stopped.
    if(state == AL_STOPPED)
    {
        active = false;
        return;
    }

    // Stop source, if it is disabled, but still playing.
    if(!active && (state == AL_PLAYING))
    {
        Stop();
        return;
    }

    // Bypass source, if it is paused or global.
    if( (state == AL_PAUSED) || (emitter_type == TR_AUDIO_EMITTER_GLOBAL) )
    {
        return;
    }

    alGetSourcef(source_index, AL_GAIN, &gain);
    alGetSourcef(source_index, AL_MAX_DISTANCE, &range);

    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.
    if(Audio_IsInRange(emitter_type, emitter_ID, range, gain))
    {
        LinkEmitter();

        if( (audio_settings.use_effects) && (is_water != fxManager.water_state) )
        {
            SetUnderwater();
        }
    }
    else
    {
        Stop();
    }
}


void AudioSource::SetBuffer(ALint buffer)
{
    ALint buffer_index = audio_world_data.audio_buffers[buffer];

    if(alIsSource(source_index) && alIsBuffer(buffer_index))
    {
        alSourcei(source_index, AL_BUFFER, buffer_index);

        // For some reason, OpenAL sometimes produces "Invalid Operation" error here,
        // so there's extra debug info - maybe it'll help some day.

        /*
        if(Audio_LogALError(1))
        {
            int channels, bits, freq;

            alGetBufferi(buffer_index, AL_CHANNELS,  &channels);
            alGetBufferi(buffer_index, AL_BITS,      &bits);
            alGetBufferi(buffer_index, AL_FREQUENCY, &freq);

            Sys_DebugLog(SYS_LOG_FILENAME, "Erroneous buffer %d info: CH%d, B%d, F%d", buffer_index, channels, bits, freq);
        }
        */
    }
}


void AudioSource::SetLooping(ALboolean is_looping)
{
    alSourcei(source_index, AL_LOOPING, is_looping);
}


void AudioSource::SetGain(ALfloat gain_value)
{
    // Clamp gain value.
    gain_value = (gain_value > 1.0) ? (1.0) : (gain_value);
    gain_value = (gain_value < 0.0) ? (0.0) : (gain_value);

    alSourcef(source_index, AL_GAIN, gain_value * audio_settings.sound_volume);
}


void AudioSource::SetPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    pitch_value = (pitch_value < 0.1) ? (0.1) : (pitch_value);
    pitch_value = (pitch_value > 2.0) ? (2.0) : (pitch_value);

    alSourcef(source_index, AL_PITCH, pitch_value);
}


void AudioSource::SetRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(source_index, AL_REFERENCE_DISTANCE, range_value / 6.0);
    alSourcef(source_index, AL_MAX_DISTANCE, range_value);
}


void AudioSource::SetPosition(const ALfloat pos_vector[])
{
    alSourcefv(source_index, AL_POSITION, pos_vector);
}


void AudioSource::SetVelocity(const ALfloat vel_vector[])
{
    alSourcefv(source_index, AL_VELOCITY, vel_vector);
}


void AudioSource::SetFX()
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
        fxManager.current_slot   = (++fxManager.current_slot > (TR_AUDIO_MAX_SLOTS-1)) ? (0) : (fxManager.current_slot);

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

    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
#endif
}


void AudioSource::UnsetFX()
{
#ifdef HAVE_ALEXT_H
    // Remove any audio sends and direct filters from channel.
    alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
#endif
}

void AudioSource::SetUnderwater()
{
#ifdef HAVE_ALEXT_H
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(fxManager.water_state)
    {
        alSourcei(source_index, AL_DIRECT_FILTER, fxManager.al_filter);
        is_water = true;
    }
    else
    {
        alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
        is_water = false;
    }
#endif
}


void AudioSource::LinkEmitter()
{
    entity_p ent;

    switch(emitter_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            ent = World_GetEntityByID(emitter_ID);
            if(ent)
            {
                ALfloat  vec[3];
                vec3_copy(vec, ent->transform + 12);
                SetPosition(vec);
                vec3_copy(vec, ent->speed);
                SetVelocity(vec);
            }
            return;

        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            SetPosition(audio_world_data.audio_emitters[emitter_ID].position);
            return;
    }
}

// ==== STREAMTRACK BUFFER CLASS IMPLEMENTATION =====
StreamTrackBuffer::StreamTrackBuffer() :
    track_index(-1),
    buffer_size(0),
    buffer(NULL),
    stream_type(TR_AUDIO_STREAM_TYPE_ONESHOT),
    format(0),
    rate(0)
{
}


StreamTrackBuffer::~StreamTrackBuffer()
{
    if(buffer)
    {
        buffer_size = 0;
        free(buffer);
        buffer = NULL;
    }
}


bool StreamTrackBuffer::Load(int track_index)
{
    if(this->track_index < 0)
    {
        this->track_index = track_index;
        char file_path[1024];
        int load_method = 0;

        if(!Script_GetSoundtrack(engine_lua, track_index, file_path, &load_method, &stream_type))
        {
            return false;
        }

        switch(load_method)
        {
            case TR_AUDIO_STREAM_METHOD_OGG:
                return Load_Ogg(file_path);

            case TR_AUDIO_STREAM_METHOD_WAD:
                return Load_Wad(file_path);

            case TR_AUDIO_STREAM_METHOD_WAV:
                return Load_Wav(file_path);

            default:
                return false;
        }
    }

    return (this->buffer != NULL);
}


bool StreamTrackBuffer::Load_Ogg(const char *path)
{
    vorbis_info    *vorbis_Info = NULL;
    SDL_RWops      *audio_file = SDL_RWFromFile(path, "rb");
    OggVorbis_File  vorbis_Stream;
    bool            ret = false;

    if(!audio_file)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "OGG: Couldn't open file: %s.", path);
        return false;
    }

    memset(&vorbis_Stream, 0x00, sizeof(OggVorbis_File));
    if(ov_open_callbacks(audio_file, &vorbis_Stream, NULL, 0, ov_sdl_callbacks) < 0)
    {
        SDL_RWclose(audio_file);
        Sys_DebugLog(SYS_LOG_FILENAME, "OGG: Couldn't open Ogg stream.");
        return false;
    }

    vorbis_Info = ov_info(&vorbis_Stream, -1);
    format = (vorbis_Info->channels == 1) ? (AL_FORMAT_MONO16) : (AL_FORMAT_STEREO16);
    rate = vorbis_Info->rate;

    {
        const size_t temp_buf_size = 64 * 1024 * 1024;
        long int bitrate_nominal = vorbis_Info->bitrate_nominal;
        char *temp_buff = (char*)malloc(temp_buf_size);
        size_t readed = 0;
        buffer_size = 0;
        do
        {
            int section;
            readed = ov_read(&vorbis_Stream, temp_buff + buffer_size, 32768, 0, 2, 1, &section);
            buffer_size += readed;
            if(buffer_size + 32768 >= temp_buf_size)
            {
                buffer_size = 0;
                break;
            }
        }
        while(readed > 0);

        ov_clear(&vorbis_Stream);
        //SDL_RWclose(audio_file);   //ov_clear closes (vorbis_Stream->datasource == audio_file);

        if(buffer_size > 0)
        {
            buffer = (uint8_t*)malloc(buffer_size);
            memcpy(buffer, temp_buff, buffer_size);
            Con_Notify("file \"%s\" loaded with rate=%d, bitrate=%.1f", path, rate, ((float)bitrate_nominal / 1000));
            ret = true;
        }
        free(temp_buff);
    }

    return ret;
}


bool StreamTrackBuffer::Load_Wad(const char *path)
{
    return false;   ///@FIXME: PLACEHOLDER!!!
}


bool StreamTrackBuffer::Load_Wav(const char *path)
{
    SDL_AudioSpec wav_spec;
    uint8_t      *wav_buffer;
    uint32_t      wav_length;
    if(SDL_LoadWAV(path, &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't load track: \"%s\"", path);
        return false;
    }

    if(wav_spec.channels > 2)   // We can't use non-mono and barely can use stereo samples.
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Error: track \"%s\" has more than 2 channels!", path);
        return false;
    }

    // Extract bitsize from SDL audio spec for further usage.
    uint8_t sample_bitsize = (uint8_t)(wav_spec.format & SDL_AUDIO_MASK_BITSIZE);

    // Check if bitsize is supported.
    // We rarely encounter samples with exotic bitsizes, but just in case...
    if((sample_bitsize != 32) && (sample_bitsize != 16) && (sample_bitsize != 8))
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Can't load sample - wrong bitsize (%d)", sample_bitsize);
        return false;
    }

<<<<<<< HEAD
    // Standard OpenAL sample loading process.
    format = 0x00;
=======
    if(false)//use_SDL_resampler
    {
        int FrameSize = wav_spec.channels * 4; // sizeof(float);
        SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq, AUDIO_F32, wav_spec.channels, 44100);

        cvt.len = wav_length;
        buffer_size = wav_length * cvt.len_mult;
        if(buffer_size % FrameSize)
        {
            buffer_size += FrameSize - (buffer_size % FrameSize);   // make align
        }

        buffer = (uint8_t*)calloc(buffer_size, 1);
        cvt.buf = buffer;
        memcpy(cvt.buf, wav_buffer, cvt.len);
>>>>>>> opentomb/master

    if(wav_spec.channels == 1)
    {
        switch(sample_bitsize)
        {
            case 8:
                format = AL_FORMAT_MONO8;
                break;
            case 16:
                format = AL_FORMAT_MONO16;
                break;
#ifdef HAVE_ALEXT_H
            case 32:
                format = AL_FORMAT_MONO_FLOAT32;
                break;
#endif
        }
    }
    else
    {
        switch(sample_bitsize)
        {
            case 8:
                format = AL_FORMAT_STEREO8;
                break;
            case 16:
                format = AL_FORMAT_STEREO16;
                break;
#ifdef HAVE_ALEXT_H
            case 32:
                format = AL_FORMAT_STEREO_FLOAT32;
                break;
#endif
        }
<<<<<<< HEAD
=======

        buffer_size = wav_length;
        buffer = (uint8_t*)malloc(buffer_size);
        rate = wav_spec.freq;
        memcpy(buffer, wav_buffer, buffer_size);
>>>>>>> opentomb/master
    }

    buffer_size = wav_length;
    buffer_offset = 0;
    buffer = (uint8_t*)calloc(buffer_size, 1);
    rate = wav_spec.freq;
    memcpy(buffer, wav_buffer, buffer_size);
    

    SDL_FreeWAV(wav_buffer);
    return true;
}


// ======== STREAMTRACK CLASS IMPLEMENTATION ========
StreamTrack::StreamTrack() :
    buffer_offset(0),
    source(0),
    current_volume(0.0f),
    damped_volume(0.0f),
    active(false),
    dampable(false),
    stream_type(TR_AUDIO_STREAM_TYPE_ONESHOT),
    current_track(-1)
{
    alGenBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);              // Generate all buffers at once.
    alGenSources(1, &source);

    if(alIsSource(source))
    {
        alSource3f(source, AL_POSITION,        0.0f,  0.0f, -1.0f); // OpenAL tut says this.
        alSource3f(source, AL_VELOCITY,        0.0f,  0.0f,  0.0f);
        alSource3f(source, AL_DIRECTION,       0.0f,  0.0f,  0.0f);
        alSourcef (source, AL_ROLLOFF_FACTOR,  0.0f              );
        alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE           );
        alSourcei (source, AL_LOOPING,         AL_FALSE          ); // No effect, but just in case...
    }
}


StreamTrack::~StreamTrack()
{
    Stop(); // In case we haven't stopped yet.

    buffer_offset = 0;

    alDeleteSources(1, &source);
    alDeleteBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);
}


bool StreamTrack::Play(bool fade_in)
{
    int buffers_to_play = 0;

    // At start-up, we fill all available buffers.
    // TR soundtracks contain a lot of short tracks, like Lara speech etc., and
    // there is high chance that such short tracks won't fill all defined buffers.
    // For this reason, we count amount of filled buffers, and immediately stop
    // allocating them as long as Stream() routine returns false. Later, we use
    // this number for queuing buffers to source.

    for(int i = 0; i < TR_AUDIO_STREAM_NUMBUFFERS; i++, buffers_to_play++)
    {
        if(!Stream(buffers[i]))
        {
            if(i == 0)
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "StreamTrack: error preparing buffers.");
                return false;
            }
            else
            {
                break;
            }
        }
    }

    if(fade_in)     // If fade-in flag is set, do it.
    {
        current_volume = 0.0;
    }
    else
    {
        current_volume = 1.0;
    }

    if(audio_settings.use_effects)
    {
        if(stream_type == TR_AUDIO_STREAM_TYPE_CHAT)
        {
            SetFX();
        }
        else
        {
            UnsetFX();
        }
    }

    alSourcef(source, AL_GAIN, current_volume * audio_settings.music_volume);
    alSourceQueueBuffers(source, buffers_to_play, buffers);
    alSourcePlay(source);

    active = true;
    return   true;
}


void StreamTrack::Pause()
{
    if(alIsSource(source))
    {
        alSourcePause(source);
    }
}


void StreamTrack::End()     // Smoothly end track with fadeout.
{
    active = false;
}


void StreamTrack::Stop()    // Immediately stop track.
{
    active = false;         // Clear activity flag.
    buffer_offset = 0;
    current_track = -1;
    if(alIsSource(source))  // Stop and unlink all associated buffers.
    {
        if(IsPlaying())
        {
            alSourceStop(source);
        }

        ALint queued = 0;
        alGetSourcei(source, /*AL_BUFFERS_QUEUED*/AL_BUFFERS_PROCESSED, &queued);
        alSourceUnqueueBuffers(source, queued, buffers);
        alDeleteBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);
        alGenBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);
    }
}


bool StreamTrack::Update()
{
    int  processed     = 0;
    bool buffered      = true;
    bool change_gain   = false;

    // Update damping, if track supports it.
    if(dampable)
    {
        // We check if damp condition is active, and if so, is it already at low-level or not.
        if(damp_active && (damped_volume < TR_AUDIO_STREAM_DAMP_LEVEL))
        {
            damped_volume += TR_AUDIO_STREAM_DAMP_SPEED;

            // Clamp volume.
            damped_volume = (damped_volume > TR_AUDIO_STREAM_DAMP_LEVEL) ? (TR_AUDIO_STREAM_DAMP_LEVEL) : (damped_volume);
            change_gain   = true;
        }
        else if(!damp_active && (damped_volume > 0))    // If damp is not active, but it's still at low, restore it.
        {
            damped_volume -= TR_AUDIO_STREAM_DAMP_SPEED;

            // Clamp volume.
            damped_volume = (damped_volume < 0.0f) ? (0.0f) : (damped_volume);
            change_gain   = true;
        }
    }

    if(!active)     // If track has ended, crossfade it.
    {
        switch(stream_type)
        {
            case TR_AUDIO_STREAM_TYPE_BACKGROUND:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_BACKGROUND;
                break;

            case TR_AUDIO_STREAM_TYPE_ONESHOT:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_ONESHOT;
                break;

            case TR_AUDIO_STREAM_TYPE_CHAT:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_CHAT;
                break;
        }

        // Crossfade has ended, we can now kill the stream.
        if(current_volume <= 0.0)
        {
            Stop();
            return true;    // Stop track, although return success, as everything is normal.
        }
        else
        {
            change_gain = true;
        }
    }
    else
    {
        // If track is active and playing, restore it from crossfade.
        if(current_volume < 1.0)
        {
            switch(stream_type)
            {
                case TR_AUDIO_STREAM_TYPE_BACKGROUND:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_BACKGROUND;
                    break;

                case TR_AUDIO_STREAM_TYPE_ONESHOT:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_ONESHOT;
                    break;

                case TR_AUDIO_STREAM_TYPE_CHAT:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_CHAT;
                    break;
            }

            // Clamp volume.
            current_volume = (current_volume > 1.0f) ? (1.0f) : (current_volume);
            change_gain    = true;
        }
    }

    if(change_gain) // If any condition which modify track gain was met, call AL gain change.
    {
        alSourcef(source, AL_GAIN, current_volume              *  // Global track volume.
                                   (1.0f - damped_volume)      *  // Damp volume.
                                   audio_settings.music_volume);  // Global music volume setting.
    }

    // Check if any track buffers were already processed.

    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)  // Manage processed buffers.
    {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);     // Unlink processed buffer.
        buffered = Stream(buffer);                      // Refill processed buffer.
        if(buffered)
        {
            alSourceQueueBuffers(source, 1, &buffer);   // Relink processed buffer.
        }
    }

    return buffered;
}


bool StreamTrack::IsTrack(const int track_index)    // Check if track has specific index.
{
    return (current_track == track_index);
}


bool StreamTrack::IsType(const int track_type)      // Check if track has specific type.
{
    return (track_type == stream_type);
}


bool StreamTrack::IsActive()                         // Check if track is still active.
{
    return active;
}


bool StreamTrack::IsDampable()                      // Check if track is dampable.
{
    return dampable;
}


bool StreamTrack::IsPlaying()                       // Check if track is playing.
{
    ALenum state;

    if(alIsSource(source))
    {
       alGetSourcei(source, AL_SOURCE_STATE, &state);
    }

    // Paused also counts as playing.
    return ((state == AL_PLAYING) || (state == AL_PAUSED));
}


bool StreamTrack::Stream(ALuint al_buffer)             // Update stream process.
{
    uint8_t *buffer = NULL;
    StreamTrackBuffer *stb = NULL;
    size_t buffer_size = 0;
    bool ret = false;

    if(current_track >= 0)
    {
        stb = audio_world_data.stream_buffers[current_track];
        buffer = stb->GetBuffer();
        buffer_size = stb->GetBufferSize();
    }

    if(buffer && (buffer_offset + 1 < buffer_size))
    {
        if(buffer_offset + audio_settings.stream_buffer_size < buffer_size)
        {
            alBufferData(al_buffer, stb->GetFormat(), buffer + buffer_offset, audio_settings.stream_buffer_size, stb->GetRate());
            buffer_offset += audio_settings.stream_buffer_size;
            ret = true;
        }
        else if(stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND)
        {
            uint8_t pcm[audio_settings.stream_buffer_size];
            for(uint32_t size = 0; size < audio_settings.stream_buffer_size; size++)
            {
                pcm[size] = buffer[buffer_offset];
                buffer_offset++;
                if(buffer_offset >= buffer_size)
                {
                    buffer_offset = 0;
                }
            }
            alBufferData(al_buffer, stb->GetFormat(), pcm, audio_settings.stream_buffer_size, stb->GetRate());
            ret = true;
        }
        else
        {
            alBufferData(al_buffer, stb->GetFormat(), buffer + buffer_offset, buffer_size - buffer_offset, stb->GetRate());
            buffer_offset = buffer_size - 1;
            ret = false;
        }
    }

    return ret;
}


void StreamTrack::SetFX()
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
        fxManager.current_slot   = (++fxManager.current_slot > (TR_AUDIO_MAX_SLOTS-1))?(0):(fxManager.current_slot);

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


void StreamTrack::UnsetFX()
{
#ifdef HAVE_ALEXT_H
    // Remove any audio sends and direct filters from channel.

    alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
#endif
}

// ======== END STREAMTRACK CLASS IMPLEMENTATION ========


// General soundtrack playing routine. All native TR CD triggers and commands should ONLY
// call this one.

int Audio_StreamPlay(const uint32_t track_index, const uint8_t mask)
{
    int    target_stream = -1;
    bool   do_fade_in    =  false;

    // Don't even try to do anything with track, if its index is greater than overall amount of
    // soundtracks specified in a stream track map count (which is derived from script).

    if(track_index >= audio_world_data.stream_track_map_count)
    {
        Con_AddLine("StreamPlay: CANCEL, track index is out of bounds.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_WRONGTRACK;
    }

    // Don't play track, if it is already playing.
    // This should become useless option, once proper one-shot trigger functionality is implemented.

    if(Audio_IsTrackPlaying(track_index))
    {
        Con_AddLine("StreamPlay: CANCEL, stream already playing.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_IGNORED;
    }

    // lua_GetSoundtrack returns stream type, file path and load method in last three
    // provided arguments. That is, after calling this function we receive stream type
    // in "stream_type" argument, file path into "file_path" argument and load method into
    // "load_method" argument. Function itself returns false, if script wasn't found or
    // request was broken; in this case, we quit.

    StreamTrackBuffer *stb = audio_world_data.stream_buffers[track_index];
    if(stb == NULL)
    {
        Con_AddLine("StreamPlay: CANCEL, wrong track index or broken script.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_LOADERROR;
    }

    // Don't try to play track, if it was already played by specified bit mask.
    // Additionally, TrackAlreadyPlayed function applies specified bit mask to track map.
    // Also, bit mask is valid only for non-looped tracks, since looped tracks are played
    // in any way.

    int stream_type = stb->GetType();
    if((stream_type != TR_AUDIO_STREAM_TYPE_BACKGROUND) &&
        Audio_TrackAlreadyPlayed(track_index, mask))
    {
        return TR_AUDIO_STREAMPLAY_IGNORED;
    }

    // Entry found, now process to actual track loading.

    target_stream = Audio_GetFreeStream();            // At first, we need to get free stream.

    if(target_stream == -1)
    {
        do_fade_in = Audio_StopStreams(stream_type);  // If no free track found, hardly stop all tracks.
        target_stream = Audio_GetFreeStream();        // Try again to assign free stream.

        if(target_stream == -1)
        {
            Con_AddLine("StreamPlay: CANCEL, no free stream.", FONTSTYLE_CONSOLE_WARNING);
            return TR_AUDIO_STREAMPLAY_NOFREESTREAM;  // No success, exit and don't play anything.
        }
    }
    else
    {
        do_fade_in = Audio_EndStreams(stream_type);   // End all streams of this type with fadeout.

        // Additionally check if track type is looped. If it is, force fade in in any case.
        // This is needed to smooth out possible pop with gapless looped track at a start-up.
        do_fade_in |= (stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND);
    }

    // Finally - load our track.
    audio_world_data.stream_tracks[target_stream].SetTrackInfo(track_index, stb->GetType());

    // Try to play newly assigned and loaded track.
    if(!(audio_world_data.stream_tracks[target_stream].Play(do_fade_in)))
    {
        Con_AddLine("StreamPlay: CANCEL, stream play error.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_PLAYERROR;
    }

    return TR_AUDIO_STREAMPLAY_PROCESSED;   // Everything is OK!
}


// General damping update procedure. Constantly checks if damp condition exists, and
// if so, it lowers the volume of tracks which are dampable.

void Audio_UpdateStreamsDamping()
{
    StreamTrack::damp_active = false;   // Reset damp activity flag.

    // Scan for any tracks that can provoke damp. Usually it's any tracks that are
    // NOT background. So we simply check this condition and set damp activity flag
    // if condition is met.

    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if(audio_world_data.stream_tracks[i].IsPlaying())
        {
            if(!audio_world_data.stream_tracks[i].IsType(TR_AUDIO_STREAM_TYPE_BACKGROUND))
            {
                StreamTrack::damp_active = true;
                return; // No need to check more, we found at least one condition.
            }
        }
    }
}


// Update routine for all streams. Should be placed into main loop.
void Audio_UpdateStreams()
{
    Audio_UpdateStreamsDamping();

    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if(audio_world_data.stream_tracks[i].IsPlaying())
        {
            audio_world_data.stream_tracks[i].Update();
        }
        else
        {
            if(audio_world_data.stream_tracks[i].IsActive())
            {
                audio_world_data.stream_tracks[i].Stop();
            }
        }
    }
}


int  Audio_IsTrackPlaying(uint32_t track_index)
{
    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if(audio_world_data.stream_tracks[i].IsPlaying() &&
           audio_world_data.stream_tracks[i].IsTrack(track_index))
        {
            return 1;
        }
    }

    return 0;
}


int  Audio_TrackAlreadyPlayed(uint32_t track_index, int8_t mask)
{
    if(!mask)
    {
        return 0;   // No mask, play in any case.
    }

    if(track_index >= audio_world_data.stream_track_map_count)
    {
        return 1;    // No such track, hence "already" played.
    }
    else
    {
        mask &= 0x3F;   // Clamp mask just in case.

        if(audio_world_data.stream_track_map[track_index] == mask)
        {
            return true;    // Immediately return true, if flags are directly equal.
        }
        else
        {
            int8_t played = audio_world_data.stream_track_map[track_index] & mask;
            if(played == mask)
            {
                return 1;    // Bits were set, hence already played.
            }
            else
            {
                audio_world_data.stream_track_map[track_index] |= mask;
                return 0;   // Not yet played, set bits and return false.
            }
        }
    }
}


int Audio_GetFreeStream()
{
    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if( (!audio_world_data.stream_tracks[i].IsPlaying()) &&
            (!audio_world_data.stream_tracks[i].IsActive())   )
        {
            return i;
        }
    }

    return TR_AUDIO_STREAMPLAY_NOFREESTREAM;  // If no free source, return error.
}


int  Audio_StopStreams(int stream_type)
{
    int ret = 0;

    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if(audio_world_data.stream_tracks[i].IsPlaying()          &&
           (audio_world_data.stream_tracks[i].IsType(stream_type) ||
            stream_type == -1)) // Stop ALL streams at once.
        {
            audio_world_data.stream_tracks[i].Stop();
            ret++;
        }
    }

    return ret;
}


int  Audio_EndStreams(int stream_type)
{
    int ret = 0;

    for(uint32_t i = 0; i < audio_world_data.stream_tracks_count; i++)
    {
        if( (stream_type == -1) ||                              // End ALL streams at once.
            ((audio_world_data.stream_tracks[i].IsPlaying()) &&
             (audio_world_data.stream_tracks[i].IsType(stream_type))) )
        {
            audio_world_data.stream_tracks[i].End();
            ret++;
        }
    }

    return ret;
}

// ======== Audio source global methods ========
int  Audio_IsInRange(int entity_type, int entity_ID, float range, float gain)
{
    ALfloat  vec[3] = {0.0, 0.0, 0.0}, dist;
    entity_p ent;

    switch(entity_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            ent = World_GetEntityByID(entity_ID);
            if(!ent)
            {
                return 0;
            }
            vec3_copy(vec, ent->transform + 12);
            break;

        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            if((uint32_t)entity_ID + 1 > audio_world_data.audio_emitters_count)
            {
                return 0;
            }
            vec3_copy(vec, audio_world_data.audio_emitters[entity_ID].position);
            break;

        case TR_AUDIO_EMITTER_GLOBAL:
            return 1;

        default:
            return 0;
    }

    dist = vec3_dist_sq(listener_position, vec);

    // We add 1/4 of overall distance to fix up some issues with
    // pseudo-looped sounds that are called at certain frames in animations.

    dist /= (gain + 1.25);

    return dist < range * range;
}


void Audio_UpdateSources()
{
    if(audio_world_data.audio_sources_count < 1)
    {
        return;
    }

    alGetListenerfv(AL_POSITION, listener_position);

    for(uint32_t i = 0; i < audio_world_data.audio_emitters_count; i++)
    {
        Audio_Send(audio_world_data.audio_emitters[i].sound_index, TR_AUDIO_EMITTER_SOUNDSOURCE, i);
    }

    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        audio_world_data.audio_sources[i].Update();
    }
}


void Audio_PauseAllSources()
{
    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        if(audio_world_data.audio_sources[i].IsActive())
        {
            audio_world_data.audio_sources[i].Pause();
        }
    }
}


void Audio_StopAllSources()
{
    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        audio_world_data.audio_sources[i].Stop();
    }
}


void Audio_ResumeAllSources()
{
    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        if(audio_world_data.audio_sources[i].IsActive())
        {
            audio_world_data.audio_sources[i].Play();
        }
    }
}


int Audio_GetFreeSource()   ///@FIXME: add condition (compare max_dist with new source dist)
{
    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        if(audio_world_data.audio_sources[i].IsActive() == false)
        {
            return i;
        }
    }

    return -1;
}


int Audio_IsEffectPlaying(int effect_ID, int entity_type, int entity_ID)
{
    for(uint32_t i = 0; i < audio_world_data.audio_sources_count; i++)
    {
        if( (audio_world_data.audio_sources[i].emitter_type == (uint32_t)entity_type) &&
            (audio_world_data.audio_sources[i].emitter_ID   == ( int32_t)entity_ID  ) &&
            (audio_world_data.audio_sources[i].effect_index == (uint32_t)effect_ID  ) &&
            audio_world_data.audio_sources[i].IsActive())
        {
            int state;
            alGetSourcei(audio_world_data.audio_sources[i].source_index, AL_SOURCE_STATE, &state);
            if(state == AL_PLAYING)
            {
                return i;
            }
        }
    }

    return -1;
}


int Audio_Send(int effect_ID, int entity_type, int entity_ID)
{
    int32_t         source_number;
    uint16_t        random_value;
    ALfloat         random_float;
    audio_effect_p  effect = NULL;
    AudioSource    *source = NULL;

    // If there are no audio buffers or effect index is wrong, don't process.
    if((audio_world_data.audio_buffers_count < 1) || (effect_ID < 0))
    {
        return TR_AUDIO_SEND_IGNORED;
    }

    // Remap global engine effect ID to local effect ID.
    if((uint32_t)effect_ID >= audio_world_data.audio_map_count)
    {
        return TR_AUDIO_SEND_NOSAMPLE;  // Sound is out of bounds; stop.
    }

    int real_ID = (int)audio_world_data.audio_map[effect_ID];

    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.

    if(real_ID == -1)
    {
        return TR_AUDIO_SEND_NOSAMPLE;
    }
    else
    {
        effect = audio_world_data.audio_effects + real_ID;
    }

    // Pre-step 2: check if sound non-looped and chance to play isn't zero,
    // then randomly select if it should be played or not.

    if((effect->loop != TR_AUDIO_LOOP_LOOPED) && (effect->chance > 0))
    {
        random_value = rand() % 0x7FFF;
        if(effect->chance < random_value)
        {
            // Bypass audio send, if chance test is not passed.
            return TR_AUDIO_SEND_IGNORED;
        }
    }

    // Pre-step 3: Calculate if effect's hearing sphere intersect listener's hearing sphere.
    // If it's not, bypass audio send (cause we don't want it to occupy channel, if it's not
    // heard).

    if(Audio_IsInRange(entity_type, entity_ID, effect->range, effect->gain ) == false)
    {
        return TR_AUDIO_SEND_IGNORED;
    }

    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and stop it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.

    source_number = Audio_IsEffectPlaying(effect_ID, entity_type, entity_ID);

    if(source_number != -1)
    {
        if(effect->loop == TR_AUDIO_LOOP_REWIND)
        {
            audio_world_data.audio_sources[source_number].Stop();
        }
        else if(effect->loop) // Any other looping case (Wait / Loop).
        {
            return TR_AUDIO_SEND_IGNORED;
        }
    }
    else
    {
        source_number = Audio_GetFreeSource();  // Get free source.
    }

    if(source_number != -1)  // Everything is OK, we're sending audio to channel.
    {
        int buffer_index;

        // Step 1. Assign buffer to source.

        if(effect->sample_count > 1)
        {
            // Select random buffer, if effect info contains more than 1 assigned samples.
            random_value = rand() % (effect->sample_count);
            buffer_index = random_value + effect->sample_index;
        }
        else
        {
            // Just assign buffer to source, if there is only one assigned sample.
            buffer_index = effect->sample_index;
        }

        source = &audio_world_data.audio_sources[source_number];

        source->SetBuffer(buffer_index);

        // Step 2. Check looped flag, and if so, set source type to looped.

        if(effect->loop == TR_AUDIO_LOOP_LOOPED)
        {
            source->SetLooping(AL_TRUE);
        }
        else
        {
            source->SetLooping(AL_FALSE);
        }

        // Step 3. Apply internal sound parameters.

        source->emitter_ID   = entity_ID;
        source->emitter_type = entity_type;
        source->effect_index = effect_ID;

        // Step 4. Apply sound effect properties.

        if(effect->rand_pitch)  // Vary pitch, if flag is set.
        {
            random_float = rand() % effect->rand_pitch_var;
            random_float = effect->pitch + ((random_float - 25.0) / 200.0);
            source->SetPitch(random_float);
        }
        else
        {
            source->SetPitch(effect->pitch);
        }

        if(effect->rand_gain)   // Vary gain, if flag is set.
        {
            random_float = rand() % effect->rand_gain_var;
            random_float = effect->gain + (random_float - 25.0) / 200.0;
            source->SetGain(random_float);
        }
        else
        {
            source->SetGain(effect->gain);
        }

        source->SetRange(effect->range);    // Set audible range.

        source->Play();                     // Everything is OK, play sound now!

        return TR_AUDIO_SEND_PROCESSED;
    }
    else
    {
        return TR_AUDIO_SEND_NOCHANNEL;
    }
}


int Audio_Kill(int effect_ID, int entity_type, int entity_ID)
{
    int playing_sound = Audio_IsEffectPlaying(effect_ID, entity_type, entity_ID);

    if(playing_sound != -1)
    {
        audio_world_data.audio_sources[playing_sound].Stop();
        return TR_AUDIO_SEND_PROCESSED;
    }

    return TR_AUDIO_SEND_IGNORED;
}


void Audio_LoadOverridedSamples()
{
    int  num_samples, num_sounds;
    int  sample_index, sample_count;
    char sample_name_mask[256];
    char sample_name[256];

    if(Script_GetOverridedSamplesInfo(engine_lua, &num_samples, &num_sounds, sample_name_mask))
    {
        int buffer_counter = 0;

        for(uint32_t i = 0; i < audio_world_data.audio_map_count; i++)
        {
            if(audio_world_data.audio_map[i] != -1)
            {
                if(Script_GetOverridedSample(engine_lua, i, &sample_index, &sample_count))
                {
                    for(int j = 0; j < sample_count; j++, buffer_counter++)
                    {
                        snprintf(sample_name, sizeof(sample_name), sample_name_mask, (sample_index + j));
                        if(Sys_FileFound(sample_name, 0))
                        {
                            Audio_LoadALbufferFromWAV_File(audio_world_data.audio_buffers[buffer_counter], sample_name);
                        }
                    }
                }
                else
                {
                    buffer_counter += audio_world_data.audio_effects[(audio_world_data.audio_map[i])].sample_count;
                }
            }
        }
    }
}


void Audio_InitGlobals()
{
    audio_settings.music_volume = 0.7;
    audio_settings.sound_volume = 0.8;
    audio_settings.use_effects  = true;
    audio_settings.listener_is_player = false;
    audio_settings.stream_buffer_size = 32;

    audio_world_data.audio_sources = NULL;
    audio_world_data.audio_sources_count = 0;
    audio_world_data.audio_buffers = NULL;
    audio_world_data.audio_buffers_count = 0;
    audio_world_data.audio_effects = NULL;
    audio_world_data.audio_effects_count = 0;

    audio_world_data.stream_tracks = NULL;
    audio_world_data.stream_tracks_count = 0;
    audio_world_data.stream_track_map = NULL;
    audio_world_data.stream_track_map_count = 0;
}


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
        Sys_DebugLog(SYS_LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return 0;
    }

    return 1;
}
#endif


void Audio_Init(uint32_t num_Sources)
{
    // FX should be inited first, as source constructor checks for FX slot to be created.

    if(audio_settings.use_effects)
    {
        Audio_InitFX();
    }

    // Generate new source array.

    num_Sources -= TR_AUDIO_STREAM_NUMSOURCES;          // Subtract sources reserved for music.
    audio_world_data.audio_sources_count = num_Sources;
    audio_world_data.audio_sources = new AudioSource[num_Sources];

    // Generate stream tracks array.

    audio_world_data.stream_tracks_count = TR_AUDIO_STREAM_NUMSOURCES;
    audio_world_data.stream_tracks = new StreamTrack[TR_AUDIO_STREAM_NUMSOURCES];

    // Reset last room type used for assigning reverb.

    fxManager.last_room_type = TR_AUDIO_FX_LASTINDEX;
}


bool Audio_IsTrackUsedInTriggers(int track_index)
{
    room_p rooms = NULL;
    uint32_t rooms_count = 0;
    World_GetRoomInfo(&rooms, &rooms_count);
    for(uint32_t room_index = 0; room_index < rooms_count; room_index++)
    {
        room_p r = rooms + room_index;
        for(uint32_t sector_index = 0; sector_index < r->sectors_count; sector_index++)
        {
            room_sector_p rs = r->sectors + sector_index;
            if(rs->trigger)
            {
                for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                {
                    if((cmd->function == TR_FD_TRIGFUNC_PLAYTRACK) && (cmd->operands == track_index))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


void Audio_GenSamples(class VT_Level *tr)
{
    uint8_t      *pointer = tr->samples_data;
    int8_t        flag;
    uint32_t      ind1, ind2;
    uint32_t      comp_size, uncomp_size;
    uint32_t      i;

    // Generate stream tracks buffers
    audio_world_data.stream_buffers = NULL;
    audio_world_data.stream_buffers_count = Script_GetNumTracks(engine_lua);
    if(audio_world_data.stream_buffers_count > 0)
    {
        int secret_track_index = Script_GetSecretTrackNumber(engine_lua);
        audio_world_data.stream_buffers = (StreamTrackBuffer**)malloc(audio_world_data.stream_buffers_count * sizeof(StreamTrackBuffer));
        for(uint32_t i = 0; i < audio_world_data.stream_buffers_count; i++)
        {
            audio_world_data.stream_buffers[i] = NULL;
            if((i == secret_track_index) || Audio_IsTrackUsedInTriggers(i))
            {
                StreamTrackBuffer *stb = new StreamTrackBuffer();
                if(stb->Load(i))
                {
                    audio_world_data.stream_buffers[i] = stb;
                }
                else
                {
                    delete stb;
                }
            }
        }
    }

    // Generate new buffer array.
    audio_world_data.audio_buffers_count = tr->samples_count;
    audio_world_data.audio_buffers = (ALuint*)malloc(audio_world_data.audio_buffers_count * sizeof(ALuint));
    memset(audio_world_data.audio_buffers, 0, sizeof(ALuint) * audio_world_data.audio_buffers_count);
    alGenBuffers(audio_world_data.audio_buffers_count, audio_world_data.audio_buffers);

    // Generate stream track map array.
    // We use scripted amount of tracks to define map bounds.
    // If script had no such parameter, we define map bounds by default.
    audio_world_data.stream_track_map_count = Script_GetNumTracks(engine_lua);
    if(audio_world_data.stream_track_map_count == 0) audio_world_data.stream_track_map_count = TR_AUDIO_STREAM_MAP_SIZE;
    audio_world_data.stream_track_map = (uint8_t*)malloc(audio_world_data.stream_track_map_count * sizeof(uint8_t));
    memset(audio_world_data.stream_track_map, 0, sizeof(uint8_t) * audio_world_data.stream_track_map_count);

    // Generate new audio effects array.
    audio_world_data.audio_effects_count = tr->sound_details_count;
    audio_world_data.audio_effects =  (audio_effect_t*)malloc(tr->sound_details_count * sizeof(audio_effect_t));
    memset(audio_world_data.audio_effects, 0xFF, sizeof(audio_effect_t) * tr->sound_details_count);

    // Generate new audio emitters array.
    audio_world_data.audio_emitters_count = tr->sound_sources_count;
    audio_world_data.audio_emitters = (audio_emitter_t*)malloc(tr->sound_sources_count * sizeof(audio_emitter_t));
    memset(audio_world_data.audio_emitters, 0, sizeof(audio_emitter_t) * tr->sound_sources_count);

    // Copy sound map.
    audio_world_data.audio_map = tr->soundmap;
    tr->soundmap = NULL;                   /// without it VT destructor free(tr->soundmap)

    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(pointer)
    {
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                audio_world_data.audio_map_count = TR_AUDIO_MAP_SIZE_TR1;

                for(i = 0; i < audio_world_data.audio_buffers_count-1; i++)
                {
                    pointer = tr->samples_data + tr->sample_indices[i];
                    uint32_t size = tr->sample_indices[i + 1] - tr->sample_indices[i];
                    Audio_LoadALbufferFromWAV_Mem(audio_world_data.audio_buffers[i], pointer, size);
                }
                i = audio_world_data.audio_buffers_count-1;
                Audio_LoadALbufferFromWAV_Mem(audio_world_data.audio_buffers[i], pointer, (tr->samples_count - tr->sample_indices[i]));
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
                audio_world_data.audio_map_count = (tr->game_version == TR_III) ? (TR_AUDIO_MAP_SIZE_TR3) : (TR_AUDIO_MAP_SIZE_TR2);
                ind1 = 0;
                ind2 = 0;
                flag = 0;
                i = 0;
                while(pointer < tr->samples_data + tr->samples_data_size - 4)
                {
                    pointer = tr->samples_data + ind2;
                    if(!memcmp(pointer, "RIFF", 4))
                    {
                        if(flag == 0x00)
                        {
                            ind1 = ind2;
                            flag = 0x01;
                        }
                        else
                        {
                            uncomp_size = ind2 - ind1;
                            Audio_LoadALbufferFromWAV_Mem(audio_world_data.audio_buffers[i], tr->samples_data + ind1, uncomp_size);
                            i++;
                            if(i > audio_world_data.audio_buffers_count - 1)
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                uncomp_size = tr->samples_data_size - ind1;
                pointer = tr->samples_data + ind1;
                if(i < audio_world_data.audio_buffers_count)
                {
                    Audio_LoadALbufferFromWAV_Mem(audio_world_data.audio_buffers[i], pointer, uncomp_size);
                }
                break;

            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                audio_world_data.audio_map_count = (tr->game_version == TR_V) ? (TR_AUDIO_MAP_SIZE_TR5) : (TR_AUDIO_MAP_SIZE_TR4);

                for(i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromWAV_Mem(audio_world_data.audio_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                audio_world_data.audio_map_count = TR_AUDIO_MAP_SIZE_NONE;
                free(tr->samples_data);
                tr->samples_data = NULL;
                tr->samples_data_size = 0;
                return;
        }

        free(tr->samples_data);
        tr->samples_data = NULL;
        tr->samples_data_size = 0;
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(i = 0; i < audio_world_data.audio_effects_count; i++)
    {
        if(tr->game_version < TR_III)
        {
            audio_world_data.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 32767.0; // Max. volume in TR1/TR2 is 32767.
            audio_world_data.audio_effects[i].chance = tr->sound_details[i].chance;
        }
        else if(tr->game_version > TR_III)
        {
            audio_world_data.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            audio_world_data.audio_effects[i].chance = tr->sound_details[i].chance * 255;
        }
        else
        {
            audio_world_data.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            audio_world_data.audio_effects[i].chance = tr->sound_details[i].chance * 127;
        }

        audio_world_data.audio_effects[i].rand_gain_var  = 50;
        audio_world_data.audio_effects[i].rand_pitch_var = 50;

        audio_world_data.audio_effects[i].pitch = (float)(tr->sound_details[i].pitch) / 127.0 + 1.0;
        audio_world_data.audio_effects[i].range = (float)(tr->sound_details[i].sound_range) * 1024.0;

        audio_world_data.audio_effects[i].rand_pitch = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_PITCH);
        audio_world_data.audio_effects[i].rand_gain  = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_VOLUME);

        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    case 0x01:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    default:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            case TR_II:
            case TR_II_DEMO:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    /*case 0x02:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;*/
                    case 0x01:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    case 0x03:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    default:
                        audio_world_data.audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            default:
                audio_world_data.audio_effects[i].loop = (tr->sound_details[i].num_samples_and_flags_1 & TR_AUDIO_LOOP_LOOPED);
                break;
        }

        audio_world_data.audio_effects[i].sample_index =  tr->sound_details[i].sample;
        audio_world_data.audio_effects[i].sample_count = (tr->sound_details[i].num_samples_and_flags_1 >> 2) & TR_AUDIO_SAMPLE_NUMBER_MASK;
    }

    // Try to override samples via script.
    // If there is no script entry exist, we just leave default samples.
    // NB! We need to override samples AFTER audio effects array is inited, as override
    //     routine refers to existence of certain audio effect in level.

    Audio_LoadOverridedSamples();

    // Hardcoded version-specific fixes!

    switch(tr->game_version)
    {
        case TR_I:
        case TR_I_DEMO:
        case TR_I_UB:
            // Fix for underwater looped sound.
            if ((audio_world_data.audio_map[TR_AUDIO_SOUND_UNDERWATER]) >= 0)
            {
                audio_world_data.audio_effects[(audio_world_data.audio_map[TR_AUDIO_SOUND_UNDERWATER])].loop = TR_AUDIO_LOOP_LOOPED;
            }
            break;
        case TR_II:
            // Fix for helicopter sound range.
            if ((audio_world_data.audio_map[297]) >= 0)
            {
                audio_world_data.audio_effects[(audio_world_data.audio_map[297])].range *= 10.0;
            }
            break;
    }

    // Cycle through sound emitters and
    // parse them to native OpenTomb sound emitters structure.

    for(i = 0; i < audio_world_data.audio_emitters_count; i++)
    {
        audio_world_data.audio_emitters[i].emitter_index = i;
        audio_world_data.audio_emitters[i].sound_index   =  tr->sound_sources[i].sound_id;
        audio_world_data.audio_emitters[i].position[0]   =  tr->sound_sources[i].x;
        audio_world_data.audio_emitters[i].position[1]   =  tr->sound_sources[i].z;
        audio_world_data.audio_emitters[i].position[2]   = -tr->sound_sources[i].y;
        audio_world_data.audio_emitters[i].flags         =  tr->sound_sources[i].flags;
    }
}


int Audio_DeInit()
{
    Audio_StopAllSources();
    Audio_StopStreams();

    if(audio_world_data.audio_sources)
    {
        audio_world_data.audio_sources_count = 0;
        delete[] audio_world_data.audio_sources;
        audio_world_data.audio_sources = NULL;
    }

    if(audio_world_data.audio_emitters)
    {
        audio_world_data.audio_emitters_count = 0;
        free(audio_world_data.audio_emitters);
        audio_world_data.audio_emitters = NULL;
    }

    if(audio_world_data.stream_tracks)
    {
        audio_world_data.stream_tracks_count = 0;
        delete[] audio_world_data.stream_tracks;
        audio_world_data.stream_tracks = NULL;
    }

    if(audio_world_data.stream_track_map)
    {
        audio_world_data.stream_track_map_count = 0;
        free(audio_world_data.stream_track_map);
        audio_world_data.stream_track_map = NULL;
    }

    ///@CRITICAL: You must delete all sources before buffers deleting!!!

    if(audio_world_data.audio_buffers)
    {
        alDeleteBuffers(audio_world_data.audio_buffers_count, audio_world_data.audio_buffers);
        audio_world_data.audio_buffers_count = 0;
        free(audio_world_data.audio_buffers);
        audio_world_data.audio_buffers = NULL;
    }

    if(audio_world_data.audio_effects)
    {
        audio_world_data.audio_effects_count = 0;
        free(audio_world_data.audio_effects);
        audio_world_data.audio_effects = NULL;
    }

    if(audio_world_data.audio_map)
    {
        audio_world_data.audio_map_count = 0;
        free(audio_world_data.audio_map);
        audio_world_data.audio_map = NULL;
    }

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

    if(audio_world_data.stream_buffers)
    {
        for(uint32_t i = 0; i < audio_world_data.stream_buffers_count; i++)
        {
            if(audio_world_data.stream_buffers[i])
            {
                delete audio_world_data.stream_buffers[i];
            }
            audio_world_data.stream_buffers[i] = NULL;
        }
        audio_world_data.stream_buffers_count = 0;
        free(audio_world_data.stream_buffers);
        audio_world_data.stream_buffers = NULL;
    }

    return 1;
}


int  Audio_LogALError(int error_marker)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "OpenAL error: %s / %d", alGetString(err), error_marker);
    }
    return err;
}


void Audio_LogOGGError(int code)
{
    switch(code)
    {
        case OV_EREAD:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Read from media.");
            break;
        case OV_ENOTVORBIS:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Not Vorbis data.");
            break;
        case OV_EVERSION:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Vorbis version mismatch.");
            break;
        case OV_EBADHEADER:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Invalid Vorbis header.");
            break;
        case OV_EFAULT:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Internal logic fault (bug or heap/stack corruption.");
            break;
        default:
            Sys_DebugLog(SYS_LOG_FILENAME, "OGG error: Unknown Ogg error.");
            break;
    }
}


int Audio_LoadALbufferFromWAV_Mem(ALuint buf_number, uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size)
{
    SDL_AudioSpec wav_spec;
    Uint8        *wav_buffer;
    Uint32        wav_length;

    SDL_RWops *src = SDL_RWFromMem(sample_pointer, sample_size);

    // Decode WAV structure with SDL methods.
    // SDL automatically defines file format (PCM/ADPCM), so we shouldn't bother
    // about if it is TR4 compressed samples or TRLE uncompressed samples.

    if(SDL_LoadWAV_RW(src, 1, &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't load sample #%03d from sample block!", buf_number);
        return -1;
    }

    // Uncomp_sample_size explicitly specifies amount of raw sample data
    // to load into buffer. It is only used in TR4/5 with ADPCM samples,
    // because full-sized ADPCM sample contains a bit of silence at the end,
    // which should be removed. That's where uncomp_sample_size comes into
    // business.
    // Note that we also need to compare if uncomp_sample_size is smaller
    // than native wav length, because for some reason many TR5 uncomp sizes
    // are messed up and actually more than actual sample size.

    if((uncomp_sample_size == 0) || (wav_length < uncomp_sample_size))
    {
        uncomp_sample_size = wav_length;
    }

    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats in same level.

    bool result = Audio_FillALBuffer(buf_number, wav_buffer, uncomp_sample_size, wav_spec);

    SDL_FreeWAV(wav_buffer);

    return (result) ? (0) : (-3);   // Zero means success.
}


int Audio_LoadALbufferFromWAV_File(ALuint buf_number, const char *fname)
{
    SDL_RWops     *file;
    SDL_AudioSpec  wav_spec;
    Uint8         *wav_buffer;
    Uint32         wav_length;

    file = SDL_RWFromFile(fname, "rb");

    if(!file)
    {
        Con_Warning("file \"%s\" not exists", fname);
        return -1;
    }

    if(SDL_LoadWAV_RW(file, 1, &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        SDL_RWclose(file);
        Con_Warning("file \"%s\" has wrog format", fname);
        return -2;
    }
    SDL_RWclose(file);

    bool result = Audio_FillALBuffer(buf_number, wav_buffer, wav_length, wav_spec);

    SDL_FreeWAV(wav_buffer);

    return (result) ? (0) : (-3);   // Zero means success.
}


bool Audio_FillALBuffer(ALuint buf_number, Uint8* buffer_data, Uint32 buffer_size, SDL_AudioSpec wav_spec, bool use_SDL_resampler)
{
    if(wav_spec.channels > 2)   // We can't use non-mono and barely can use stereo samples.
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Error: sample %03d has more than 2 channels!", buf_number);
        return false;
    }

    // Extract bitsize from SDL audio spec for further usage.
    uint8_t sample_bitsize = (uint8_t)(wav_spec.format & SDL_AUDIO_MASK_BITSIZE);

    // Check if bitsize is supported.
    // We rarely encounter samples with exotic bitsizes, but just in case...
    if((sample_bitsize != 32) && (sample_bitsize != 16) && (sample_bitsize != 8))
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "Can't load sample - wrong bitsize (%d)", sample_bitsize);
        return false;
    }

    // Standard OpenAL sample loading process.
        ALenum sample_format = 0x00;

        if(wav_spec.channels == 1)
        {
            switch(sample_bitsize)
            {
                case 8:
                    sample_format = AL_FORMAT_MONO8;
                    break;
                case 16:
                    sample_format = AL_FORMAT_MONO16;
                    break;
#ifdef HAVE_ALEXT_H
                case 32:
                    sample_format = AL_FORMAT_MONO_FLOAT32;
                    break;
#endif
            }
        }
        else
        {
            switch(sample_bitsize)
            {
                case 8:
                    sample_format = AL_FORMAT_STEREO8;
                    break;
                case 16:
                    sample_format = AL_FORMAT_STEREO16;
                    break;
#ifdef HAVE_ALEXT_H
                case 32:
                    sample_format = AL_FORMAT_STEREO_FLOAT32;
                    break;
#endif
            }
        }

        alBufferData(buf_number, sample_format, buffer_data, buffer_size, wav_spec.freq);

    return true;
}


/**
 * Updates listener parameters by camera structure. For correct speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void Audio_UpdateListenerByCamera(struct camera_s *cam)
{
    ALfloat v[6];       // vec3 - forvard, vec3 - up

    vec3_copy(v + 0, cam->gl_transform + 8);   // cam_OZ
    vec3_copy(v + 3, cam->gl_transform + 4);   // cam_OY
    alListenerfv(AL_ORIENTATION, v);

    vec3_copy(v, cam->gl_transform + 12);
    alListenerfv(AL_POSITION, v);

    vec3_sub(v, cam->gl_transform + 12, cam->prev_pos);
    v[3] = 1.0 / engine_frame_time;
    vec3_mul_scalar(v, v, v[3]);
    alListenerfv(AL_VELOCITY, v);
    vec3_copy(cam->prev_pos, cam->gl_transform + 12);

    if(cam->current_room)
    {
        if(cam->current_room->flags & TR_ROOM_FLAG_WATER)
        {
            fxManager.current_room_type = TR_AUDIO_FX_WATER;
        }
        else
        {
            fxManager.current_room_type = cam->current_room->content->reverb_info;
        }

        if(fxManager.water_state != (int8_t)(cam->current_room->flags & TR_ROOM_FLAG_WATER))
        {
            fxManager.water_state = cam->current_room->flags & TR_ROOM_FLAG_WATER;

            if(fxManager.water_state)
            {
                Audio_Send(TR_AUDIO_SOUND_UNDERWATER);
            }
            else
            {
                Audio_Kill(TR_AUDIO_SOUND_UNDERWATER);
            }
        }
    }
}


void Audio_UpdateListenerByEntity(struct entity_s *ent)
{
    ///@FIXME: Add entity listener updater here.
}


void Audio_Update(float time)
{
    static float game_logic_time  = 0.0;
    game_logic_time += time;

    if(game_logic_time >= GAME_LOGIC_REFRESH_INTERVAL)
    {
        int32_t t = game_logic_time / GAME_LOGIC_REFRESH_INTERVAL;
        float dt = (float)t * GAME_LOGIC_REFRESH_INTERVAL;
        game_logic_time -= dt;

        Audio_UpdateSources();
        Audio_UpdateStreams();
        Audio_UpdateListenerByCamera(&engine_camera);
    }
}
