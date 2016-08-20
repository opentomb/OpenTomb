
#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define IMAGE_FORMAT_PCX    (1)
#define IMAGE_FORMAT_PNG    (2)

/**
 * supported formats: PNG, PCX;
 * supported pixel formats: RGB, RGBA, so bpp can be 24 or 32;
 * Image_Load: allocates memory inside by *buffer = (uint8_t*)malloc(...);
 * w, h, bpp are outputs;
 * return 0 on fail and 1 if success;
 */
int Image_Load(const char *file_name, int format, uint8_t **buffer, uint32_t *w, uint32_t *h, uint32_t *bpp); 

/**
 * supported formats: PNG;
 * supported pixel formats: RGB, RGBA, so bpp can be 24 or 32;
 * return 0 on fail and 1 if success;
 */
int Image_Save(const char *file_name, int format, uint8_t *buffer, uint32_t w, uint32_t h, uint32_t bpp);

#ifdef	__cplusplus
}
#endif

#endif
