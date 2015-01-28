#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "string.h"
#include "system.h"

uint32_t* String_MakeUTF32(const char* utf8_string, uint32_t* string_length)
{
    uint32_t  i;
    int32_t   cur_size   = 0;
    uint32_t* new_string     = NULL;
    uint32_t* string_pointer = NULL;
    
    char* utf8_string_ = (char*) utf8_string;
    
    for(i = 0; i < *string_length; i++)
    {
        if(cur_size == i)    // Preload memory for decoding.
        {
            cur_size += STR_CONV_PRELOAD_SIZE;
            new_string = (uint32_t*)realloc(new_string, sizeof(uint32_t) * cur_size);
            string_pointer = new_string + i;
        }
        
        utf8_string_ = String_UTF8char_to_UTF32char(utf8_string_, string_pointer);
        
        if(*string_pointer == 0) break;    // Null termination, string end.
        
        string_pointer++;
    }
    
    new_string = (uint32_t*)realloc(new_string, sizeof(uint32_t) * i);  // Pack string.
   *string_length = i;
    
    return new_string;
}


char* String_UTF8char_to_UTF32char(char* utf8, uint32_t* utf32)
{
    if(!utf8)
    {
        *utf32 = 0;
        return NULL;  // Foolproof
    }
    
    unsigned char* u_utf8 = (unsigned char*) utf8;
    unsigned char b = *u_utf8++;

    if (!(b & 0x80))
    {
        if (utf32)
            *utf32 = b;
        return utf8 + 1;
    }
    
    uint32_t len = 0;
    while (b & 0x80)
    {
        b <<= 1;
        ++len;
    }
    
    uint32_t c = b;
    uint32_t shift = 6 - len;

    while (--len)
    {
        c <<= shift;
        c |= (*u_utf8++) & 0x3f;
        shift = 6;
    }
    
    if (utf32)
        *utf32 = c;

    return (char*) u_utf8;
}
