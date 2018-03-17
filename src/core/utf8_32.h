/*
 * File:   gl_font.h
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#ifndef UTF_8_32_H
#define	UTF_8_32_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t *utf8_next_symbol(uint8_t *utf8);
uint32_t utf8_strlen(const char *str);
uint8_t *utf8_to_utf32(uint8_t *utf8, uint32_t *utf32);
uint32_t utf32_to_utf8(uint8_t utf8[6], uint32_t utf32);
void     utf8_delete_char(uint8_t *utf8, uint32_t pos);
void     utf8_insert_char(uint8_t *utf8, uint32_t utf32, uint32_t pos, uint32_t size);


#ifdef	__cplusplus
}
#endif

#endif	/* UTF_8_32_H */

