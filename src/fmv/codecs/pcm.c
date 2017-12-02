/*
 * PCM codecs
 * Copyright (c) 2001 Fabrice Bellard
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

/**
 * @file
 * PCM codecs
 */
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "avcodec.h"
#include "../tiny_codec.h"
#define BITSTREAM_READER_LE
#include "../internal/get_bits.h"
#include "../internal/bytestream.h"

typedef struct PCMDecode
{
    short   table[256];
    float   scale;
} PCMDecode;

/**
 * Read PCM samples macro
 * @param size   Data size of native machine format
 * @param endian bytestream_get_xxx() endian suffix
 * @param src    Source pointer (variable name)
 * @param dst    Destination pointer (variable name)
 * @param n      Total number of samples (variable name)
 * @param shift  Bitshift (bits)
 * @param offset Sample value offset
 */
#define DECODE(size, endian, src, dst, n, shift, offset)                \
    for (; n > 0; n--) {                                                \
        uint ## size ## _t v = bytestream_get_ ## endian(&src);         \
        AV_WN ## size ## A(dst, (v - offset) << shift);                 \
        dst += size / 8;                                                \
    }

#define DECODE_PLANAR(size, endian, src, dst, n, shift, offset)         \
    n /= avctx->audio.channels;                                         \
    for (c = 0; c < avctx->audio.channels; c++) {                       \
        int i;                                                          \
        dst = avctx->audio.buff_p[c];                                   \
        for (i = n; i > 0; i--) {                                       \
            uint ## size ## _t v = bytestream_get_ ## endian(&src);     \
            AV_WN ## size ## A(dst, (v - offset) << shift);             \
            dst += size / 8;                                            \
        }                                                               \
    }

static int32_t pcm_decode_frame(struct tiny_codec_s *avctx, struct AVPacket *avpkt)
{
    const uint8_t *src = avpkt->data;
    int buf_size       = avpkt->size;
    PCMDecode *s       = (PCMDecode*)avctx->audio.priv_data;
    int sample_size, c, n, samples_per_block;
    uint32_t output_buff_size = 0;
    uint8_t *samples;
    int32_t *dst_int32_t;

    sample_size = avctx->audio.bits_per_sample >> 3;

    /* av_get_bits_per_sample returns 0 for AV_CODEC_ID_PCM_DVD */
    samples_per_block = 1;
    if (avctx->audio.codec_tag == AV_CODEC_ID_PCM_LXF)
    {
        /* we process 40-bit blocks per channel for LXF */
        samples_per_block = 2;
        sample_size       = 5;
    }

    if (sample_size == 0)
    {
        //av_log(avctx, AV_LOG_ERROR, "Invalid sample_size\n");
        return -1;
    }

    if (avctx->audio.channels == 0)
    {
        //av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return -1;
    }

    n = avctx->audio.channels * sample_size;

    if (n && buf_size % n)
    {
        if (buf_size < n)
        {
            return -1;
        }
        else
            buf_size -= buf_size % n;
    }

    n = buf_size / sample_size;

    /* get output buffer */
    output_buff_size = codec_resize_audio_buffer(avctx, sample_size, n * samples_per_block / avctx->audio.channels);
    samples = (uint8_t*)avctx->audio.buff;

    switch (avctx->audio.codec_tag)
    {
    case AV_CODEC_ID_PCM_U32LE:
        DECODE(32, le32, src, samples, n, 0, 0x80000000)
        break;
    case AV_CODEC_ID_PCM_U32BE:
        DECODE(32, be32, src, samples, n, 0, 0x80000000)
        break;
    case AV_CODEC_ID_PCM_S24LE:
        DECODE(32, le24, src, samples, n, 8, 0)
        break;
    case AV_CODEC_ID_PCM_S24LE_PLANAR:
        DECODE_PLANAR(32, le24, src, samples, n, 8, 0);
        break;
    case AV_CODEC_ID_PCM_S24BE:
        DECODE(32, be24, src, samples, n, 8, 0)
        break;
    case AV_CODEC_ID_PCM_U24LE:
        DECODE(32, le24, src, samples, n, 8, 0x800000)
        break;
    case AV_CODEC_ID_PCM_U24BE:
        DECODE(32, be24, src, samples, n, 8, 0x800000)
        break;
    case AV_CODEC_ID_PCM_S24DAUD:
        for (; n > 0; n--)
        {
            uint32_t v = bytestream_get_be24(&src);
            v >>= 4; // sync flags are here
            AV_WN16A(samples, ff_reverse[(v >> 8) & 0xff] +
                             (ff_reverse[v        & 0xff] << 8));
            samples += 2;
        }
        break;
    case AV_CODEC_ID_PCM_U16LE:
        DECODE(16, le16, src, samples, n, 0, 0x8000)
        break;
    case AV_CODEC_ID_PCM_U16BE:
        DECODE(16, be16, src, samples, n, 0, 0x8000)
        break;
    case AV_CODEC_ID_PCM_S8:
        for (; n > 0; n--)
            *samples++ = *src++ + 128;
        break;
    case AV_CODEC_ID_PCM_S8_PLANAR:
        n /= avctx->audio.channels;
        for (c = 0; c < avctx->audio.channels; c++)
        {
            int i;
            samples = avctx->audio.buff_p[c];
            for (i = n; i > 0; i--)
                *samples++ = *src++ + 128;
        }
        break;
#if HAVE_BIGENDIAN
    case AV_CODEC_ID_PCM_S64LE:
    case AV_CODEC_ID_PCM_F64LE:
        DECODE(64, le64, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_F32LE:
    case AV_CODEC_ID_PCM_F24LE:
    case AV_CODEC_ID_PCM_F16LE:
        DECODE(32, le32, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_S32LE_PLANAR:
        DECODE_PLANAR(32, le32, src, samples, n, 0, 0);
        break;
    case AV_CODEC_ID_PCM_S16LE:
        DECODE(16, le16, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
        DECODE_PLANAR(16, le16, src, samples, n, 0, 0);
        break;
    case AV_CODEC_ID_PCM_F64BE:
    case AV_CODEC_ID_PCM_F32BE:
    case AV_CODEC_ID_PCM_S64BE:
    case AV_CODEC_ID_PCM_S32BE:
    case AV_CODEC_ID_PCM_S16BE:
#else
    case AV_CODEC_ID_PCM_S64BE:
    case AV_CODEC_ID_PCM_F64BE:
        DECODE(64, be64, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_F32BE:
    case AV_CODEC_ID_PCM_S32BE:
        DECODE(32, be32, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_S16BE:
        DECODE(16, be16, src, samples, n, 0, 0)
        break;
    case AV_CODEC_ID_PCM_S16BE_PLANAR:
        DECODE_PLANAR(16, be16, src, samples, n, 0, 0);
        break;
    case AV_CODEC_ID_PCM_F64LE:
    case AV_CODEC_ID_PCM_F32LE:
    case AV_CODEC_ID_PCM_F24LE:
    case AV_CODEC_ID_PCM_F16LE:
    case AV_CODEC_ID_PCM_S64LE:
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_S16LE:
#endif /* HAVE_BIGENDIAN */
    case AV_CODEC_ID_PCM_U8:
        memcpy(samples, src, n * sample_size);
        break;
#if HAVE_BIGENDIAN
    case AV_CODEC_ID_PCM_S16BE_PLANAR:
#else
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
    case AV_CODEC_ID_PCM_S32LE_PLANAR:
#endif /* HAVE_BIGENDIAN */
        n /= avctx->audio.channels;
        for (c = 0; c < avctx->audio.channels; c++)
        {
            samples = avctx->audio.buff_p[c];
            bytestream_get_buffer(&src, samples, n * sample_size);
        }
        break;
    case AV_CODEC_ID_PCM_ZORK:
        for (; n > 0; n--)
        {
            int v = *src++;
            if (v < 128)
                v = 128 - v;
            *samples++ = v;
        }
        break;
    case AV_CODEC_ID_PCM_ALAW:
    case AV_CODEC_ID_PCM_MULAW:
        for (; n > 0; n--)
        {
            AV_WN16A(samples, s->table[*src++]);
            samples += 2;
        }
        break;
    case AV_CODEC_ID_PCM_LXF:
    {
        int i;
        n /= avctx->audio.channels;
        for (c = 0; c < avctx->audio.channels; c++)
        {
            dst_int32_t = (int32_t *)avctx->audio.buff_p[c];
            for (i = 0; i < n; i++)
            {
                // extract low 20 bits and expand to 32 bits
                *dst_int32_t++ =  (src[2]         << 28) |
                                  (src[1]         << 20) |
                                  (src[0]         << 12) |
                                 ((src[2] & 0x0F) <<  8) |
                                   src[1];
                // extract high 20 bits and expand to 32 bits
                *dst_int32_t++ =  (src[4]         << 24) |
                                  (src[3]         << 16) |
                                 ((src[2] & 0xF0) <<  8) |
                                  (src[4]         <<  4) |
                                  (src[3]         >>  4);
                src += 5;
            }
        }
        break;
    }
    default:
        return -1;
    }

    if (avctx->audio.codec_tag == AV_CODEC_ID_PCM_F16LE ||
        avctx->audio.codec_tag == AV_CODEC_ID_PCM_F24LE)
    {
        /*s->fdsp->vector_fmul_scalar((float *)frame->extended_data[0],
                                    (const float *)frame->extended_data[0],
                                    s->scale, FFALIGN(frame->nb_samples * avctx->channels, 4));
        emms_c();*/
    }

    avctx->audio.buff_offset += avctx->audio.buff_size;
    avctx->audio.buff_size = output_buff_size;

    return buf_size;
}


/* from g711.c by SUN microsystems (unrestricted use) */

#define         SIGN_BIT        (0x80)      /* Sign bit for a A-law byte. */
#define         QUANT_MASK      (0xf)       /* Quantization field mask. */
#define         NSEGS           (8)         /* Number of A-law segments. */
#define         SEG_SHIFT       (4)         /* Left shift for segment number. */
#define         SEG_MASK        (0x70)      /* Segment field mask. */

#define         BIAS            (0x84)      /* Bias for linear code. */

/* alaw2linear() - Convert an A-law value to 16-bit linear PCM */
static int alaw2linear(unsigned char a_val)
{
        int t;
        int seg;

        a_val ^= 0x55;

        t = a_val & QUANT_MASK;
        seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
        if(seg) t= (t + t + 1 + 32) << (seg + 2);
        else    t= (t + t + 1     ) << 3;

        return (a_val & SIGN_BIT) ? t : -t;
}

static int ulaw2linear(unsigned char u_val)
{
        int t;

        /* Complement to obtain normal u-law value. */
        u_val = ~u_val;

        /*
         * Extract and bias the quantization bits. Then
         * shift up by the segment number and subtract out the bias.
         */
        t = ((u_val & QUANT_MASK) << 3) + BIAS;
        t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

        return (u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS);
}


static void pcm_free_data(void *data)
{
    PCMDecode *s = (PCMDecode*)data;
    if(s)
    {
        free(s);
    }
}


void pcm_decode_init(struct tiny_codec_s *avctx)
{
    if ((avctx->audio.channels > 0) && !avctx->audio.priv_data)
    {
        int i;
        PCMDecode *s = (PCMDecode*)malloc(sizeof(PCMDecode));
        avctx->audio.priv_data = s;
        avctx->audio.free_data = pcm_free_data;
        avctx->audio.decode = pcm_decode_frame;

        switch(avctx->audio.codec_tag)
        {
            case AV_CODEC_ID_DSD_LSBF:
            case AV_CODEC_ID_DSD_MSBF:
            case AV_CODEC_ID_DSD_LSBF_PLANAR:
            case AV_CODEC_ID_DSD_MSBF_PLANAR:
            case AV_CODEC_ID_PCM_ALAW:
            case AV_CODEC_ID_PCM_MULAW:
            case AV_CODEC_ID_PCM_S8:
            case AV_CODEC_ID_PCM_S8_PLANAR:
            case AV_CODEC_ID_PCM_U8:
            case AV_CODEC_ID_PCM_ZORK:
            case AV_CODEC_ID_SDX2_DPCM:
                avctx->audio.bits_per_sample =  8;
                break;
            case AV_CODEC_ID_PCM_S16BE:
            case AV_CODEC_ID_PCM_S16BE_PLANAR:
            case AV_CODEC_ID_PCM_S16LE:
            case AV_CODEC_ID_PCM_S16LE_PLANAR:
            case AV_CODEC_ID_PCM_U16BE:
            case AV_CODEC_ID_PCM_U16LE:
                avctx->audio.bits_per_sample =  16;
                break;
            case AV_CODEC_ID_PCM_S24DAUD:
            case AV_CODEC_ID_PCM_S24BE:
            case AV_CODEC_ID_PCM_S24LE:
            case AV_CODEC_ID_PCM_S24LE_PLANAR:
            case AV_CODEC_ID_PCM_U24BE:
            case AV_CODEC_ID_PCM_U24LE:
                avctx->audio.bits_per_sample =  24;
                break;
            case AV_CODEC_ID_PCM_S32BE:
            case AV_CODEC_ID_PCM_S32LE:
            case AV_CODEC_ID_PCM_S32LE_PLANAR:
            case AV_CODEC_ID_PCM_U32BE:
            case AV_CODEC_ID_PCM_U32LE:
            case AV_CODEC_ID_PCM_F32BE:
            case AV_CODEC_ID_PCM_F32LE:
            case AV_CODEC_ID_PCM_F24LE:
            case AV_CODEC_ID_PCM_F16LE:
                avctx->audio.bits_per_sample =  32;
                break;
            case AV_CODEC_ID_PCM_F64BE:
            case AV_CODEC_ID_PCM_F64LE:
            case AV_CODEC_ID_PCM_S64BE:
            case AV_CODEC_ID_PCM_S64LE:
                avctx->audio.bits_per_sample =  64;
                break;
            default:
                avctx->audio.bits_per_sample = 0;
        }

        switch (avctx->audio.codec_tag)
        {
            case AV_CODEC_ID_PCM_ALAW:
                for (i = 0; i < 256; i++)
                    s->table[i] = alaw2linear(i);
                break;
            case AV_CODEC_ID_PCM_MULAW:
                for (i = 0; i < 256; i++)
                    s->table[i] = ulaw2linear(i);
                break;
            case AV_CODEC_ID_PCM_F16LE:
            case AV_CODEC_ID_PCM_F24LE:
                s->scale = 1. / (1 << (avctx->audio.bits_per_coded_sample - 1));
                break;
            default:
                break;
        }
    }
}
