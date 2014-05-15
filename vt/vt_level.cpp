#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include <stdio.h>
#include "vt_level.h"

#define RCSID "$Id: vt_level.cpp,v 1.1 2002/09/20 15:59:02 crow Exp $"

void VT_Level::prepare_level()
{
	uint32_t i;

	if ((game_version >= TR_II) && (game_version <= TR_V))
        {
		if (!read_32bit_textiles)
                {
			if (textile32_count == 0)
                        {
                                this->textile32_count = this->num_textiles;
                                this->textile32 = (tr4_textile32_t*)malloc(this->textile32_count * sizeof(tr4_textile32_t));
                        }
			for (i = 0; i < (num_textiles - num_misc_textiles); i++)
				convert_textile16_to_textile32(textile16[i], textile32[i]);
		}
	}
        else
        {
		this->textile32_count = this->num_textiles;
                this->textile32 = (tr4_textile32_t*)malloc(this->textile32_count * sizeof(tr4_textile32_t));
		for (i = 0; i < num_textiles; i++)
			convert_textile8_to_textile32(textile8[i], palette, textile32[i]);
	}
}

tr_staticmesh_t *VT_Level::find_staticmesh_id(uint32_t object_id)
{
	uint32_t i;

	for (i = 0; i < static_meshes_count; i++)
		if (static_meshes[i].object_id == object_id)
			return &static_meshes[i];

	return NULL;
}

tr2_item_t *VT_Level::find_item_id(int32_t object_id)
{
	uint32_t i;

	for (i = 0; i < items_count; i++)
		if (items[i].object_id == object_id)
			return &items[i];

	return NULL;
}

tr_moveable_t *VT_Level::find_moveable_id(uint32_t object_id)
{
	uint32_t i;

	for (i = 0; i < moveables_count; i++)
		if (moveables[i].object_id == object_id)
			return &moveables[i];

	return NULL;
}

void VT_Level::convert_textile8_to_textile32(tr_textile8_t & tex, tr2_palette_t & pal, tr4_textile32_t & dst)
{
	int x, y;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			int col = tex.pixels[y][x];

			if (col > 0)
				dst.pixels[y][x] = ((int)pal.colour[col].r) | ((int)pal.colour[col].g << 8) | ((int)pal.colour[col].b << 16) | (0xff << 24);
			else
				dst.pixels[y][x] = 0x00000000;
		}
	}
}

void VT_Level::convert_textile16_to_textile32(tr2_textile16_t & tex, tr4_textile32_t & dst)
{
	int x, y;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			int col = tex.pixels[y][x];

			if (col & 0x8000)
				dst.pixels[y][x] = ((col & 0x00007c00) >> 7) | (((col & 0x000003e0) >> 2) << 8) | (((col & 0x0000001f) << 3) << 16) | 0xff000000;
			else
				dst.pixels[y][x] = 0x00000000;
		}
	}
}

void WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY)
{
	unsigned char c;
	unsigned short s;
	int x, y;
	FILE *st;

	st = fopen(filename, "wb");
	if (st == NULL)
		return;

	// write the header
	// id_length
	c = 0;
	fwrite(&c, sizeof(c), 1, st);
	// colormap_type
	c = 0;
	fwrite(&c, sizeof(c), 1, st);
	// image_type
	c = 2;
	fwrite(&c, sizeof(c), 1, st);
	// colormap_index
	s = 0;
	fwrite(&s, sizeof(s), 1, st);
	// colormap_length
	s = 0;
	fwrite(&s, sizeof(s), 1, st);
	// colormap_size
	c = 0;
	fwrite(&c, sizeof(c), 1, st);
	// x_origin
	s = 0;
	fwrite(&s, sizeof(s), 1, st);
	// y_origin
	s = 0;
	fwrite(&s, sizeof(s), 1, st);
	// width
	s = SDL_SwapLE16(width);
	fwrite(&s, sizeof(s), 1, st);
	// height
	s = SDL_SwapLE16(height);
	fwrite(&s, sizeof(s), 1, st);
	// bits_per_pixel
	c = 32;
	fwrite(&c, sizeof(c), 1, st);
	// attributes
	c = 0;
	fwrite(&c, sizeof(c), 1, st);

        if(invY)
        {
            for (y = 0; y < height; y++)
                for (x = 0; x < width; x++)
                {
                        fwrite(&data[(y * width + x) * 4 + 2], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 1], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 0], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 3], sizeof(uint8_t), 1, st);
                }
        }
        else
        {
            for (y = height-1; y >= 0; y--)
                for (x = 0; x < width; x++)
                {
                        fwrite(&data[(y * width + x) * 4 + 2], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 1], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 0], sizeof(uint8_t), 1, st);
                        fwrite(&data[(y * width + x) * 4 + 3], sizeof(uint8_t), 1, st);
                }
        }
	fclose(st);
}

void VT_Level::dump_textures()
{
        uint32_t i;
        char buffer[1024];

        for (i = 0; i < num_textiles; i++) {
                snprintf(buffer, 1024, "dump/%03i_32.tga", i);
                WriteTGAfile(buffer, (uint8_t *)&textile32[i].pixels, 256, 256, 0);
        }
}

