/*
 * File:   gl_font.c
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#include <stdint.h>
#include <string.h>
#include "utf8_32.h"


static uint8_t *utf8_next_symbol(uint8_t *utf8)
{
    uint8_t b = *utf8;

    // save ASC symbol as is
    if(!(b & 0x80))
    {
        return utf8 + 1;
    }

    // calculate lenght
    while(b & 0x80)
    {
        b <<= 1;
        utf8++;
    }

    return utf8;
}


uint32_t utf8_strlen(const char *str)
{
    uint32_t i = 0;
    uint8_t *ch = (uint8_t*)str;

    for(; *ch; i++)
    {
        ch = utf8_next_symbol(ch);
    }

    return i;
}


uint8_t *utf8_to_utf32(uint8_t *utf8, uint32_t *utf32)
{
    uint8_t *u_utf8 = utf8;
    uint8_t b = *u_utf8++;
    uint32_t c, shift;
    int len = 0;

    // save ASC symbol as is
    if(!(b & 0x80))
    {
       *utf32 = b;
        return utf8 + 1;
    }

    // calculate lenght
    while(b & 0x80)
    {
        b <<= 1;
        ++len;
    }

    c = b;
    shift = 6 - len;

    while(--len)
    {
        c <<= shift;
        c |= (*u_utf8++) & 0x3f;
        shift = 6;
    }

   *utf32 = c;
    return u_utf8;
}


uint32_t utf32_to_utf8(uint8_t utf8[6], uint32_t utf32)
{
    uint32_t utf8_len = 0;
    if((utf32 & (~0x0000007F)) == 0)
    {
        utf8[0] = 0x0000007F & utf32;
        utf8_len = 1;
    }
    else if((utf32 & (~0x000007FF)) == 0)
    {
        utf8[1] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[0] = (0x0000003F & utf32) | 0xC0;
        utf8_len = 2;
    }
    else if((utf32 & (~0x0000FFFF)) == 0)
    {
        utf8[2] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[1] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[0] = (0x0000003F & utf32) | 0xE0;
        utf8_len = 3;
    }
    else if((utf32 & (~0x001FFFFF)) == 0)
    {
        utf8[3] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[2] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[1] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[0] = (0x0000003F & utf32) | 0xF0;
        utf8_len = 4;
    }
    else if((utf32 & (~0x03FFFFFF)) == 0)
    {
        utf8[4] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[3] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[2] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[1] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[0] = (0x0000003F & utf32) | 0xF8;
        utf8_len = 5;
    }
    else if((utf32 & (~0x7FFFFFFF)) == 0)
    {
        utf8[5] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[4] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[3] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[2] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[1] = (0x0000003F & utf32) | 0x80;
        utf32 >>= 6;
        utf8[0] = (0x0000003F & utf32) | 0xFC;
        utf8_len = 6;
    }
    
    return utf8_len;
}


void     utf8_delete_char(uint8_t *utf8, uint32_t pos)
{
    uint32_t i = 0;
    uint8_t *ch = utf8;

    for(; *ch && i < pos; i++)
    {
        ch = utf8_next_symbol(ch);
    }

    if((i == pos) && *ch)
    {
        int len = 0;
        uint8_t b = *ch;
        uint8_t *next_ch;
        while(b & 0x80)
        {
            b <<= 1;
            ++len;
        }
        len = (len == 0) ? (1) : (len);

        for(next_ch = ch + len; *next_ch; ch++, next_ch++)
        {
            *ch = *next_ch;
        }
        *ch = 0;
    }
}


void     utf8_insert_char(uint8_t *utf8, uint32_t utf32, uint32_t pos, uint32_t size)
{
    uint32_t i = 0;
    uint8_t *ch = utf8;

    for(; *ch && i < pos; i++)
    {
        ch = utf8_next_symbol(ch);
    }

    if(i == pos)
    {
        uint8_t new_char[8] = { 0 };
        uint32_t utf8_len = utf32_to_utf8(new_char, utf32);
        uint32_t old_len = strlen((const char*)utf8);
        if((utf8_len > 0) && (old_len + utf8_len < size))
        {
            uint8_t *tch = utf8 + old_len;
            for(; tch != ch; tch--)
            {
                *(tch + utf8_len) = *tch;
            }
            *(tch + utf8_len) = *tch;
            for(; utf8_len; utf8_len--)
            {
                ch[utf8_len-1] = new_char[utf8_len-1];
            }
        }
    }
}