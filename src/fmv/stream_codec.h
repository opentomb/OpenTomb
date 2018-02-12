/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   stream_codec.h
 * Author: nTesla64a
 *
 * Created on September 5, 2017, 5:02 PM
 */

#ifndef TINY_STREAM_H
#define TINY_STREAM_H

#include <stdint.h>
#include <pthread.h>

#include "tiny_codec.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_STATE_STOPPED     (0)
#define VIDEO_STATE_QEUED       (1)
#define VIDEO_STATE_RUNNING     (2)


typedef struct stream_codec_s
{
    struct tiny_codec_s      codec;
    int                      is_thread_run;
    pthread_t                thread;
    pthread_mutex_t          timer_mutex;
    pthread_mutex_t          video_buffer_mutex;
    pthread_mutex_t          audio_buffer_mutex;
    volatile int             stop;
    volatile int             update_audio;
    volatile int             state;
} stream_codec_t, *stream_codec_p;


void stream_codec_init(stream_codec_p s);
void stream_codec_clear(stream_codec_p s);
void stream_codec_stop(stream_codec_p s, int wait);
int  stream_codec_check_end(stream_codec_p s);

void stream_codec_video_lock(stream_codec_p s);
void stream_codec_video_unlock(stream_codec_p s);
void stream_codec_audio_lock(stream_codec_p s);
void stream_codec_audio_unlock(stream_codec_p s);

int stream_codec_play(stream_codec_p s);

#ifdef __cplusplus
}
#endif

#endif /* TINY_STREAM_H */

