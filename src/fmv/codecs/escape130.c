/*
 * Escape 130 video decoder
 * Copyright (C) 2008 Eli Friedman (eli.friedman <at> gmail.com)
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define BITSTREAM_READER_LE
#include <inttypes.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "../get_bits.h"
#include "../tiny_codec.h"

typedef struct Escape130Context
{
    uint8_t *old_y_avg;

    uint8_t *new_y, *old_y;
    uint8_t *new_u, *old_u;
    uint8_t *new_v, *old_v;

    uint8_t *buf1, *buf2;
    int     linesize[3];
} Escape130Context;

static const uint8_t offset_table[] = { 2, 4, 10, 20 };
static const int8_t sign_table[64][4] = {
    {  0,  0,  0,  0 },
    { -1,  1,  0,  0 },
    {  1, -1,  0,  0 },
    { -1,  0,  1,  0 },
    { -1,  1,  1,  0 },
    {  0, -1,  1,  0 },
    {  1, -1,  1,  0 },
    { -1, -1,  1,  0 },
    {  1,  0, -1,  0 },
    {  0,  1, -1,  0 },
    {  1,  1, -1,  0 },
    { -1,  1, -1,  0 },
    {  1, -1, -1,  0 },
    { -1,  0,  0,  1 },
    { -1,  1,  0,  1 },
    {  0, -1,  0,  1 },

    {  0,  0,  0,  0 },
    {  1, -1,  0,  1 },
    { -1, -1,  0,  1 },
    { -1,  0,  1,  1 },
    { -1,  1,  1,  1 },
    {  0, -1,  1,  1 },
    {  1, -1,  1,  1 },
    { -1, -1,  1,  1 },
    {  0,  0, -1,  1 },
    {  1,  0, -1,  1 },
    { -1,  0, -1,  1 },
    {  0,  1, -1,  1 },
    {  1,  1, -1,  1 },
    { -1,  1, -1,  1 },
    {  0, -1, -1,  1 },
    {  1, -1, -1,  1 },

    {  0,  0,  0,  0 },
    { -1, -1, -1,  1 },
    {  1,  0,  0, -1 },
    {  0,  1,  0, -1 },
    {  1,  1,  0, -1 },
    { -1,  1,  0, -1 },
    {  1, -1,  0, -1 },
    {  0,  0,  1, -1 },
    {  1,  0,  1, -1 },
    { -1,  0,  1, -1 },
    {  0,  1,  1, -1 },
    {  1,  1,  1, -1 },
    { -1,  1,  1, -1 },
    {  0, -1,  1, -1 },
    {  1, -1,  1, -1 },
    { -1, -1,  1, -1 },

    {  0,  0,  0,  0 },
    {  1,  0, -1, -1 },
    {  0,  1, -1, -1 },
    {  1,  1, -1, -1 },
    { -1,  1, -1, -1 },
    {  1, -1, -1, -1 }
};

static const int8_t luma_adjust[] = { -4, -3, -2, -1, 1, 2, 3, 4 };

static const int8_t chroma_adjust[2][8] = {
    { 1, 1, 0, -1, -1, -1,  0,  1 },
    { 0, 1, 1,  1,  0, -1, -1, -1 }
};

static const uint8_t chroma_vals[] = {
     20,  28,  36,  44,  52,  60,  68,  76,
     84,  92, 100, 106, 112, 116, 120, 124,
    128, 132, 136, 140, 144, 150, 156, 164,
    172, 180, 188, 196, 204, 212, 220, 228
};

static int decode_skip_count(GetBitContext* gb)
{
    int value;

    if (get_bits_left(gb) < 1+3)
        return -1;

    value = get_bits1(gb);
    if (value)
        return 0;

    value = get_bits(gb, 3);
    if (value)
        return value;

    value = get_bits(gb, 8);
    if (value)
        return value + 7;

    value = get_bits(gb, 15);
    if (value)
        return value + 262;

    return -1;
}

static int escape130_decode_frame(struct tiny_codec_s *avctx, struct AVPacket *avpkt)
{
    int buf_size        = avpkt->size;
    Escape130Context *s = (Escape130Context*)avctx->video.priv_data;
    GetBitContext gb;
    int ret;

    uint8_t *old_y, *old_cb, *old_cr,
            *new_y, *new_cb, *new_cr;
    unsigned old_y_stride, old_cb_stride, old_cr_stride,
             new_y_stride, new_cb_stride, new_cr_stride;
    unsigned total_blocks = avctx->video.width * avctx->video.height / 4,
             block_index, block_x = 0;
    unsigned y[4] = { 0 }, cb = 0x10, cr = 0x10;
    int skip = -1, y_avg = 0, i, j;
    uint8_t *ya = s->old_y_avg;

    // first 16 bytes are header; no useful information in here
    if(buf_size <= 16)
    {
        //av_log(avctx, AV_LOG_ERROR, "Insufficient frame data\n");
        return -1;
    }

    //if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
    //    return ret;

    if ((ret = init_get_bits8(&gb, avpkt->data, avpkt->size)) < 0)
        return ret;
    skip_bits_long(&gb, 16 * 8);

    new_y  = s->new_y;
    new_cb = s->new_u;
    new_cr = s->new_v;
    new_y_stride  = s->linesize[0];
    new_cb_stride = s->linesize[1];
    new_cr_stride = s->linesize[2];
    old_y  = s->old_y;
    old_cb = s->old_u;
    old_cr = s->old_v;
    old_y_stride  = s->linesize[0];
    old_cb_stride = s->linesize[1];
    old_cr_stride = s->linesize[2];

    for(block_index = 0; block_index < total_blocks; block_index++)
    {
        // Note that this call will make us skip the rest of the blocks
        // if the frame ends prematurely.
        if (skip == -1)
            skip = decode_skip_count(&gb);
        if (skip == -1)
        {
            //av_log(avctx, AV_LOG_ERROR, "Error decoding skip value\n");
            return -1;
        }

        if (skip)
        {
            y[0] = old_y[0];
            y[1] = old_y[1];
            y[2] = old_y[old_y_stride];
            y[3] = old_y[old_y_stride + 1];
            y_avg = ya[0];
            cb = old_cb[0];
            cr = old_cr[0];
        }
        else
        {
            if (get_bits1(&gb))
            {
                unsigned sign_selector       = get_bits(&gb, 6);
                unsigned difference_selector = get_bits(&gb, 2);
                y_avg = 2 * get_bits(&gb, 5);
                for (i = 0; i < 4; i++)
                {
                    y[i] = av_clip_c(y_avg + offset_table[difference_selector] *
                                     sign_table[sign_selector][i], 0, 63);
                }
            }
            else if(get_bits1(&gb))
            {
                if(get_bits1(&gb))
                {
                    y_avg = get_bits(&gb, 6);
                }
                else
                {
                    unsigned adjust_index = get_bits(&gb, 3);
                    y_avg = (y_avg + luma_adjust[adjust_index]) & 63;
                }
                for (i = 0; i < 4; i++)
                    y[i] = y_avg;
            }

            if(get_bits1(&gb))
            {
                if(get_bits1(&gb))
                {
                    cb = get_bits(&gb, 5);
                    cr = get_bits(&gb, 5);
                }
                else
                {
                    unsigned adjust_index = get_bits(&gb, 3);
                    cb = (cb + chroma_adjust[0][adjust_index]) & 31;
                    cr = (cr + chroma_adjust[1][adjust_index]) & 31;
                }
            }
        }
        *ya++ = y_avg;

        new_y[0]                = y[0];
        new_y[1]                = y[1];
        new_y[new_y_stride]     = y[2];
        new_y[new_y_stride + 1] = y[3];
        *new_cb = cb;
        *new_cr = cr;

        old_y += 2;
        old_cb++;
        old_cr++;
        new_y += 2;
        new_cb++;
        new_cr++;
        block_x++;
        if(block_x * 2 == avctx->video.width)
        {
            block_x = 0;
            old_y  += old_y_stride * 2  - avctx->video.width;
            old_cb += old_cb_stride     - avctx->video.width / 2;
            old_cr += old_cr_stride     - avctx->video.width / 2;
            new_y  += new_y_stride * 2  - avctx->video.width;
            new_cb += new_cb_stride     - avctx->video.width / 2;
            new_cr += new_cr_stride     - avctx->video.width / 2;
        }
        skip--;
    }

    if(avctx->video.rgba)
    {
        uint8_t *rgba = avctx->video.rgba;
        float y, u, v;
        int r, g, b;
        new_cb = s->new_u;
        new_cr = s->new_v;

        for(i = 0; i < avctx->video.height; ++i)
        {
            for(int j = 0; j < avctx->video.width; ++j)
            {
                uint8_t cb = new_cb[j / 2] & 31;
                uint8_t cr = new_cr[j / 2] & 31;
                y = (s->new_y[new_y_stride * i + j] << 2);
                u = chroma_vals[cb];
                v = chroma_vals[cr];
                r = y + 1.13983f * (v - 128);
                g = y - 0.39465f * (u - 128) - 0.58060f * (v - 128);
                b = y + 2.03211f * (u - 128);
                r = (r < 0) ? (0) : (r);
                g = (g < 0) ? (0) : (g);
                b = (b < 0) ? (0) : (b);
                *rgba++ = (r <= 0xFF) ? (r) : 0xFF;
                *rgba++ = (g <= 0xFF) ? (g) : 0xFF;
                *rgba++ = (b <= 0xFF) ? (b) : 0xFF;
                *rgba++ = 0xFF;
            }
            if(i & 1)
            {
                new_cb += new_cb_stride;
                new_cr += new_cr_stride;
            }
        }
    }
    //ff_dlog(avctx, "Frame data: provided %d bytes, used %d bytes\n",
    //        buf_size, get_bits_count(&gb) >> 3);

    FFSWAP(uint8_t*, s->old_y, s->new_y);
    FFSWAP(uint8_t*, s->old_u, s->new_u);
    FFSWAP(uint8_t*, s->old_v, s->new_v);

    return buf_size;
}

static void escape130_free_data(void *data)
{
    Escape130Context *s = (Escape130Context*)data;
    if(s)
    {
        free(s->old_y_avg);
        free(s->buf1);
        free(s->buf2);
        free(s);
    }
}

void escape130_decode_init(struct tiny_codec_s *avctx)
{
    if(!avctx->video.priv_data && !((avctx->video.width & 1) || (avctx->video.height & 1)))
    {
        Escape130Context *s = (Escape130Context*)malloc(sizeof(Escape130Context));
        avctx->video.decode = escape130_decode_frame;
        avctx->video.priv_data = s;
        avctx->video.free_data = escape130_free_data;

        //avctx->pix_fmt = AV_PIX_FMT_YUV420P;
        s->old_y_avg = malloc(avctx->video.width * avctx->video.height / 4);
        s->buf1      = malloc(avctx->video.width * avctx->video.height * 3 / 2);
        s->buf2      = malloc(avctx->video.width * avctx->video.height * 3 / 2);

        s->linesize[0] = avctx->video.width;
        s->linesize[1] = avctx->video.width / 2;
        s->linesize[2] = avctx->video.width / 2;

        s->new_y = s->buf1;
        s->new_u = s->new_y + avctx->video.width * avctx->video.height;
        s->new_v = s->new_u + avctx->video.width * avctx->video.height / 4;
        s->old_y = s->buf2;
        s->old_u = s->old_y + avctx->video.width * avctx->video.height;
        s->old_v = s->old_u + avctx->video.width * avctx->video.height / 4;
        memset(s->old_y, 0,    avctx->video.width * avctx->video.height);
        memset(s->old_u, 0x10, avctx->video.width * avctx->video.height / 4);
        memset(s->old_v, 0x10, avctx->video.width * avctx->video.height / 4);
    }
}
