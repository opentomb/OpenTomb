
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#ifndef _POSIX_SOURCE
#define __USE_POSIX199309  (1)   // make posix GCC workable
#define __USE_XOPEN2K
#endif
#include <pthread.h>
#ifdef _TIMESPEC_DEFINED         // make MinGW workable
#include <pthread_time.h>
#endif

#include "tiny_codec.h"
#include "stream_codec.h"

void stream_codec_init(stream_codec_p s)
{
    s->state = VIDEO_STATE_STOPPED;
    s->stop = 0;
    s->update_audio = 1;
    s->thread = 0;
    pthread_mutex_init(&s->timer_mutex, NULL);
    pthread_mutex_init(&s->video_buffer_mutex, NULL);
    pthread_mutex_init(&s->audio_buffer_mutex, NULL);
    codec_init(&s->codec, NULL);
}


void stream_codec_clear(stream_codec_p s)
{
    s->stop = 1;
    if(s->thread)
    {
        pthread_join(s->thread, NULL);
        s->thread = 0;
    }

    pthread_mutex_destroy(&s->timer_mutex);
    pthread_mutex_destroy(&s->video_buffer_mutex);
    pthread_mutex_destroy(&s->audio_buffer_mutex);
}


int stream_codec_check_end(stream_codec_p s)
{
    if(s->state == VIDEO_STATE_STOPPED)
    {
        if(s->thread)
        {
            pthread_join(s->thread, NULL);
            s->thread = 0;
            return 1;
        }
        return 0;
    }
    return -1;
}


void stream_codec_stop(stream_codec_p s, int wait)
{
    s->stop = 1;
    if(wait && s->thread)
    {
        pthread_join(s->thread, NULL);
        s->thread = 0;
    }
}


static void *stream_codec_thread_func(void *data)
{
    stream_codec_p s = (stream_codec_p)data;
    if(s)
    {
        uint64_t frame = 0;
        uint64_t ns = 0;
        struct timespec time_start = { 0 };
        struct timespec vid_time;
        int can_continue = 1;

        clock_gettime(CLOCK_REALTIME, &time_start);

        while(!s->stop && can_continue)
        {
            frame++;
            can_continue = 0;
            ns = (frame * s->codec.fps_denum) % s->codec.fps_num;
            ns = ns * 1000000000 / s->codec.fps_num;
            vid_time.tv_sec = time_start.tv_sec + frame * s->codec.fps_denum / s->codec.fps_num;
            vid_time.tv_nsec = time_start.tv_nsec + ns;
            if(vid_time.tv_nsec >= 1000000000)
            {
                vid_time.tv_nsec -= 1000000000;
                vid_time.tv_sec++;
            }

            if(s->update_audio && s->codec.audio.decode && (s->codec.packet(&s->codec, &s->codec.audio.pkt) >= 0))
            {
                pthread_mutex_lock(&s->audio_buffer_mutex);
                s->codec.audio.decode(&s->codec, &s->codec.audio.pkt);
                s->update_audio = 0;
                pthread_mutex_unlock(&s->audio_buffer_mutex);
            }

            if(s->codec.video.decode && (s->codec.packet(&s->codec, &s->codec.video.pkt) >= 0))
            {
                pthread_mutex_lock(&s->video_buffer_mutex);
                s->codec.video.decode(&s->codec, &s->codec.video.pkt);
                pthread_mutex_unlock(&s->video_buffer_mutex);
                can_continue++;
            }

            s->state = VIDEO_STATE_RUNNING;
            pthread_mutex_timedlock(&s->timer_mutex, &vid_time);
        }
        s->state = VIDEO_STATE_QEUED;

        pthread_mutex_lock(&s->video_buffer_mutex);
        pthread_mutex_lock(&s->audio_buffer_mutex);
        codec_clear(&s->codec);
        pthread_mutex_unlock(&s->audio_buffer_mutex);
        pthread_mutex_unlock(&s->video_buffer_mutex);
        
        SDL_RWclose(s->codec.input);
        s->codec.input = NULL;
    }
    s->state = VIDEO_STATE_STOPPED;

    return NULL;
}


void stream_codec_video_lock(stream_codec_p s)
{
    pthread_mutex_lock(&s->video_buffer_mutex);
}


void stream_codec_video_unlock(stream_codec_p s)
{
    pthread_mutex_unlock(&s->video_buffer_mutex);
}


void stream_codec_audio_lock(stream_codec_p s)
{
    pthread_mutex_lock(&s->audio_buffer_mutex);
}


void stream_codec_audio_unlock(stream_codec_p s)
{
    pthread_mutex_unlock(&s->audio_buffer_mutex);
}


int stream_codec_play(stream_codec_p s)
{
    if((s->state == VIDEO_STATE_STOPPED) && s->codec.input)
    {
        s->state = VIDEO_STATE_QEUED;
        s->stop = 0;
        return 0 != pthread_create(&s->thread, NULL, stream_codec_thread_func, s);
    }
    return 0;
}
