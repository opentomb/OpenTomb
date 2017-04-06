
#include <string.h>
#include <stdlib.h>

#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_endian.h>

#include <png.h>
#include <SDL2/SDL_pixels.h>

#include "image.h"


/// png and pcx code partially taken from SDL_Image, partialy from github and forums.
/* Load a PNG type image from an SDL datasource */
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
    SDL_RWops *src = (SDL_RWops *)png_get_io_ptr(ctx);
    SDL_RWread(src, area, size, 1);
}

static int Image_LoadPNG(const char *file_name, uint8_t **buffer, uint32_t *w, uint32_t *h, uint32_t *bpp)
{
    png_byte png_header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    SDL_RWops *src = SDL_RWFromFile(file_name, "rb");
    if(!src)
    {
        return 0;
    }

    SDL_RWread(src, png_header, 1, 8);
    if(png_sig_cmp(png_header, 0, 8))
    {
        SDL_RWclose(src);
        return 0;
    }

    SDL_RWseek(src, 0, RW_SEEK_SET);
    /* initialize stuff */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr)
    {
        SDL_RWclose(src);
        return 0;
    }

    png_infop read_info_ptr = png_create_info_struct(png_ptr);
    png_infop end_info_ptr = png_create_info_struct(png_ptr);
    if(!read_info_ptr || !end_info_ptr)
    {
        png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
        SDL_RWclose(src);
        return 0;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
        SDL_RWclose(src);
        return 0;
    }

    png_set_read_fn(png_ptr, src, png_read_data);
    png_read_info(png_ptr, read_info_ptr);

    *w = png_get_image_width(png_ptr, read_info_ptr);
    *h = png_get_image_height(png_ptr, read_info_ptr);
    int color_type = png_get_color_type(png_ptr, read_info_ptr);

    /* tell libpng to strip 16 bit/color files down to 8 bits/color */
    png_set_strip_16(png_ptr) ;

    /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
     * byte into separate bytes (useful for paletted and grayscale images).
     */
    png_set_packing(png_ptr);

    /* scale greyscale values to the range 0..255 */
    if(color_type == PNG_COLOR_TYPE_GRAY)
    {
        png_set_expand(png_ptr);
    }

    /* For images with a single "transparent colour", set colour key;
       if more than one index has transparency, or if partially transparent
       entries exist, use full alpha channel */
    if(png_get_valid(png_ptr, read_info_ptr, PNG_INFO_tRNS))
    {
        png_color_16 *transv = NULL;
        int num_trans;
        uint8_t *trans;
        png_get_tRNS(png_ptr, read_info_ptr, &trans, &num_trans, &transv);
        if(color_type == PNG_COLOR_TYPE_PALETTE)
        {
            /* Check if all tRNS entries are opaque except one */
            int j, t = -1;
            for(j = 0; j < num_trans; j++)
            {
                if(trans[j] == 0)
                {
                    if(t >= 0)
                    {
                        break;
                    }
                    t = j;
                }
                else if(trans[j] != 255)
                {
                    break;
                }
            }
            if(j != num_trans)
            {
                /* more than one transparent index, or translucency */
                png_set_expand(png_ptr);
            }
        }
    }

    if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png_ptr);
    }

    //int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, read_info_ptr);
    color_type = png_get_color_type(png_ptr, read_info_ptr);

    if(color_type == (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA))
    {
        *bpp = 8 * 4;
    }
    else if(color_type == PNG_COLOR_MASK_COLOR)
    {
        *bpp = 8 * 3;
    }

    /* read file */
    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
        SDL_RWclose(src);
        return 0;
    }

    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*h));
    int row_bytes = png_get_rowbytes(png_ptr, read_info_ptr);
    for(uint32_t y = 0; y < (*h); y++)
    {
        row_pointers[y] = (png_byte*)malloc(row_bytes);
    }

    png_read_image(png_ptr, row_pointers);

    *buffer = (uint8_t*)malloc(row_bytes * (*h));
    uint8_t *p = *buffer;
    for(uint32_t y = 0; y < (*h); y++)
    {
        memcpy(p, row_pointers[y], row_bytes);
        p += row_bytes;
    }

    for(uint32_t y = 0; y < (*h); y++)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);

    png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
    SDL_RWclose(src);
    return 1;
}


struct PCXheader
{
    uint8_t Manufacturer;
    uint8_t Version;
    uint8_t Encoding;
    uint8_t BitsPerPixel;
    int16_t Xmin, Ymin, Xmax, Ymax;
    int16_t HDpi, VDpi;
    uint8_t Colormap[48];
    uint8_t Reserved;
    uint8_t NPlanes;
    int16_t BytesPerLine;
    int16_t PaletteInfo;
    int16_t HscreenSize;
    int16_t VscreenSize;
    uint8_t Filler[54];
};

/* See if an image is contained in a data source */
int IMG_isPCX(SDL_RWops *src)
{
    int64_t start;
    int is_PCX = 0;
    const int ZSoft_Manufacturer = 10;
    const int PC_Paintbrush_Version = 5;
    const int PCX_Uncompressed_Encoding = 0;
    const int PCX_RunLength_Encoding = 1;
    struct PCXheader pcxh;
    start = SDL_RWtell(src);

    if(SDL_RWread(src, &pcxh, sizeof(pcxh), 1) == 1)
    {
        if ( (pcxh.Manufacturer == ZSoft_Manufacturer) &&
             (pcxh.Version == PC_Paintbrush_Version) &&
             (pcxh.Encoding == PCX_RunLength_Encoding ||
              pcxh.Encoding == PCX_Uncompressed_Encoding) )
        {
            is_PCX = 1;
        }
    }
    SDL_RWseek(src, start, RW_SEEK_SET);
    return is_PCX;
}

/* Load a PCX type image from an SDL datasource */
static int Image_LoadPCX(const char *file_name, uint8_t **buffer, uint32_t *w, uint32_t *h, uint32_t *bpp)
{
    struct PCXheader pcxh;
    uint32_t y, bpl;
    int bits, src_bits;
    SDL_RWops *src = SDL_RWFromFile(file_name, "rb");

    if(!src)
    {
        /* The error message has been set in SDL_RWFromFile */
        return 0;
    }

    if(!IMG_isPCX(src))
    {
        SDL_RWclose(src);
        return 0;
    }

    if(!SDL_RWread(src, &pcxh, sizeof(pcxh), 1))
    {
        SDL_RWclose(src);
        return 0;
    }
    *bpp = 24;
    pcxh.Xmin = SDL_SwapLE16(pcxh.Xmin);
    pcxh.Ymin = SDL_SwapLE16(pcxh.Ymin);
    pcxh.Xmax = SDL_SwapLE16(pcxh.Xmax);
    pcxh.Ymax = SDL_SwapLE16(pcxh.Ymax);
    pcxh.BytesPerLine = SDL_SwapLE16(pcxh.BytesPerLine);

    *w = (pcxh.Xmax - pcxh.Xmin) + 1;
    *h = (pcxh.Ymax - pcxh.Ymin) + 1;
    src_bits = pcxh.BitsPerPixel * pcxh.NPlanes;
    if((pcxh.BitsPerPixel == 1 && pcxh.NPlanes >= 1 && pcxh.NPlanes <= 4)
       || (pcxh.BitsPerPixel == 8 && pcxh.NPlanes == 1))
    {
        bits = 8;
    }
    else if(pcxh.BitsPerPixel == 8 && pcxh.NPlanes == 3)
    {
        bits = 24;
    }
    else
    {
        SDL_RWclose(src);
        return 0;
    }

    bpl = pcxh.NPlanes * pcxh.BytesPerLine;
    uint8_t *orig_pixels = (uint8_t *)malloc(bpl * (*h));
    uint8_t *temp_line = (uint8_t *)malloc(bpl);
    uint8_t *row = orig_pixels;
    for(y = 0; y < (*h); ++y)
    {
        /* decode a scan line to a temporary buffer first */
        uint32_t i, count = 0;
        uint8_t ch;
        uint8_t *dst = (src_bits == 8) ? row : temp_line;
        if(pcxh.Encoding == 0)
        {
            if(!SDL_RWread(src, dst, bpl, 1))
            {
                free(temp_line);
                free(orig_pixels);
                SDL_RWclose(src);
                return 0;
            }
        }
        else
        {
            for(i = 0; i < bpl; i++)
            {
                if(!count)
                {
                    if(!SDL_RWread(src, &ch, 1, 1))
                    {
                        free(temp_line);
                        free(orig_pixels);
                        SDL_RWclose(src);
                        return 0;
                    }
                    if((ch & 0xc0) == 0xc0)
                    {
                        count = ch & 0x3f;
                        if(!SDL_RWread(src, &ch, 1, 1))
                        {
                            free(temp_line);
                            free(orig_pixels);
                            SDL_RWclose(src);
                            return 0;
                        }
                    }
                    else
                    {
                        count = 1;
                    }
                }
                dst[i] = ch;
                count--;
            }
        }

        if(src_bits <= 4)
        {
            /* expand planes to 1 byte/pixel */
            uint8_t *innerSrc = temp_line;
            int plane;
            for(plane = 0; plane < pcxh.NPlanes; plane++)
            {
                uint32_t j, x = 0;
                for(j = 0; j < pcxh.BytesPerLine; j++)
                {
                    uint8_t byte = *innerSrc++;
                    int sk = 7;
                    for(; sk >= 0; sk--)
                    {
                        unsigned bit = (byte >> sk) & 1;
                        /* skip padding bits */
                        if(j * 8 + sk >= (*w))
                        {
                            continue;
                        }
                        row[x++] |= bit << plane;
                    }
                }
            }
        }
        else if(src_bits == 24)
        {
            /* de-interlace planes */
            uint8_t *innerSrc = temp_line;
            int plane;
            for(plane = 0; plane < pcxh.NPlanes; plane++)
            {
                uint32_t x;
                dst = row + plane;
                for(x = 0; x < (*w); x++)
                {
                    *dst = *innerSrc++;
                    dst += pcxh.NPlanes;
                }
            }
        }

        row += bpl;
    }
    free(temp_line);

    if(bits == 8)
    {
        int i;
        int nc = 1 << src_bits;
        SDL_Color *colors = (SDL_Color*)malloc(sizeof(SDL_Color) * nc);

        if(src_bits == 8)
        {
            uint8_t ch;
            /* look for a 256-colour palette */
            do
            {
                if(!SDL_RWread(src, &ch, 1, 1))
                {
                    free(colors);
                    free(orig_pixels);
                    SDL_RWclose(src);
                    return 0;
                }
            } while(ch != 12);

            for(i = 0; i < 256; i++)
            {
                SDL_RWread(src, &colors[i].r, 1, 1);
                SDL_RWread(src, &colors[i].g, 1, 1);
                SDL_RWread(src, &colors[i].b, 1, 1);
            }
        }
        else
        {
            for(i = 0; i < nc; i++)
            {
                colors[i].r = pcxh.Colormap[i * 3];
                colors[i].g = pcxh.Colormap[i * 3 + 1];
                colors[i].b = pcxh.Colormap[i * 3 + 2];
            }
        }

        *buffer = (uint8_t *)malloc((*w) * (*h) * 3);
        uint8_t *dst_row = *buffer;
        uint8_t *src_row = orig_pixels;
        for(y = 0; y < (*h); ++y)
        {
            for(uint32_t x = 0; x < *w; x++)
            {
                if(src_row[x] < nc)
                {
                    dst_row[x * 3 + 0] = colors[src_row[x]].r;
                    dst_row[x * 3 + 1] = colors[src_row[x]].g;
                    dst_row[x * 3 + 2] = colors[src_row[x]].b;
                }
                else
                {
                    dst_row[x * 3 + 0] = 0;
                    dst_row[x * 3 + 1] = 0;
                    dst_row[x * 3 + 2] = 0;
                }
            }
            src_row += bpl;
            dst_row += (*w) * 3;
        }
        free(colors);
        free(orig_pixels);
    }
    else if(bits == 24)
    {
        *buffer = orig_pixels;
    }

    SDL_RWclose(src);
    return 1;
}


int Image_Load(const char *file_name, int format, uint8_t **buffer, uint32_t *w, uint32_t *h, uint32_t *bpp)
{
    switch(format)
    {
        case IMAGE_FORMAT_PNG:
            return Image_LoadPNG(file_name, buffer, w, h, bpp);

        case IMAGE_FORMAT_PCX:
            return Image_LoadPCX(file_name, buffer, w, h, bpp);

        default:
            return 0;
    }
}

//------------------------------------------------------------------------------

static void png_write_data(png_structp ctx, png_bytep area, png_size_t size)
{
    SDL_RWops *dst = (SDL_RWops *)png_get_io_ptr(ctx);
    SDL_RWwrite(dst, area, size, 1);
}


static void png_flush_data(png_structp ctx)
{
}


static int Image_SavePNG(const char *file_name, uint8_t *buffer, uint32_t w, uint32_t h, uint32_t bpp)
{
    SDL_RWops *dst = SDL_RWFromFile(file_name, "wb");
    int cell_size;
    int png_color_type;

    if(!dst)
    {
        return 0;
    }

    if(bpp == 24)
    {
        png_color_type = PNG_COLOR_TYPE_RGB;
        cell_size = 3;
    }
    else if(bpp == 32)
    {
        png_color_type = PNG_COLOR_TYPE_RGBA;
        cell_size = 4;
    }
    else
    {
        SDL_RWclose(dst);
        return 0;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL)
    {
        SDL_RWclose(dst);
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        SDL_RWclose(dst);
        return 0;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        SDL_RWclose(dst);
        return 0;
    }

    //png_set_compression_level(png_ptr, 9);
    png_set_write_fn(png_ptr, dst, png_write_data, png_flush_data);

    png_set_IHDR(png_ptr, info_ptr, w, h,
         8, png_color_type, PNG_INTERLACE_NONE,
         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    // Write image data
    for(uint32_t y = 0 ; y < h ; y++)
    {
        png_bytep row = (png_bytep)(buffer + (h - y - 1) * w * cell_size);
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);
    SDL_RWclose(dst);

    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return 1;
}


int Image_Save(const char *file_name, int format, uint8_t *buffer, uint32_t w, uint32_t h, uint32_t bpp)
{
    switch(format)
    {
        case IMAGE_FORMAT_PNG:
            return Image_SavePNG(file_name, buffer, w, h, bpp);

        default:
            return 0;
    }
}
