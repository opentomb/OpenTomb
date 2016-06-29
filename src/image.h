
#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define IMAGE_FORMAT_PCX    (1)
#define IMAGE_FORMAT_PNG    (2)


int Image_Load(const char *file_name, int format, uint8_t **buffer, uint32_t *w, uint32_t *h, uint32_t *bpp); 
int Image_Save(const char *file_name, int format, uint8_t *buffer, uint32_t w, uint32_t h, uint32_t bpp);

#ifdef	__cplusplus
}
#endif

#endif
