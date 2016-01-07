//
//  bordered_texture_atlas.cpp
//  tr1engine
//
//  Created by Torsten Kammer on 16.07.13.
//  Copyright (c) 2013 Torsten Kammer. All rights reserved.
//

#include "bordered_texture_atlas.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/gl_util.h"
#include "../core/polygon.h"
#include "bsp_tree_2d.h"
#include "../vt/vt_level.h"

#ifndef __APPLE__
/*!
 * Fills an area of memory with a four-byte pattern pointed to.
 * This is a standard library function on Mac OS X, but sadly not anywhere else, so I'm redefining it here. Because I know where it will be called, I can add additional requirements: len must be a multiple of four, and pattern and b both must be four-byte aligned.
 */
static void memset_pattern4(void *b, const void *pattern, const size_t len)
{
    uint32_t *intb = (uint32_t *) b;
    uint32_t patternValue = *((uint32_t *) pattern);
    for (size_t i = 0; i < len/4; i++)
        intb[i] = patternValue;
}
#endif

static __inline GLuint NextPowerOf2(GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}

#define ARRAY_CAPACITY_INCREASE_STEP (32)
#define WHITE_TEXTURE_INDEX          (0x8000)

/*!
 * The bordered texture atlas used by the borderedTextureAtlas_CompareCanonicalTextureSizes function. Sadly, qsort does not allow passing this context through as a parameter, and the nonstandard extensions qsort_r/qsort_s which do are not supported on MinGW, so this has to be done as a global variable.
 */
static bordered_texture_atlas *compare_context = 0;

/*!
 * Compare function for qsort. It interprets the the parameters as pointers to indices into the canonical object textures of the atlas currently stored in compare_context. It returns -1, 0 or 1 if the first texture is logically ordered before, the same or after the second texture.
 *
 * A texture comes before another texture if it is higher. If both have the same height, the wider texture comes first. If both have the same height and width, they are ordered the same.
 */
int bordered_texture_atlas::compareCanonicalTextureSizes(const void *parameter1, const void *parameter2)
{
    unsigned long index1 = *((const unsigned long *) parameter1);
    unsigned long index2 = *((const unsigned long *) parameter2);

    const canonical_object_texture &texture1 = compare_context->canonical_object_textures[index1];
    const canonical_object_texture &texture2 = compare_context->canonical_object_textures[index2];

    // First order by height. qsort brings "lower" values to the front, so treat greater height as lower.
    if (texture1.height > texture2.height)
        return -1;
    else if (texture1.height < texture2.height)
        return 1;

    // Then order by width
    if (texture1.width > texture2.width)
        return -1;
    else if (texture1.width < texture2.width)
        return 1;

    // If they have the same height and width then their order does not matter.
    return 0;
}

/*!
 * Lays out the texture data and switches the atlas to laid out mode. This makes
 * use of a bsp_tree_2d to handle all the really annoying stuff.
 */
void bordered_texture_atlas::layOutTextures()
{
    // First step: Sort the canonical textures by size.
    unsigned long *sorted_indices = new unsigned long[number_canonical_object_textures];
    for (unsigned long i = 0; i < number_canonical_object_textures; i++)
        sorted_indices[i] = i;

    compare_context = this;
    qsort(sorted_indices, number_canonical_object_textures, sizeof(sorted_indices[0]), compareCanonicalTextureSizes);
    compare_context = NULL;

    // Find positions for the canonical textures
    number_result_pages = 0;
    result_page_height = NULL;
    bsp_tree_2d_p *result_pages = NULL;

    for (unsigned long texture = 0; texture < number_canonical_object_textures; texture++)
    {
        struct canonical_object_texture &canonical = canonical_object_textures[sorted_indices[texture]];

        // Try to find space in an existing page.
        bool found_place = 0;
        for (unsigned long page = 0; page < number_result_pages; page++)
        {
            found_place = BSPTree2D_FindSpaceFor(result_pages[page],
                                                 canonical.width + 2*border_width,
                                                 canonical.height + 2*border_width,
                                                 &(canonical.new_x_with_border),
                                                 &(canonical.new_y_with_border));
            if (found_place)
            {
                canonical.new_page = page;

                unsigned highest_y = canonical.new_y_with_border + canonical.height + border_width * 2;
                if (highest_y + 1 > result_page_height[page])
                    result_page_height[page] = highest_y;

                break;
            }
        }

        // No existing page has enough remaining space so open new one.
        if (!found_place)
        {
            number_result_pages += 1;
            result_pages = (bsp_tree_2d_p *) realloc(result_pages, sizeof(bsp_tree_2d_p) * number_result_pages);
            result_pages[number_result_pages - 1] = BSPTree2D_Create(result_page_width, result_page_width);
            result_page_height = (unsigned *) realloc(result_page_height, sizeof(unsigned) * number_result_pages);

            BSPTree2D_FindSpaceFor(result_pages[number_result_pages - 1],
                                   canonical.width + 2*border_width,
                                   canonical.height + 2*border_width,
                                   &(canonical.new_x_with_border),
                                   &(canonical.new_y_with_border));
            canonical.new_page = number_result_pages - 1;

            unsigned highest_y = canonical.new_y_with_border + canonical.height + border_width * 2;
            result_page_height[number_result_pages - 1] = highest_y;
        }
    }

    // Fix up heights if necessary
    for (unsigned page = 0; page < number_result_pages; page++)
    {
        result_page_height[page] = NextPowerOf2(result_page_height[page]);
    }

    // Cleanup
    delete [] sorted_indices;
    for (unsigned long i = 0; i < number_result_pages; i++)
        BSPTree2D_Destroy(result_pages[i]);
    free(result_pages);
}

bordered_texture_atlas::bordered_texture_atlas(int border,
                                               size_t page_count,
                                               const tr4_textile32_t *pages,
                                               size_t object_texture_count,
                                               const tr4_object_texture_t *object_textures,
                                               size_t sprite_texture_count,
                                               const tr_sprite_texture_t *sprite_textures)
: border_width(border),
number_result_pages(0),
result_page_width(0),
result_page_height(NULL),
number_original_pages(page_count),
original_pages(pages),
number_file_object_textures(0),
file_object_textures(NULL),
number_sprite_textures(0),
canonical_textures_for_sprite_textures(NULL),
number_canonical_object_textures(0),
canonical_object_textures(NULL),
textures_indexes(NULL)
{
    GLint max_texture_edge_length = 0;
    qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_edge_length);
    if (max_texture_edge_length > 4096)
        max_texture_edge_length = 4096; // That is already 64 MB and covers up to 256 pages.
    result_page_width = max_texture_edge_length;

    size_t maxNumberCanonicalTextures = object_texture_count + sprite_texture_count + 1;
    canonical_object_textures = new canonical_object_texture[maxNumberCanonicalTextures];

    number_canonical_object_textures = 1;
    canonical_object_texture &canonical = canonical_object_textures[0];
    canonical.width = 8;
    canonical.height = 8;
    canonical.original_page = WHITE_TEXTURE_INDEX;
    canonical.original_x = 0;
    canonical.original_y = 0;

    file_object_textures = new file_object_texture[object_texture_count];
    for (size_t i = 0; i < object_texture_count; i++)
    {
        addObjectTexture(object_textures[i]);
    }

    canonical_textures_for_sprite_textures = new unsigned long[sprite_texture_count];
    for (size_t i = 0; i < sprite_texture_count; i++)
    {
        addSpriteTexture(sprite_textures[i]);
    }

    layOutTextures();
}

bordered_texture_atlas::~bordered_texture_atlas()
{
    delete [] file_object_textures;
    delete [] canonical_textures_for_sprite_textures;
    delete [] canonical_object_textures;
    original_pages = NULL;
    free(result_page_height);
}

void bordered_texture_atlas::addObjectTexture(const tr4_object_texture_t &texture)
{
    // Determine the canonical texture for this texture.
    // Use only first three vertices to find min, max, because for triangles the last will be 0,0 with no other marker that this is a triangle. As long as all textures are axis-aligned rectangles, this will always return the right result anyway.
    uint8_t max[2] = { 0, 0 }, min[2] = { 255, 255 };
    for (int i = 0; i < 3; i++)
    {
        if (texture.vertices[i].xpixel > max[0])
            max[0] = texture.vertices[i].xpixel;
        if (texture.vertices[i].ypixel > max[1])
            max[1] = texture.vertices[i].ypixel;
        if (texture.vertices[i].xpixel < min[0])
            min[0] = texture.vertices[i].xpixel;
        if (texture.vertices[i].ypixel < min[1])
            min[1] = texture.vertices[i].ypixel;
    }
    uint8_t width = max[0] - min[0];
    uint8_t height = max[1] - min[1];

    // See whether it already exists
    long canonical_index = -1;
    for (unsigned long i = 0; i < number_canonical_object_textures; i++)
    {
        canonical_object_texture *canonical_candidate = &(canonical_object_textures[i]);

        if (canonical_candidate->original_page == (texture.tile_and_flag & TR_TEXTURE_INDEX_MASK_TR4)
            && canonical_candidate->original_x == min[0]
            && canonical_candidate->original_y == min[1]
            && canonical_candidate->width == width
            && canonical_candidate->height == height)
        {
            canonical_index = i;
            break;
        }
    }

    // Create it if not.
    if (canonical_index < 0)
    {
        canonical_index = number_canonical_object_textures;
        number_canonical_object_textures += 1;

        canonical_object_texture &canonical = canonical_object_textures[canonical_index];
        canonical.width = width;
        canonical.height = height;
        canonical.original_page = texture.tile_and_flag & TR_TEXTURE_INDEX_MASK_TR4;
        canonical.original_x = min[0];
        canonical.original_y = min[1];
    }

    // Create file object texture.
    file_object_texture &file_object_texture = file_object_textures[number_file_object_textures];
    number_file_object_textures += 1;

    file_object_texture.canonical_texture_index = canonical_index;
    for (int i = 0; i < 4; i++)
    {
        if (texture.vertices[i].xpixel == min[0])
        {
            if (texture.vertices[i].ypixel == min[1])
                file_object_texture.corner_locations[i] = TOP_LEFT;
            else
                file_object_texture.corner_locations[i] = BOTTOM_LEFT;
        }
        else
        {
            if (texture.vertices[i].ypixel == min[1])
                file_object_texture.corner_locations[i] = TOP_RIGHT;
            else
                file_object_texture.corner_locations[i] = BOTTOM_RIGHT;
        }
    }
}

void bordered_texture_atlas::addSpriteTexture(const tr_sprite_texture_t &texture)
{
    // Determine the canonical texture for this texture.
    unsigned x = texture.x0;
    unsigned y = texture.y0;
    unsigned width = texture.x1 - texture.x0;
    unsigned height = texture.y1 - texture.y0;

    // See whether it already exists
    long canonical_index = -1;
    for (unsigned long i = 0; i < number_canonical_object_textures; i++)
    {
        canonical_object_texture *canonical_candidate = &(canonical_object_textures[i]);

        if (canonical_candidate->original_page == (texture.tile & TR_TEXTURE_INDEX_MASK_TR4)
            && canonical_candidate->original_x == x
            && canonical_candidate->original_y == y
            && canonical_candidate->width == width
            && canonical_candidate->height == height)
        {
            canonical_index = i;
            break;
        }
    }

    // Create it if not.
    if (canonical_index < 0)
    {
        canonical_index = number_canonical_object_textures;
        number_canonical_object_textures += 1;

        canonical_object_texture &canonical = canonical_object_textures[canonical_index];
        canonical.width = width;
        canonical.height = height;
        canonical.original_page = texture.tile & TR_TEXTURE_INDEX_MASK_TR4;
        canonical.original_x = x;
        canonical.original_y = y;
    }

    // Create sprite texture assignmen.
    canonical_textures_for_sprite_textures[number_sprite_textures] = canonical_index;
    number_sprite_textures += 1;
}

unsigned long bordered_texture_atlas::getCanonicalTextureHeight(unsigned long texture) const
{
    assert(texture < number_file_object_textures);

    const file_object_texture &file_object_texture = file_object_textures[texture];
    const canonical_object_texture &canonical = canonical_object_textures[file_object_texture.canonical_texture_index];

    return canonical.height;
}

float bordered_texture_atlas::getTextureHeight(unsigned long texture) const
{
    assert(texture < number_file_object_textures);

    const file_object_texture &file_object_texture = file_object_textures[texture];
    const canonical_object_texture &canonical = canonical_object_textures[file_object_texture.canonical_texture_index];

    return (GLfloat)canonical.height / (GLfloat)(result_page_height[canonical.new_page]);
}

void bordered_texture_atlas::getWhiteTextureCoordinates(polygon_p poly)
{
    const canonical_object_texture &canonical = canonical_object_textures[0];
    poly->texture_index = textures_indexes[canonical.new_page];
    for (unsigned long i = 0; i < poly->vertex_count; i++)
    {
        unsigned x_coord;
        unsigned y_coord;

        switch (file_object_textures->corner_locations[i])
        {
            case TOP_LEFT:
                x_coord = canonical.new_x_with_border + border_width;
                y_coord = canonical.new_y_with_border + border_width;
                break;
            case TOP_RIGHT:
                x_coord = canonical.new_x_with_border + border_width + canonical.width;
                y_coord = canonical.new_y_with_border + border_width;
                break;
            case BOTTOM_LEFT:
                x_coord = canonical.new_x_with_border + border_width;
                y_coord = canonical.new_y_with_border + border_width + canonical.height;
                break;
            case BOTTOM_RIGHT:
                x_coord = canonical.new_x_with_border + border_width + canonical.width;
                y_coord = canonical.new_y_with_border + border_width + canonical.height;
                break;
            default:
                assert(0);
        }

        poly->vertices[i].tex_coord[0] = (GLfloat) x_coord / (GLfloat) result_page_width;
        poly->vertices[i].tex_coord[1] = (GLfloat) y_coord / (GLfloat) result_page_height[canonical.new_page];
    }
}

void bordered_texture_atlas::getCoordinates(polygon_p poly,
                                         unsigned long texture,
                                         bool reverse,
                                         signed shift,
                                         bool split)  const
{
    assert(poly->vertex_count <= 4);

    assert(texture < number_file_object_textures);
    const file_object_texture &file_object_texture = file_object_textures[texture];
    const canonical_object_texture &canonical = canonical_object_textures[file_object_texture.canonical_texture_index];

    poly->texture_index = textures_indexes[canonical.new_page];
    for (unsigned long i = 0; i < poly->vertex_count; i++)
    {
        unsigned x_coord;
        unsigned y_coord;

        switch (file_object_texture.corner_locations[i])
        {
            case TOP_LEFT:
                x_coord = canonical.new_x_with_border + border_width;
                y_coord = canonical.new_y_with_border + border_width - shift;

                if(split)
                {
                    y_coord += (canonical.height / 2);
                }
                break;
            case TOP_RIGHT:
                x_coord = canonical.new_x_with_border + border_width + canonical.width;
                y_coord = canonical.new_y_with_border + border_width - shift;

                if(split)
                {
                    y_coord += (canonical.height / 2);
                }
                break;
            case BOTTOM_LEFT:
                x_coord = canonical.new_x_with_border + border_width;
                y_coord = canonical.new_y_with_border + border_width + canonical.height - shift;
                break;
            case BOTTOM_RIGHT:
                x_coord = canonical.new_x_with_border + border_width + canonical.width;
                y_coord = canonical.new_y_with_border + border_width + canonical.height - shift;
                break;
            default:
                assert(0);
        }

        unsigned long index = reverse ? (poly->vertex_count - i-1) : i;

        poly->vertices[index].tex_coord[0] = (GLfloat) x_coord / (GLfloat) result_page_width;
        poly->vertices[index].tex_coord[1] = (GLfloat) y_coord / (GLfloat) result_page_height[canonical.new_page];
    }
}

void bordered_texture_atlas::getSpriteCoordinates(GLfloat *coordinates, unsigned long sprite_texture, uint32_t *outPage) const
{
    assert(sprite_texture < number_sprite_textures);

    unsigned long canonical_index = canonical_textures_for_sprite_textures[sprite_texture];
    const canonical_object_texture &canonical = canonical_object_textures[canonical_index];

    *outPage = textures_indexes[canonical.new_page];

    unsigned pixel_coordinates[8] = {
        // top right
        canonical.new_x_with_border + border_width + canonical.width,
        canonical.new_y_with_border + border_width + canonical.height,

        // top left
        canonical.new_x_with_border + border_width,
        canonical.new_y_with_border + border_width + canonical.height,

        // bottom left
        canonical.new_x_with_border + border_width,
        canonical.new_y_with_border + border_width,

        // bottom right
        canonical.new_x_with_border + border_width + canonical.width,
        canonical.new_y_with_border + border_width,
    };

    for (int i = 0; i < 4; i++) {
        coordinates[i*2 + 0] = (GLfloat) pixel_coordinates[i*2 + 0] / (GLfloat) result_page_width;
        coordinates[i*2 + 1] = (GLfloat) pixel_coordinates[i*2 + 1] / (GLfloat) result_page_height[canonical.new_page];
    }
}

unsigned long bordered_texture_atlas::getNumAtlasPages() const
{
    return number_result_pages;
}

void bordered_texture_atlas::createTextures(GLuint *textureNames)
{
    GLubyte *data = (GLubyte *) malloc(4 * result_page_width * result_page_width);

    qglGenTextures((GLsizei) number_result_pages, textureNames);

    textures_indexes = textureNames;

    for (unsigned long page = 0; page < number_result_pages; page++)
    {
        for (unsigned long texture = 0; texture < number_canonical_object_textures; texture++)
        {
            const canonical_object_texture &canonical = canonical_object_textures[texture];
            if (canonical.new_page != page)
                continue;

            if(canonical.original_page == WHITE_TEXTURE_INDEX)
            {
                uint32_t white_pixels[1] = {0xFFFFFFFFU};
                // Add top border
                for (int border = 0; border < border_width; border++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + border;

                    // expand top-left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           white_pixels, 4 * border_width);
                    // copy top line
                    memset_pattern4(&data[(y*result_page_width + x + border_width) * 4],
                           white_pixels, canonical.width * 4);
                    // expand top-right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           white_pixels, 4 * border_width);
                }

                // Copy main content
                for (int line = 0; line < canonical.height; line++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + border_width + line;

                    // expand left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           white_pixels, 4 * border_width);
                    // copy line
                    memset_pattern4(&data[(y*result_page_width + x + border_width) * 4],
                           white_pixels, canonical.width * 4);
                    // expand right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           white_pixels, 4 * border_width);
                }

                // Add bottom border
                for (int border = 0; border < border_width; border++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + canonical.height + border_width + border;

                    // expand bottom-left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           white_pixels, 4 * border_width);
                    // copy bottom line
                    memset_pattern4(&data[(y*result_page_width + x + border_width) * 4],
                           white_pixels, canonical.width * 4);
                    // expand bottom-right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           white_pixels, 4 * border_width);
                }
            }
            else
            {
                const char *original = (char *) original_pages[canonical.original_page].pixels;
                // Add top border
                for (int border = 0; border < border_width; border++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + border;
                    unsigned old_x = canonical.original_x;
                    unsigned old_y = canonical.original_y;

                    // expand top-left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           &(original[(old_y * 256 + old_x) * 4]),
                           4 * border_width);
                    // copy top line
                    memcpy(&data[(y*result_page_width + x + border_width) * 4],
                           &original[(old_y * 256 + old_x) * 4],
                           canonical.width * 4);
                    // expand top-right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           &(original[(old_y * 256 + old_x + canonical.width) * 4]),
                           4 * border_width);
                }

                // Copy main content
                for (int line = 0; line < canonical.height; line++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + border_width + line;
                    unsigned old_x = canonical.original_x;
                    unsigned old_y = canonical.original_y + line;

                    // expand left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           &(original[(old_y * 256 + old_x) * 4]),
                           4 * border_width);
                    // copy line
                    memcpy(&data[(y*result_page_width + x + border_width) * 4],
                           &original[(old_y * 256 + old_x) * 4],
                           canonical.width * 4);
                    // expand right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           &(original[(old_y * 256 + old_x + canonical.width) * 4]),
                           4 * border_width);
                }

                // Add bottom border
                for (int border = 0; border < border_width; border++)
                {
                    unsigned x = canonical.new_x_with_border;
                    unsigned y = canonical.new_y_with_border + canonical.height + border_width + border;
                    unsigned old_x = canonical.original_x;
                    unsigned old_y = canonical.original_y + canonical.height;

                    // expand bottom-left pixel
                    memset_pattern4(&data[(y*result_page_width + x) * 4],
                           &(original[(old_y * 256 + old_x) * 4]),
                           4 * border_width);
                    // copy bottom line
                    memcpy(&data[(y*result_page_width + x + border_width) * 4],
                           &original[(old_y * 256 + old_x) * 4],
                           canonical.width * 4);
                    // expand bottom-right pixel
                    memset_pattern4(&data[(y*result_page_width + x + border_width + canonical.width) * 4],
                           &(original[(old_y * 256 + old_x + canonical.width) * 4]),
                           4 * border_width);
                }
            }
        }

        qglBindTexture(GL_TEXTURE_2D, textureNames[page]);
        qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)result_page_width, (GLsizei) result_page_height[page], 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        if(qglGenerateMipmap != NULL)
        {
            qglGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            int mip_level = 1;
            int w = result_page_width / 2;
            int h = result_page_height[page] / 2;
            GLubyte *mip_data = (GLubyte *) malloc(4 * w * h);

            assert(w > 0 && h > 0);
            for(int i = 0; i < h; i++)
            {
                for(int j = 0; j < w; j++)
                {
                    mip_data[i * w * 4 + j * 4 + 0] = 0.25 * ((int)data[i * w * 16 + j * 8 + 0] + (int)data[i * w * 16 + j * 8 + 4 + 0] + (int)data[i * w * 16 + w * 8 + j * 8 + 0] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 0]);
                    mip_data[i * w * 4 + j * 4 + 1] = 0.25 * ((int)data[i * w * 16 + j * 8 + 1] + (int)data[i * w * 16 + j * 8 + 4 + 1] + (int)data[i * w * 16 + w * 8 + j * 8 + 1] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 1]);
                    mip_data[i * w * 4 + j * 4 + 2] = 0.25 * ((int)data[i * w * 16 + j * 8 + 2] + (int)data[i * w * 16 + j * 8 + 4 + 2] + (int)data[i * w * 16 + w * 8 + j * 8 + 2] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 2]);
                    mip_data[i * w * 4 + j * 4 + 3] = 0.25 * ((int)data[i * w * 16 + j * 8 + 3] + (int)data[i * w * 16 + j * 8 + 4 + 3] + (int)data[i * w * 16 + w * 8 + j * 8 + 3] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 3]);
                }
            }

            //char tgan[128];
            //WriteTGAfile("mip_00.tga", data, result_page_width, result_page_height[page], 0);
            //sprintf(tgan, "mip_%0.2d.tga", mip_level);
            //WriteTGAfile(tgan, mip_data, w, h, 0);
            qglTexImage2D(GL_TEXTURE_2D, mip_level, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip_data);

            while((w > 1) && (h > 1) /*&& (mip_level < 4)*/)
            {
                mip_level++;
                w /= 2; w = (w==0)?1:w;
                h /= 2; h = (h==0)?1:h;
                for(int i = 0; i < h; i++)
                {
                    for(int j = 0; j < w; j++)
                    {
                        mip_data[i * w * 4 + j * 4 + 0] = 0.25 * ((int)mip_data[i * w * 16 + j * 8 + 0] + (int)mip_data[i * w * 16 + j * 8 + 4 + 0] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 0] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 0]);
                        mip_data[i * w * 4 + j * 4 + 1] = 0.25 * ((int)mip_data[i * w * 16 + j * 8 + 1] + (int)mip_data[i * w * 16 + j * 8 + 4 + 1] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 1] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 1]);
                        mip_data[i * w * 4 + j * 4 + 2] = 0.25 * ((int)mip_data[i * w * 16 + j * 8 + 2] + (int)mip_data[i * w * 16 + j * 8 + 4 + 2] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 2] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 2]);
                        mip_data[i * w * 4 + j * 4 + 3] = 0.25 * ((int)mip_data[i * w * 16 + j * 8 + 3] + (int)mip_data[i * w * 16 + j * 8 + 4 + 3] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 3] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 3]);
                    }
                }
                //sprintf(tgan, "mip_%0.2d.tga", mip_level);
                //WriteTGAfile(tgan, mip_data, w, h, 0);
                qglTexImage2D(GL_TEXTURE_2D, mip_level, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip_data);
            }
            free(mip_data);
        }
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    free(data);
}
