
/*
 * File:   tiny_codec.h
 * Author: nTesla64a
 *
 * Created on August 31, 2017, 5:06 PM
 */

#ifndef TINY_CODEC_H
#define TINY_CODEC_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AV_PKT_FLAG_KEY     0x0001 ///< The packet contains a keyframe
#define AV_PKT_FLAG_CORRUPT 0x0002 ///< The packet content is corrupted

typedef struct AVPacket
{
    /**
     * A reference to the reference-counted buffer where the packet data is
     * stored.
     * May be NULL, then the packet data is not reference-counted.
     */
    //AVBufferRef *buf;
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    int64_t pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    int64_t dts;
    uint8_t *data;
    int   size;
    int   allocated_size;
    int   stream_index;
    /**
     * A combination of AV_PKT_FLAG values
     */
    uint16_t   flags;
    uint16_t   is_video;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    int64_t duration;

    int64_t pos;                            ///< byte position in stream, -1 if unknown
} AVPacket;

typedef struct index_entry_s
{
    int64_t pos;
    int64_t timestamp;
    int size;
    int distance;
    int flags;
}index_entry_t, *index_entry_p;

typedef struct tiny_codec_s
{
    struct SDL_RWops   *pb;
    void               *private_context;
    void              (*free_context)(void *context);
    int               (*packet)(struct tiny_codec_s *s, struct AVPacket *pkt);
    uint64_t            fps_num;
    uint64_t            fps_denum;
    struct
    {
        uint32_t        codec_tag;
        uint16_t        width;
        uint16_t        height;
        uint32_t        line_bytes;
        uint8_t        *rgba;
        uint8_t        *buff;
        void           *priv_data;
        void          (*free_data)(void *data);
        int           (*decode)(struct tiny_codec_s *s, struct AVPacket *pkt);

        uint32_t                entry_current;
        uint32_t                entry_size;
        struct index_entry_s   *entry;
    } video;

    struct
    {
        uint32_t        codec_tag;
        uint16_t        frquency;
        uint16_t        format;
        uint16_t        bit_rate;
        uint16_t        sample_rate;
        uint16_t        bits_per_coded_sample;
        uint16_t        channels;
        void           *priv_data;
        void          (*free_data)(void *data);
        uint32_t      (*decode)(struct tiny_codec_s *s, struct AVPacket *pkt);
        uint32_t        buff_size;
        uint32_t        buff_offset;
        uint8_t        *buff;

        uint32_t                entry_current;
        uint32_t                entry_size;
        struct index_entry_s   *entry;
    } audio;
}tiny_codec_t, *tiny_codec_p;


void av_init_packet(AVPacket *pkt);
int av_get_packet(SDL_RWops *pb, AVPacket *pkt, int size);
void av_packet_unref(AVPacket *pkt);


void codec_init(struct tiny_codec_s *s, SDL_RWops *rw);
void codec_clear(struct tiny_codec_s *s);

int codec_open_rpl(struct tiny_codec_s *s);

#ifdef __cplusplus
}
#endif

#endif /* TINY_CODEC_H */

