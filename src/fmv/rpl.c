/*
 * ARMovie/RPL demuxer
 * Copyright (c) 2007 Christian Ohm, 2008 Eli Friedman
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

#include <inttypes.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "tiny_codec.h"


#define RPL_SIGNATURE "ARMovie"
#define RPL_SIGNATURE_SIZE 8

/** 256 is arbitrary, but should be big enough for any reasonable file. */
#define RPL_LINE_LENGTH 256

typedef struct RPLContext
{
    // RPL header data
    int32_t frames_per_chunk;

    // Stream position data
    uint32_t chunk_number;
    uint32_t chunk_part;
    uint32_t frame_in_part;
} RPLContext;

static int read_line(SDL_RWops *pb, char* line, int bufsize)
{
    int i;
    uint8_t b = 0;
    for (i = 0; (i < bufsize - 1) && SDL_RWread(pb, &b, 1, 1); i++)
    {
        if (b == 0)
            break;

        if(b == '\n')
        {
            line[i] = '\0';
            return 0;
        }
        line[i] = b;
    }
    line[i] = '\0';
    return -1;
}

static int32_t read_int(const char* line, const char** endptr, int* error)
{
    unsigned long result = 0;
    for (; *line>='0' && *line<='9'; line++) {
        if (result > (0x7FFFFFFF - 9) / 10)
            *error = -1;
        result = 10 * result + *line - '0';
    }
    *endptr = line;
    return result;
}

static int32_t read_line_and_int(SDL_RWops *pb, int* error)
{
    char line[RPL_LINE_LENGTH];
    const char *endptr;
    *error |= read_line(pb, line, sizeof(line));
    return read_int(line, &endptr, error);
}

/** Parsing for fps, which can be a fraction. Unfortunately,
  * the spec for the header leaves out a lot of details,
  * so this is mostly guessing.
  */
/*static AVRational read_fps(const char* line, int* error)
{
    int64_t num, den = 1;
    AVRational result;
    num = read_int(line, &line, error);
    if (*line == '.')
        line++;
    for (; *line>='0' && *line<='9'; line++) {
        // Truncate any numerator too large to fit into an int64_t
        if (num > (INT64_MAX - 9) / 10 || den > INT64_MAX / 10)
            break;
        num  = 10 * num + *line - '0';
        den *= 10;
    }
    if (!num)
        *error = -1;
    av_reduce(&result.num, &result.den, num, den, 0x7FFFFFFF);
    return result;
}*/


static int rpl_read_packet(struct tiny_codec_s *s, struct AVPacket *pkt)
{
    SDL_RWops *pb = s->pb;
    RPLContext *rpl = (RPLContext*)s->private_context;
    int ret = -1;
    //AVStream* stream;
    index_entry_p index_entry;

    if(rpl->chunk_part == rpl->frames_per_chunk)
    {
        rpl->chunk_number++;
        rpl->chunk_part = 0;
    }

    if (rpl->chunk_number >= s->video.entry_size)
        return -1;

    index_entry = &s->video.entry[rpl->chunk_number];
    
    if((rpl->frame_in_part == 0) && (SDL_RWseek(pb, index_entry->pos, RW_SEEK_SET) < 0))
        return -1;

    if (s->video.codec_tag == 124)
    {
        // We have to split Escape 124 frames because there are
        // multiple frames per chunk in Escape 124 samples.
        uint32_t frame_size;

        SDL_ReadLE32(pb);//avio_skip(pb, 4); // flags `
        frame_size = SDL_ReadLE32(pb);
        if (SDL_RWseek(pb, -8, RW_SEEK_CUR) < 0)
            return -1;

        ret = av_get_packet(pb, pkt, frame_size);
        if (ret < 0)
            return ret;
        if (ret != frame_size)
        {
            av_packet_unref(pkt);
            return -1;
        }
        pkt->duration = 1;
        pkt->pts = index_entry->timestamp + rpl->frame_in_part;
        pkt->stream_index = rpl->chunk_part;

        rpl->frame_in_part++;
        if (rpl->frame_in_part == rpl->frames_per_chunk)
        {
            rpl->frame_in_part = 0;
            rpl->chunk_part++;
        }
    }
    else
    {
        ret = av_get_packet(pb, pkt, index_entry->size);
        if (ret < 0)
            return ret;
        if (ret != index_entry->size)
        {
            av_packet_unref(pkt);
            return -1;
        }

        if (pkt->is_video)
        {
            // frames_per_chunk should always be one here; the header
            // parsing will warn if it isn't.
            pkt->duration = rpl->frames_per_chunk;
        }
        else
        {
            // All the audio codecs supported in this container
            // (at least so far) are constant-bitrate.
            pkt->duration = ret * 8;
        }
        pkt->pts = index_entry->timestamp;
        pkt->stream_index = rpl->chunk_part;
        rpl->chunk_part++;
    }

    // None of the Escape formats have keyframes, and the ADPCM
    // format used doesn't have keyframes.
    if (rpl->chunk_number == 1 && rpl->frame_in_part == 0)
        pkt->flags |= AV_PKT_FLAG_KEY;

    return ret;
}


int escape124_decode_frame(struct tiny_codec_s *avctx, struct AVPacket *avpkt);
void escape124_decode_init(struct tiny_codec_s *avctx);

int rpl_read_header(struct tiny_codec_s *s)
{
    SDL_RWops *pb = s->pb;
    s->private_context = (RPLContext*)calloc(sizeof(RPLContext), 1);
    s->free_context = free;
    RPLContext *rpl = (RPLContext*)s->private_context;
    int total_audio_size;
    int codec_tag;
    int error = 0;

    uint32_t i;
    int32_t audio_format, chunk_catalog_offset, number_of_chunks;
    //AVRational fps;

    char line[RPL_LINE_LENGTH];
    
    SDL_RWseek(pb, 0, RW_SEEK_SET);
    // The header for RPL/ARMovie files is 21 lines of text
    // containing the various header fields.  The fields are always
    // in the same order, and other text besides the first
    // number usually isn't important.
    // (The spec says that there exists some significance
    // for the text in a few cases; samples needed.)
    error |= read_line(pb, line, sizeof(line));      // ARMovie
    if(strncmp(line, RPL_SIGNATURE, RPL_LINE_LENGTH))
    {
        return -1;
    }
    error |= read_line(pb, line, sizeof(line));      // movie name
    //av_dict_set(&s->metadata, "title"    , line, 0);
    error |= read_line(pb, line, sizeof(line));      // date/copyright
    //av_dict_set(&s->metadata, "copyright", line, 0);
    error |= read_line(pb, line, sizeof(line));      // author and other
    //av_dict_set(&s->metadata, "author"   , line, 0);

    // video headers
    /*vst = avformat_new_stream(s, NULL);
    if (!vst)
        return -1;*/
    /*vst->codecpar->codec_type      = AVMEDIA_TYPE_VIDEO;*/
    codec_tag = read_line_and_int(pb, &error);  // video format
    s->packet = rpl_read_packet;
    s->video.width           = read_line_and_int(pb, &error);  // video width
    s->video.height          = read_line_and_int(pb, &error);  // video height
    read_line_and_int(pb, &error);  // video bits per sample
    error |= read_line(pb, line, sizeof(line));                   // video frames per second
    /*fps = read_fps(line, &error);
    avpriv_set_pts_info(vst, 32, fps.den, fps.num);*/
    s->video.rgba = NULL;
    // Figure out the video codec
    switch (codec_tag)
    {
#if 0
        case 122:
            vst->codecpar->codec_id = AV_CODEC_ID_ESCAPE122;
            break;
#endif
        case 124:
            s->video.decode = escape124_decode_frame;
            s->video.codec_tag = 124;
            s->video.line_bytes = s->video.width * 2;
            s->video.buff = (uint8_t*)calloc(1, 2 * s->video.line_bytes * s->video.height);
            escape124_decode_init(s);
            // The header is wrong here, at least sometimes
            break;

        case 130:
            s->video.decode = NULL;//AV_CODEC_ID_ESCAPE130;
            s->video.codec_tag = 130;
            break;

        default:
            s->video.decode = NULL;
            s->video.codec_tag = 0;
            break;
    }

    // Audio headers

    // ARMovie supports multiple audio tracks; I don't have any
    // samples, though. This code will ignore additional tracks.
    audio_format = read_line_and_int(pb, &error);  // audio format ID
    s->audio.buff = NULL;
    s->audio.decode = NULL;
    if(audio_format)
    {
        s->audio.format          = audio_format;
        s->audio.sample_rate     = read_line_and_int(pb, &error);  // audio bitrate
        s->audio.channels        = read_line_and_int(pb, &error);  // number of audio channels
        s->audio.bits_per_coded_sample = read_line_and_int(pb, &error);  // audio bits per sample
        // At least one sample uses 0 for ADPCM, which is really 4 bits
        // per sample.
        if (s->audio.bits_per_coded_sample == 0)
            s->audio.bits_per_coded_sample = 4;

        s->audio.bit_rate = s->audio.sample_rate * s->audio.bits_per_coded_sample * s->audio.channels;

        switch (audio_format)
        {
            case 1:
                if(s->audio.bits_per_coded_sample == 16)
                {
                    // 16-bit audio is always signed
                    s->audio.decode = NULL;//AV_CODEC_ID_PCM_S16LE;
                    s->audio.codec_tag = 0;
                    break;
                }
                // There are some other formats listed as legal per the spec;
                // samples needed.
                break;
            case 101:
                if (s->audio.bits_per_coded_sample == 8)
                {
                    // The samples with this kind of audio that I have
                    // are all unsigned.
                    s->audio.decode = NULL;//AV_CODEC_ID_PCM_U8;
                    s->audio.codec_tag = 0;
                    break;
                }
                else if (s->audio.bits_per_coded_sample == 4)
                {
                    s->audio.decode = NULL;//AV_CODEC_ID_ADPCM_IMA_EA_SEAD;
                    s->audio.codec_tag = 0;
                    break;
                }
                break;
        }
        //if (ast->codecpar->codec_id == AV_CODEC_ID_NONE)
        //    avpriv_request_sample(s, "Audio format %"PRId32,
        //                          audio_format);
        //avpriv_set_pts_info(ast, 32, 1, ast->codecpar->bit_rate);
    }
    else
    {
        for (i = 0; i < 3; i++)
            error |= read_line(pb, line, sizeof(line));
    }

    rpl->frames_per_chunk = read_line_and_int(pb, &error);  // video frames per chunk
    //if (rpl->frames_per_chunk > 1 && vst->codecpar->codec_tag != 124)
    //    av_log(s, AV_LOG_WARNING,
    //           "Don't know how to split frames for video format %s. "
    //           "Video stream will be broken!\n", av_fourcc2str(vst->codecpar->codec_tag));

    number_of_chunks = read_line_and_int(pb, &error);  // number of chunks in the file
    // The number in the header is actually the index of the last chunk.
    number_of_chunks++;

    error |= read_line(pb, line, sizeof(line));  // "even" chunk size in bytes
    error |= read_line(pb, line, sizeof(line));  // "odd" chunk size in bytes
    chunk_catalog_offset =                       // offset of the "chunk catalog"
        read_line_and_int(pb, &error);           //   (file index)
    error |= read_line(pb, line, sizeof(line));  // offset to "helpful" sprite
    error |= read_line(pb, line, sizeof(line));  // size of "helpful" sprite
    error |= read_line(pb, line, sizeof(line));  // offset to key frame list

    // Read the index
    SDL_RWseek(pb, chunk_catalog_offset, RW_SEEK_SET);
    total_audio_size = 0;

    s->video.entry_size = number_of_chunks;
    s->video.entry = (index_entry_p)malloc(number_of_chunks * sizeof(index_entry_t));
    s->audio.entry_size = number_of_chunks;
    s->audio.entry = (index_entry_p)malloc(number_of_chunks * sizeof(index_entry_t));

    for (i = 0; !error && i < number_of_chunks; i++)
    {
        int64_t offset, video_size, audio_size;
        error |= read_line(pb, line, sizeof(line));
        if (3 != sscanf(line, "%"SCNd64" , %"SCNd64" ; %"SCNd64,
                        &offset, &video_size, &audio_size))
        {
            error = -1;
            continue;
        }
        s->video.entry[i].pos = offset;
        s->video.entry[i].timestamp = i * rpl->frames_per_chunk;
        s->video.entry[i].size = video_size;
        s->video.entry[i].distance = rpl->frames_per_chunk;
        s->video.entry[i].flags = 0;

        s->audio.entry[i].pos = offset + video_size;
        s->audio.entry[i].timestamp = total_audio_size;
        s->audio.entry[i].size = audio_size;
        s->audio.entry[i].distance = audio_size * 8;
        s->audio.entry[i].flags = 0;

        total_audio_size += audio_size * 8;
    }

    return error;
}

