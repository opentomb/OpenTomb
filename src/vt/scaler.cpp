#ifdef _MSC_VER///@GH0ST
#include <SDL.h>
#include <SDL_endian.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#endif

static unsigned int colorMask = 0xF7DEF7DE;
static unsigned int lowPixelMask = 0x08210821;
static unsigned int qcolorMask = 0xE79CE79C;
static unsigned int qlowpixelMask = 0x18631863;
static unsigned int redblueMask = 0xF81F;
static unsigned int greenMask = 0x7E0;

int Init_2xSaI(unsigned int BitFormat)
{
    if (BitFormat == 565) {
        colorMask = (0xF7DEF7DE);
        lowPixelMask = (0x08210821);
        qcolorMask = (0xE79CE79C);
        qlowpixelMask = (0x18631863);
        redblueMask = (0xF81F);
        greenMask = (0x7E0);
    } else if (BitFormat == 555) {
        colorMask = (0x7BDE7BDE);
        lowPixelMask = (0x04210421);
        qcolorMask = (0x739C739C);
        qlowpixelMask = (0x0C630C63);
        redblueMask = (0x7C1F);
        greenMask = (0x3E0);
    } else {
        return 0;
    }

#ifdef MMX
    Init_2xSaIMMX(BitFormat);
#endif

    return 1;
}


void Scale2x(unsigned char *src, unsigned int src_pitch, int src_bytes_per_pixel, unsigned char *dst, unsigned int dst_pitch, int dst_bytes_per_pixel, int width, int height, int pal[256])
{
#define GET_COLOR(x) (pal[(x)])

    int x, y;
    unsigned char *src_line;
    unsigned char *dst_line[2];

    src_line = src;
    dst_line[0] = dst;
    dst_line[1] = dst + dst_pitch;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color;

            if (src_bytes_per_pixel == 1) {
                color = GET_COLOR(*(((unsigned char *)src_line) + x));
            } else if (src_bytes_per_pixel == 2) {
                color = *(((unsigned short *)src_line) + x);
            } else {
                color = *(((unsigned int *)src_line) + x);
            }

            if (dst_bytes_per_pixel == 2) {
                *((unsigned long *)(&dst_line[0][x * 4])) = color | (color << 16);
                *((unsigned long *)(&dst_line[1][x * 4])) = color | (color << 16);
            } else {
                *((unsigned long *)(&dst_line[0][x * 8])) = color;
                *((unsigned long *)(&dst_line[0][x * 8 + 4])) = color;
                *((unsigned long *)(&dst_line[1][x * 8])) = color;
                *((unsigned long *)(&dst_line[1][x * 8 + 4])) = color;
            }
        }

        src_line += src_pitch;

        if (y < height - 1) {
            dst_line[0] += dst_pitch * 2;
            dst_line[1] += dst_pitch * 2;
        }
    }
}


void Super2xSaI(unsigned char *src, unsigned int src_pitch, int src_bytes_per_pixel, unsigned char *dst, unsigned int dst_pitch, int dst_bytes_per_pixel, int width, int height, int pal[256])
{
#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

#define INTERPOLATE(A, B) (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask))

#define Q_INTERPOLATE(A, B, C, D) ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2) \
    + ((((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2) & qlowpixelMask)

#define GET_COLOR(x) (pal[(x)])

    unsigned char *src_line[4];
    unsigned char *dst_line[2];
    int x, y;
    unsigned long color[16];

    if ((width < 2) || (height < 2)) {
        Scale2x(src, src_pitch, src_bytes_per_pixel, dst, dst_pitch, dst_bytes_per_pixel, width, height, pal);
        return;
    }

    /* Point to the first 3 lines. */
    src_line[0] = src;
    src_line[1] = src;
    src_line[2] = src + src_pitch;
    src_line[3] = src + (src_pitch * 2);

    dst_line[0] = dst;
    dst_line[1] = dst + dst_pitch;

    if (src_bytes_per_pixel == 1) {
        unsigned char *sbp;

        sbp = src_line[0];
        color[0] = GET_COLOR(*sbp);
        color[1] = color[0];
        color[2] = color[0];
        color[3] = color[0];
        color[4] = color[0];
        color[5] = color[0];
        color[6] = GET_COLOR(*(sbp + 1));
        color[7] = GET_COLOR(*(sbp + 2));
        sbp = src_line[2];
        color[8] = GET_COLOR(*sbp);
        color[9] = color[8];
        color[10] = GET_COLOR(*(sbp + 1));
        color[11] = GET_COLOR(*(sbp + 2));
        sbp = src_line[3];
        color[12] = GET_COLOR(*sbp);
        color[13] = color[12];
        color[14] = GET_COLOR(*(sbp + 1));
        color[15] = GET_COLOR(*(sbp + 2));
    } else if (src_bytes_per_pixel == 2) {
        unsigned short *sbp;

        sbp = (unsigned short *)src_line[0];
        color[0] = *sbp;
        color[1] = color[0];
        color[2] = color[0];
        color[3] = color[0];
        color[4] = color[0];
        color[5] = color[0];
        color[6] = *(sbp + 1);
        color[7] = *(sbp + 2);
        sbp = (unsigned short *)src_line[2];
        color[8] = *sbp;
        color[9] = color[8];
        color[10] = *(sbp + 1);
        color[11] = *(sbp + 2);
        sbp = (unsigned short *)src_line[3];
        color[12] = *sbp;
        color[13] = color[12];
        color[14] = *(sbp + 1);
        color[15] = *(sbp + 2);
    } else {
        unsigned long *lbp;

        lbp = (unsigned long *)src_line[0];
        color[0] = *lbp;
        color[1] = color[0];
        color[2] = color[0];
        color[3] = color[0];
        color[4] = color[0];
        color[5] = color[0];
        color[6] = *(lbp + 1);
        color[7] = *(lbp + 2);
        lbp = (unsigned long *)src_line[2];
        color[8] = *lbp;
        color[9] = color[8];
        color[10] = *(lbp + 1);
        color[11] = *(lbp + 2);
        lbp = (unsigned long *)src_line[3];
        color[12] = *lbp;
        color[13] = color[12];
        color[14] = *(lbp + 1);
        color[15] = *(lbp + 2);
    }

    for (y = 0; y < height; y++) {

        /* Todo: x = width - 2, x = width - 1 */

        for (x = 0; x < width; x++) {
            unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------  B0 B1 B2 B3    0  1  2  3
//                                         4  5* 6  S2 -> 4  5* 6  7
//                                         1  2  3  S1    8  9 10 11
//                                         A0 A1 A2 A3   12 13 14 15
//--------------------------------------
            if (color[9] == color[6] && color[5] != color[10]) {
                product2b = color[9];
                product1b = product2b;
            } else if (color[5] == color[10] && color[9] != color[6]) {
                product2b = color[5];
                product1b = product2b;
            } else if (color[5] == color[10] && color[9] == color[6]) {
                int r = 0;

                r += GET_RESULT(color[6], color[5], color[8], color[13]);
                r += GET_RESULT(color[6], color[5], color[4], color[1]);
                r += GET_RESULT(color[6], color[5], color[14], color[11]);
                r += GET_RESULT(color[6], color[5], color[2], color[7]);

                if (r > 0)
                    product1b = color[6];
                else if (r < 0)
                    product1b = color[5];
                else
                    product1b = INTERPOLATE(color[5], color[6]);

                product2b = product1b;

            } else {
                if (color[6] == color[10] && color[10] == color[13] && color[9] != color[14] && color[10] != color[12])
                    product2b = Q_INTERPOLATE(color[10], color[10], color[10], color[9]);
                else if (color[5] == color[9] && color[9] == color[14] && color[13] != color[10] && color[9] != color[15])
                    product2b = Q_INTERPOLATE(color[9], color[9], color[9], color[10]);
                else
                    product2b = INTERPOLATE(color[9], color[10]);

                if (color[6] == color[10] && color[6] == color[1] && color[5] != color[2] && color[6] != color[0])
                    product1b = Q_INTERPOLATE(color[6], color[6], color[6], color[5]);
                else if (color[5] == color[9] && color[5] == color[2] && color[1] != color[6] && color[5] != color[3])
                    product1b = Q_INTERPOLATE(color[6], color[5], color[5], color[5]);
                else
                    product1b = INTERPOLATE(color[5], color[6]);
            }

            if (color[5] == color[10] && color[9] != color[6] && color[4] == color[5] && color[5] != color[14])
                product2a = INTERPOLATE(color[9], color[5]);
            else if (color[5] == color[8] && color[6] == color[5] && color[4] != color[9] && color[5] != color[12])
                product2a = INTERPOLATE(color[9], color[5]);
            else
                product2a = color[9];

            if (color[9] == color[6] && color[5] != color[10] && color[8] == color[9] && color[9] != color[2])
                product1a = INTERPOLATE(color[9], color[5]);
            else if (color[4] == color[9] && color[10] == color[9] && color[8] != color[5] && color[9] != color[0])
                product1a = INTERPOLATE(color[9], color[5]);
            else
                product1a = color[5];

            if (dst_bytes_per_pixel == 2) {
                unsigned long tmp;

                //*((unsigned long *) (&dst_line[0][x * 4])) = product1a | (product1b << 16);
                //*((unsigned long *) (&dst_line[1][x * 4])) = product2a | (product2b << 16);
                tmp = SDL_SwapLE16(product1a) | SDL_SwapLE16(product1b) << 16;
                *((unsigned long *)(&dst_line[0][x * 4])) = SDL_SwapLE32(tmp);
                tmp = SDL_SwapLE16(product2a) | SDL_SwapLE16(product2b) << 16;
                *((unsigned long *)(&dst_line[1][x * 4])) = SDL_SwapLE32(tmp);
            } else {
                *((unsigned long *)(&dst_line[0][x * 8])) = product1a;
                *((unsigned long *)(&dst_line[0][x * 8 + 4])) = product1b;
                *((unsigned long *)(&dst_line[1][x * 8])) = product2a;
                *((unsigned long *)(&dst_line[1][x * 8 + 4])) = product2b;
            }

            /* Move color matrix forward */
            color[0] = color[1];
            color[4] = color[5];
            color[8] = color[9];
            color[12] = color[13];
            color[1] = color[2];
            color[5] = color[6];
            color[9] = color[10];
            color[13] = color[14];
            color[2] = color[3];
            color[6] = color[7];
            color[10] = color[11];
            color[14] = color[15];

            if (x < width - 3) {
                x += 3;
                if (src_bytes_per_pixel == 1) {
                    color[3] = GET_COLOR(*(((unsigned char *)src_line[0]) + x));
                    color[7] = GET_COLOR(*(((unsigned char *)src_line[1]) + x));
                    color[11] = GET_COLOR(*(((unsigned char *)src_line[2]) + x));
                    color[15] = GET_COLOR(*(((unsigned char *)src_line[3]) + x));
                } else if (src_bytes_per_pixel == 2) {
                    color[3] = *(((unsigned short *)src_line[0]) + x);
                    color[7] = *(((unsigned short *)src_line[1]) + x);
                    color[11] = *(((unsigned short *)src_line[2]) + x);
                    color[15] = *(((unsigned short *)src_line[3]) + x);
                } else {
                    color[3] = *(((unsigned long *)src_line[0]) + x);
                    color[7] = *(((unsigned long *)src_line[1]) + x);
                    color[11] = *(((unsigned long *)src_line[2]) + x);
                    color[15] = *(((unsigned long *)src_line[3]) + x);
                }
                x -= 3;
            }
        }

        /* We're done with one line, so we shift the source lines up */
        src_line[0] = src_line[1];
        src_line[1] = src_line[2];
        src_line[2] = src_line[3];

        /* Read next line */
        if (y + 3 >= height)
            src_line[3] = src_line[2];
        else
            src_line[3] = src_line[2] + src_pitch;

        /* Then shift the color matrix up */
        if (src_bytes_per_pixel == 1) {
            unsigned char *sbp;

            sbp = src_line[0];
            color[0] = GET_COLOR(*sbp);
            color[1] = color[0];
            color[2] = GET_COLOR(*(sbp + 1));
            color[3] = GET_COLOR(*(sbp + 2));
            sbp = src_line[1];
            color[4] = GET_COLOR(*sbp);
            color[5] = color[4];
            color[6] = GET_COLOR(*(sbp + 1));
            color[7] = GET_COLOR(*(sbp + 2));
            sbp = src_line[2];
            color[8] = GET_COLOR(*sbp);
            color[9] = color[8];
            color[10] = GET_COLOR(*(sbp + 1));
            color[11] = GET_COLOR(*(sbp + 2));
            sbp = src_line[3];
            color[12] = GET_COLOR(*sbp);
            color[13] = color[12];
            color[14] = GET_COLOR(*(sbp + 1));
            color[15] = GET_COLOR(*(sbp + 2));
        } else if (src_bytes_per_pixel == 2) {
            unsigned short *sbp;

            sbp = (unsigned short *)src_line[0];
            color[0] = *sbp;
            color[1] = color[0];
            color[2] = *(sbp + 1);
            color[3] = *(sbp + 2);
            sbp = (unsigned short *)src_line[1];
            color[4] = *sbp;
            color[5] = color[4];
            color[6] = *(sbp + 1);
            color[7] = *(sbp + 2);
            sbp = (unsigned short *)src_line[2];
            color[8] = *sbp;
            color[9] = color[9];
            color[10] = *(sbp + 1);
            color[11] = *(sbp + 2);
            sbp = (unsigned short *)src_line[3];
            color[12] = *sbp;
            color[13] = color[12];
            color[14] = *(sbp + 1);
            color[15] = *(sbp + 2);
        } else {
            unsigned long *lbp;

            lbp = (unsigned long *)src_line[0];
            color[0] = *lbp;
            color[1] = color[0];
            color[2] = *(lbp + 1);
            color[3] = *(lbp + 2);
            lbp = (unsigned long *)src_line[1];
            color[4] = *lbp;
            color[5] = color[4];
            color[6] = *(lbp + 1);
            color[7] = *(lbp + 2);
            lbp = (unsigned long *)src_line[2];
            color[8] = *lbp;
            color[9] = color[9];
            color[10] = *(lbp + 1);
            color[11] = *(lbp + 2);
            lbp = (unsigned long *)src_line[3];
            color[12] = *lbp;
            color[13] = color[12];
            color[14] = *(lbp + 1);
            color[15] = *(lbp + 2);
        }

        if (y < height - 1) {
            dst_line[0] += dst_pitch * 2;
            dst_line[1] += dst_pitch * 2;
        }
    }
}
