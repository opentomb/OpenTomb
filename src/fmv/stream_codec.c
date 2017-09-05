
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <pthread.h>
#ifdef _TIMESPEC_DEFINED
#include <pthread_time.h>
#elif !defined CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#include "tiny_codec.h"
#include "stream_codec.h"

void stream_codec_init(stream_codec_p s)
{
    s->state = VIDEO_STATE_STOPPED;
    s->stop = 0;
    s->thread = 0;
    pthread_mutex_init(&s->timer_mutex, NULL);
    pthread_mutex_init(&s->buffer_mutex, NULL);
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
    pthread_mutex_destroy(&s->buffer_mutex);
}


int stream_codec_check_playing(stream_codec_p s)
{
    if(s->state == VIDEO_STATE_STOPPED)
    {
        if(s->thread)
        {
            pthread_join(s->thread, NULL);
            s->thread = 0;
        }
        return 0;
    }
    return 1;
}


void stream_codec_stop(stream_codec_p s, int wait)
{
    s->stop = 1;
    if(wait && s->thread)
    {
        pthread_join(s->thread, NULL);
        s->thread = 0;
    }
    codec_clear(&s->codec);
}


static void *stream_codec_thread_func(void *data)
{
    stream_codec_p s = (stream_codec_p)data;
    if(s)
    {
        AVPacket pkt;
        uint64_t frame = 0;
        uint64_t ns = 0;
        struct timespec time_start = { 0 };
        struct timespec vid_time;

        s->codec.video.rgba = (uint8_t*)malloc(4 * s->codec.video.width * s->codec.video.height);
        pkt.is_video = 1;
        pkt.data = NULL;
        pkt.size = 0;
        pkt.allocated_size = 0;
        clock_gettime(CLOCK_REALTIME, &time_start);

        while(!s->stop && (s->codec.packet(&s->codec, &pkt) != -1))
        {
            frame++;
            ns = (frame * s->codec.fps_denum) % s->codec.fps_num;
            ns = ns * 1000000000 / s->codec.fps_num;
            vid_time.tv_sec = time_start.tv_sec + frame * s->codec.fps_denum / s->codec.fps_num;
            vid_time.tv_nsec = time_start.tv_nsec + ns;
            if(vid_time.tv_nsec >= 1000000000)
            {
                vid_time.tv_nsec -= 1000000000;
                vid_time.tv_sec++;
            }
            pthread_mutex_lock(&s->buffer_mutex);
            s->codec.video.decode(&s->codec, &pkt);
            pthread_mutex_unlock(&s->buffer_mutex);
            s->state = VIDEO_STATE_RUNNING;
            pthread_mutex_timedlock(&s->timer_mutex, &vid_time);
        }
        av_packet_unref(&pkt);
        s->state = VIDEO_STATE_QEUED;
        pthread_mutex_lock(&s->buffer_mutex);
        free(s->codec.video.rgba);
        s->codec.video.rgba = NULL;
        pthread_mutex_unlock(&s->buffer_mutex);

        codec_clear(&s->codec);
        SDL_RWclose(s->codec.pb);
        s->codec.pb = NULL;
    }
    s->state = VIDEO_STATE_STOPPED;

    return NULL;
}


uint8_t *stream_codec_get_rgba_lock(stream_codec_p s)
{
    pthread_mutex_lock(&s->buffer_mutex);
    return (s->state == VIDEO_STATE_RUNNING) ? (s->codec.video.rgba) : (NULL);
}


void stream_codec_unlock_get(stream_codec_p s)
{
    pthread_mutex_unlock(&s->buffer_mutex);
}


int stream_codec_play_rpl(stream_codec_p s, const char *name)
{
    if((0 == s->thread) && (s->state == VIDEO_STATE_STOPPED))
    {
        codec_init(&s->codec, SDL_RWFromFile(name, "rb"));
        if(s->codec.pb)
        {
            if(0 == codec_open_rpl(&s->codec))
            {
                s->state = VIDEO_STATE_QEUED;
                s->stop = 0;
                return pthread_create(&s->thread, NULL, stream_codec_thread_func, s);
            }
            else
            {
                SDL_RWclose(s->codec.pb);
                s->codec.pb = NULL;
            }
        }
    }
    return -1;
}
