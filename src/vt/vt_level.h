#pragma once

#include "l_main.h"

#define TR_TEXTURE_INDEX_MASK_TR4   (0x7FFF)          // in some custom levels we need to use 0x7FFF flag
#define TR_TEXTURE_INDEX_MASK       (0x0FFF)
//#define TR_TEXTURE_SHAPE_MASK     (0x7000)          // still not used
#define TR_TEXTURE_FLIPPED_MASK     (0x8000)

void WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);
