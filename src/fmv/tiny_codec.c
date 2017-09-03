/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "tiny_codec.h"

int av_get_packet(SDL_RWops *pb, AVPacket *pkt, int size)
{
    pkt->data = (uint8_t*)malloc(size);
    pkt->size = size;
    pkt->dts = 0;
    pkt->pts = 0;
    pkt->duration = 0;
    pkt->pos = 0;
    pkt->flags = 0;
    return SDL_RWread(pb, pkt->data, 1, size);
}

void av_packet_unref(AVPacket *pkt)
{
    if(pkt->data)
    {
        free(pkt->data);
    }
    pkt->data = NULL;
    pkt->size = 0;
}

void codec_init(struct tiny_codec_s *s, SDL_RWops *rw)
{
    s->pb = rw;
    s->audio.free_data = NULL;
    s->audio.priv_data = NULL;

    s->audio.buff = NULL;
    s->audio.entry = NULL;
    s->audio.priv_data = NULL;
    s->audio.free_data = NULL;
    s->audio.decode = NULL;
    s->audio.codec_tag = 0;

    s->video.buff = NULL;
    s->video.rgba = NULL;
    s->video.entry = NULL;
    s->video.priv_data = NULL;
    s->video.free_data = NULL;
    s->video.decode = NULL;
    s->video.codec_tag = 0;
}

void codec_clear(struct tiny_codec_s *s)
{
    if(s->free_context)
    {
        s->free_context(s->private_context);
        s->private_context = NULL;
    }
    s->packet = NULL;
    s->video.decode = NULL;
    s->video.codec_tag = 0;
    s->audio.decode = NULL;
    s->audio.codec_tag = 0;

    if(s->video.buff)
    {
        free(s->video.buff);
        s->video.buff = NULL;
    }
    if(s->video.rgba)
    {
        free(s->video.rgba);
        s->video.rgba = NULL;
    }
    if(s->video.entry)
    {
        free(s->video.entry);
        s->video.entry = NULL;
    }
    if(s->video.free_data)
    {
        s->video.free_data(s->video.priv_data);
        s->video.priv_data = NULL;
    }

    if(s->audio.buff)
    {
        free(s->audio.buff);
        s->audio.buff = NULL;
    }
    if(s->audio.entry)
    {
        free(s->audio.entry);
        s->audio.entry = NULL;
    }
    if(s->audio.free_data)
    {
        s->audio.free_data(s->audio.priv_data);
        s->audio.priv_data = NULL;
    }
}