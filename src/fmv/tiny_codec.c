/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "tiny_codec.h"

void av_init_packet(AVPacket *pkt)
{
    pkt->pos = 0;
    pkt->data = NULL;
    pkt->size = 0;
    pkt->allocated_size = 0;
    pkt->flags = 0;
    pkt->stream_index = 0;
}

int av_get_packet(SDL_RWops *pb, AVPacket *pkt, int size)
{
    int ret = 0;
    if(pkt->allocated_size < size)
    {
        pkt->allocated_size = size + 1024 - size % 1024;
        if(pkt->data)
        {
            free(pkt->data);
        }
        pkt->data = (uint8_t*)malloc(pkt->allocated_size);
    }
    pkt->size = size;
    pkt->dts = 0;
    pkt->pts = 0;
    pkt->duration = 0;
    pkt->flags = 0;
    ret = SDL_RWread(pb, pkt->data, 1, size);
    pkt->pos = SDL_RWtell(pb);
    return ret;
}

void av_packet_unref(AVPacket *pkt)
{
    if(pkt->data)
    {
        free(pkt->data);
    }
    pkt->data = NULL;
    pkt->allocated_size = 0;
    pkt->size = 0;
}

void codec_init(struct tiny_codec_s *s, SDL_RWops *rw)
{
    s->pb = rw;
    s->private_context = NULL;
    s->free_context = NULL;
    s->fps_num = 24;
    s->fps_denum = 1;

    s->audio.buff_allocated_size = 0;
    s->audio.buff_size = 0;
    s->audio.buff_offset = 0;
    s->audio.buff = NULL;
    s->audio.entry = NULL;
    s->audio.entry_size = 0;
    s->audio.entry_current = 0;
    s->audio.priv_data = NULL;
    s->audio.free_data = NULL;
    s->audio.decode = NULL;
    s->audio.codec_tag = 0;

    s->video.rgba = NULL;
    s->video.entry = NULL;
    s->video.entry_size = 0;
    s->video.entry_current = 0;
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
        s->free_context = NULL;
        s->private_context = NULL;
    }
    s->packet = NULL;
    s->video.decode = NULL;
    s->video.codec_tag = 0;
    s->audio.decode = NULL;
    s->audio.codec_tag = 0;
    s->audio.buff_allocated_size = 0;
    s->audio.buff_size = 0;
    s->audio.buff_offset = 0;

    if(s->video.entry)
    {
        free(s->video.entry);
        s->video.entry = NULL;
        s->video.entry_size = 0;
        s->video.entry_current = 0;
    }
    if(s->video.free_data)
    {
        s->video.free_data(s->video.priv_data);
        s->video.priv_data = NULL;
        s->video.free_data = NULL;
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
        s->audio.entry_size = 0;
        s->audio.entry_current = 0;
    }
    if(s->audio.free_data)
    {
        s->audio.free_data(s->audio.priv_data);
        s->audio.priv_data = NULL;
        s->audio.free_data = NULL;
    }
}
