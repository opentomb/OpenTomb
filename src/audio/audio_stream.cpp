
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <math.h>

extern "C" {
#include <al.h>
#include <alc.h>
}

#include "audio.h"
#include "audio_stream.h"


// ======== PRIVATE PROTOTYPES =============
struct stream_internal_s
{
    ALuint          source;
    ALuint          buffers[TR_AUDIO_STREAM_NUMBUFFERS];
};
    
bool Audio_FillALBuffer(ALuint buf_number, Uint8* buffer_data, Uint32 buffer_size, int sample_bitsize, int channels, int frequency);
void Audio_SetFX(ALuint source);
void Audio_UnsetFX(ALuint source);


void StreamTrack_Init(stream_track_p s)
{
    s->type = TR_AUDIO_STREAM_TYPE_ONESHOT;
    s->state = TR_AUDIO_STREAM_STOPPED;
    s->linked_buffers = 0;
    s->buffer_offset = 0;
    s->current_volume = 0.0f;
    s->track = -1;
    s->internal = (struct stream_internal_s*)malloc(sizeof(struct stream_internal_s));
    alGenBuffers(TR_AUDIO_STREAM_NUMBUFFERS, s->internal->buffers);
    alGenSources(1, &s->internal->source);
    if(alIsSource(s->internal->source))
    {
        alSource3f(s->internal->source, AL_POSITION,        0.0f,  0.0f, -1.0f); // OpenAL tut says this.
        alSource3f(s->internal->source, AL_VELOCITY,        0.0f,  0.0f,  0.0f);
        alSource3f(s->internal->source, AL_DIRECTION,       0.0f,  0.0f,  0.0f);
        alSourcef (s->internal->source, AL_ROLLOFF_FACTOR,  0.0f              );
        alSourcei (s->internal->source, AL_SOURCE_RELATIVE, AL_TRUE           );
        alSourcei (s->internal->source, AL_LOOPING,         AL_FALSE          ); // No effect, but just in case...
    }
}

void StreamTrack_Clear(stream_track_p s)
{
    StreamTrack_Stop(s);
    s->buffer_offset = 0;
    s->linked_buffers = 0;
    if(alIsSource(s->internal->source))
    {
        alSourceStop(s->internal->source);
        alDeleteSources(1, &s->internal->source);
        s->internal->source = 0;
    }
    alDeleteBuffers(TR_AUDIO_STREAM_NUMBUFFERS, s->internal->buffers);
    free(s->internal);
    s->internal = NULL;
}


void StreamTrack_SetEffects(stream_track_p s, int value)
{
    if(value)
    {
        Audio_SetFX(s->internal->source);
    }
    else
    {
        Audio_UnsetFX(s->internal->source);
    }
}


int StreamTrack_IsNeedUpdateBuffer(stream_track_p s)
{
    if(alIsSource(s->internal->source))
    {
        if(s->linked_buffers >= TR_AUDIO_STREAM_NUMBUFFERS)
        {
            ALint processed = 0;
            alGetSourcei(s->internal->source, AL_BUFFERS_PROCESSED, &processed);
            return processed > 0;
        }
        return 1;
    }
    return 0;
}


int StreamTrack_UpdateBuffer(stream_track_p s, uint8_t *buff, size_t size, int sample_bitsize, int channels, int frequency)
{
    if(alIsSource(s->internal->source))
    {
        if(s->linked_buffers >= TR_AUDIO_STREAM_NUMBUFFERS)
        {
            ALint processed = 0;
            // Check if any track buffers were already processed.
            // by doc: "Buffer queuing loop must operate in a new thread"
            alGetSourcei(s->internal->source, AL_BUFFERS_PROCESSED, &processed);
            if(processed > 0)
            {
                ALuint buffer_index = 0;
                alSourceUnqueueBuffers(s->internal->source, 1, &buffer_index);
                if(Audio_FillALBuffer(buffer_index, buff, size, sample_bitsize, channels, frequency))
                {
                    s->buffer_offset += size;
                    alSourceQueueBuffers(s->internal->source, 1, &buffer_index);
                    return 1;
                }
                return -1;
            }
            return 0;
        }
        else
        {
            ALuint buffer_index = s->internal->buffers[s->linked_buffers];
            ++s->linked_buffers;
            if(Audio_FillALBuffer(buffer_index, buff, size, sample_bitsize, channels, frequency))
            {
                alSourceQueueBuffers(s->internal->source, 1, &buffer_index);
                s->buffer_offset += size;
                return 1;
            }
            return -1;
        }
    }
    return -1;
}


int StreamTrack_Play(stream_track_p s)
{
    if(alIsSource(s->internal->source))
    {
        ALint state = 0;
        alGetSourcei(s->internal->source, AL_SOURCE_STATE, &state);
        if(state != AL_PLAYING)
        {
            s->state = TR_AUDIO_STREAM_PLAYING;
            alSourcePlay(s->internal->source);
        }
        alSourcef(s->internal->source, AL_GAIN, s->current_volume);
        return 1;
    }
    return -1;
}


int StreamTrack_Stop(stream_track_p s)
{
    if(alIsSource(s->internal->source))
    {
        ALint queued = 0;
        ALint state = AL_STOPPED;  // AL_STOPPED, AL_INITIAL, AL_PLAYING, AL_PAUSED
        alGetSourcei(s->internal->source, AL_SOURCE_STATE, &state);
        if(state != AL_STOPPED)
        {
            alSourceStop(s->internal->source);
        }

        alGetSourcei(s->internal->source, AL_BUFFERS_QUEUED, &queued);
        while(0 < queued--)
        {
            ALuint buffer;
            alSourceUnqueueBuffers(s->internal->source, 1, &buffer);
        }
        s->linked_buffers = 0;
        s->buffer_offset = 0;
        s->state = TR_AUDIO_STREAM_STOPPED;
        return 1;
    }
    return -1;
}


int StreamTrack_Pause(stream_track_p s)
{
    if(alIsSource(s->internal->source) && (s->state == TR_AUDIO_STREAM_PLAYING))
    {
        alSourcePause(s->internal->source);
        s->state = TR_AUDIO_STREAM_PAUSED;
    }
}


int StreamTrack_CheckForEnd(stream_track_p s)
{
    if(alIsSource(s->internal->source) && (s->state != TR_AUDIO_STREAM_STOPPED))
    {
        ALint processed = 0;
        ALint state = AL_STOPPED;  // AL_STOPPED, AL_INITIAL, AL_PLAYING, AL_PAUSED
        alGetSourcei(s->internal->source, AL_SOURCE_STATE, &state);
        if((state == AL_STOPPED) || ((state == AL_PAUSED) && (s->state != TR_AUDIO_STREAM_PAUSED)))
        {
            return 1;
        }

        alGetSourcei(s->internal->source, AL_BUFFERS_PROCESSED, &processed);
        return (processed >= s->linked_buffers) ? (1) : (0);
    }
    return 0;
}


int StreamTrack_UpdateState(stream_track_p s, float time, float volume)
{
    if((s->state != TR_AUDIO_STREAM_STOPPED) && alIsSource(s->internal->source))
    {
        ALint state = 0;
        ALfloat inc = 0.0f;
        alGetSourcei(s->internal->source, AL_SOURCE_STATE, &state);

        if(StreamTrack_CheckForEnd(s))
        {
            StreamTrack_Stop(s);
            return 0;
        }

        switch(s->type)
        {
            case TR_AUDIO_STREAM_TYPE_BACKGROUND:
                inc = time * TR_AUDIO_STREAM_CROSSFADE_BACKGROUND;
                break;

            case TR_AUDIO_STREAM_TYPE_ONESHOT:
                inc = time * TR_AUDIO_STREAM_CROSSFADE_ONESHOT;
                break;

            case TR_AUDIO_STREAM_TYPE_CHAT:
                inc = time * TR_AUDIO_STREAM_CROSSFADE_CHAT;
                break;
        }

        if(s->state == TR_AUDIO_STREAM_STOPPING)
        {
            s->current_volume -= inc;
            if(s->current_volume <= 0.0f)
            {
                s->current_volume = 0.0f;
                StreamTrack_Stop(s);
                return 0;
            }
            else
            {
                alSourcef(s->internal->source, AL_GAIN, s->current_volume);
            }
        }
        else if((s->state == TR_AUDIO_STREAM_PLAYING) && (s->current_volume < volume))
        {
            s->current_volume += inc;
            if(s->current_volume > volume)
            {
                s->current_volume = volume;
            }
            alSourcef(s->internal->source, AL_GAIN, s->current_volume);
        }
        
        return 1;
    }
    
    return 0;
}