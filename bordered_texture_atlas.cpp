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

#include "bsp_tree_2d.h"
#include "gl_util.h"
#include "polygon.h"
#include "vt/vt_level.h"

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

/*!
 * @abstract Identifies a corner.
 * @discussion This is used for mapping a corner in a file object texture to the corresponding corner in the canonical object texture.
 */
enum corner_location_e
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

/*!
 * An internal representation of a file texture. Note that this only stores a reference to the canonical texture and how the corners of the canonical texture map to this.
 */
struct file_object_texture_s
{
    unsigned long canonical_texture_index;
    enum corner_location_e corner_locations[4];
};

/*!
 * The canonical texture. In TR, a lot of textures can refer to the same rectangle of pixels, only in different winding orders. It is not practical to treat these as different textures, so they are all mapped to one canonical object texture. This structure consists of two parts: Describing the original location, and describing the new final location. The latter is only valid after the data in the texture atlas has been laid out.
 */
struct canonical_object_texture_s
{
    // The unadjusted size
    uint8_t width;
    uint8_t height;

    // Original origin
    uint16_t original_page;
    uint8_t original_x;
    uint8_t original_y;

    // New origin
    unsigned long new_page;
    unsigned new_x_with_border; // Where the adjusted data starts. The start of the actual data is this plus the atlas's border size.
    unsigned new_y_with_border; // See above.
};

/*!
 * @abstract The main structure for the texture atlas.
 * @discussion The fields are explained below. One important pattern: For all variable-length fields, there is a number and a capacity. Capacity means how long the array actually is. It gets increased in ARRAY_CAPACITY_INCREASE_STEP steps, to avoid calling realloc way too often, just to be a bit more clean about things.
 */
struct bordered_texture_atlas_s
{
    // How much border to add.
    int border_width;

    // Store whether the data has been laid out. Adding more object textures after that is illegal.
    int data_has_been_laid_out;

    // Whether non-power-of-two textures are supported.
    int supports_npot;

    // Result pages
    // Note: No capacity here, this is handled internally by the layout method. Also, all result pages have the same width, which will always be less than or equal to the height.
    unsigned long number_result_pages;
    unsigned result_page_width;
    unsigned *result_page_height;

    // Original data
    unsigned long number_original_pages;
    unsigned long capacity_original_pages;
    void **original_pages;

    // Object textures in the file.
    unsigned long number_file_object_textures;
    unsigned long capacity_file_object_textures;
    struct file_object_texture_s *file_object_textures;

    // Sprite texture in the file.
    // Note: No data is saved for them, they get mapped directly to canonical textures.
    unsigned long number_sprite_textures;
    unsigned long capacity_sprite_textures;
    unsigned long *canonical_textures_for_sprite_textures;

    // Canonical object textures
    unsigned long number_canonical_object_textures;
    unsigned long capacity_canonical_object_textures;
    struct canonical_object_texture_s *canonical_object_textures;
};

#define ARRAY_CAPACITY_INCREASE_STEP 32

/*!
 * The bordered texture atlas used by the borderedTextureAtlas_CompareCanonicalTextureSizes function. Sadly, qsort does not allow passing this context through as a parameter, and the nonstandard extensions qsort_r/qsort_s which do are not supported on MinGW, so this has to be done as a global variable.
 */
static bordered_texture_atlas_p compare_context = 0;

/*!
 * Compare function for qsort. It interprets the the parameters as pointers to indices into the canonical object textures of the atlas currently stored in compare_context. It returns -1, 0 or 1 if the first texture is logically ordered before, the same or after the second texture.
 *
 * A texture comes before another texture if it is higher. If both have the same height, the wider texture comes first. If both have the same height and width, they are ordered the same.
 */
static int borderedTextureAtlas_CompareCanonicalTextureSizes(const void *parameter1, const void *parameter2)
{
    unsigned long index1 = *((const unsigned long *) parameter1);
    unsigned long index2 = *((const unsigned long *) parameter2);

    const canonical_object_texture_s *texture1 = compare_context->canonical_object_textures + index1;
    const canonical_object_texture_s *texture2 = compare_context->canonical_object_textures + index2;

    // First order by height. qsort brings "lower" values to the front, so treat greater height as lower.
    if (texture1->height > texture2->height)
        return -1;
    else if (texture1->height < texture2->height)
        return 1;

    // Then order by width
    if (texture1->width > texture2->width)
        return -1;
    else if (texture1->width < texture2->width)
        return 1;

    // If they have the same height and width then their order does not matter.
    return 0;
}

/*!
 * Lays out the texture data and switches the atlas to laid out mode. This makes
 * use of a bsp_tree_2d to handle all the really annoying stuff.
 */
static void borderedTextureAtlas_LayOutTextures(bordered_texture_atlas_p atlas)
{
    // First step: Sort the canonical textures by size.
    unsigned long *sorted_indices = (unsigned long *) malloc(sizeof(unsigned long) * atlas->number_canonical_object_textures);
    for (unsigned long i = 0; i < atlas->number_canonical_object_textures; i++)
        sorted_indices[i] = i;

    compare_context = atlas;
    qsort(sorted_indices, atlas->number_canonical_object_textures, sizeof(sorted_indices[0]), borderedTextureAtlas_CompareCanonicalTextureSizes);
    compare_context = NULL;

    // Find positions for the canonical textures
    atlas->number_result_pages = 0;
    atlas->result_page_height = NULL;
    bsp_tree_2d_p *result_pages = NULL;

    for (unsigned long texture = 0; texture < atlas->number_canonical_object_textures; texture++)
    {
        struct canonical_object_texture_s *canonical = &(atlas->canonical_object_textures[sorted_indices[texture]]);

        // Try to find space in an existing page.
        int found_place = 0;
        for (int page = 0; page < atlas->number_result_pages; page++)
        {
            found_place = BSPTree2D_FindSpaceFor(result_pages[page],
                                                 canonical->width + 2*atlas->border_width,
                                                 canonical->height + 2*atlas->border_width,
                                                 &(canonical->new_x_with_border),
                                                 &(canonical->new_y_with_border));
            if (found_place)
            {
                canonical->new_page = page;

                unsigned highest_y = canonical->new_y_with_border + canonical->height + atlas->border_width * 2;
                if (highest_y + 1 > atlas->result_page_height[page])
                    atlas->result_page_height[page] = highest_y + 1;

                break;
            }
        }

        // No existing page has enough remaining space so open new one.
        if (!found_place)
        {
            atlas->number_result_pages += 1;
            result_pages = (bsp_tree_2d_p *) realloc(result_pages, sizeof(bsp_tree_2d_p) * atlas->number_result_pages);
            result_pages[atlas->number_result_pages - 1] = BSPTree2D_Create(atlas->result_page_width, atlas->result_page_width);
            atlas->result_page_height = (unsigned *) realloc(atlas->result_page_height, sizeof(unsigned) * atlas->number_result_pages);

            BSPTree2D_FindSpaceFor(result_pages[atlas->number_result_pages - 1],
                                   canonical->width + 2*atlas->border_width,
                                   canonical->height + 2*atlas->border_width,
                                   &(canonical->new_x_with_border),
                                   &(canonical->new_y_with_border));
            canonical->new_page = atlas->number_result_pages - 1;

            unsigned highest_y = canonical->new_y_with_border + canonical->height + atlas->border_width * 2;
            atlas->result_page_height[atlas->number_result_pages - 1] = highest_y + 1;
        }
    }

    // Fix up heights if necessary
    if (!atlas->supports_npot)
    {
        for (unsigned page = 0; page < atlas->number_result_pages; page++)
        {
            atlas->result_page_height[page] = pow(ceil(log2((double) atlas->result_page_height[page])), 2);
        }
    }

    // Cleanup
    free(sorted_indices);
    for (unsigned long i = 0; i < atlas->number_result_pages; i++)
        BSPTree2D_Destroy(result_pages[i]);
    free(result_pages);

    atlas->data_has_been_laid_out = 1;
}

// Leave everything at 0, except border width. Calls to realloc will fill it up.
bordered_texture_atlas_p BorderedTextureAtlas_Create(int border)
{
    bordered_texture_atlas_p atlas = (bordered_texture_atlas_p) calloc(1, sizeof(struct bordered_texture_atlas_s));

    atlas->border_width = border;

    GLint max_texture_edge_length = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_edge_length);
	if (max_texture_edge_length > 4096)
		max_texture_edge_length = 4096; // That is already 64 MB and covers up to 256 pages.
    atlas->result_page_width = max_texture_edge_length;
	atlas->supports_npot = IsGLExtensionSupported("GL_ARB_texture_non_power_of_two");


    return atlas;
}

void BorderedTextureAtlas_Destroy(bordered_texture_atlas_p atlas)
{
    free(atlas->original_pages);
    free(atlas->file_object_textures);
    free(atlas->canonical_object_textures);
    free(atlas->result_page_height);
    free(atlas);
}

void BorderedTextureAtlas_AddPage(bordered_texture_atlas_p atlas, void *trpage)
{
    assert(!atlas->data_has_been_laid_out);

    if (atlas->capacity_original_pages < atlas->number_original_pages + 1)
    {
        atlas->capacity_original_pages += ARRAY_CAPACITY_INCREASE_STEP;
        atlas->original_pages = (void **) realloc(atlas->original_pages, sizeof(atlas->original_pages[0]) * atlas->capacity_original_pages);
    }

    atlas->original_pages[atlas->number_original_pages] = trpage;
    atlas->number_original_pages += 1;
}

void BorderedTextureAtlas_AddObjectTexture(bordered_texture_atlas_p atlas, const tr4_object_texture_t *texture)
{
    assert(!atlas->data_has_been_laid_out);

    // Determine the canonical texture for this texture.
    // Use only first three vertices to find min, max, because for triangles the last will be 0,0 with no other marker that this is a triangle. As long as all textures are axis-aligned rectangles, this will always return the right result anyway.
    uint8_t max[2] = { 0, 0 }, min[2] = { 255, 255 };
    for (int i = 0; i < 3; i++)
    {
        if (texture->vertices[i].xpixel > max[0])
            max[0] = texture->vertices[i].xpixel;
        if (texture->vertices[i].ypixel > max[1])
            max[1] = texture->vertices[i].ypixel;
        if (texture->vertices[i].xpixel < min[0])
            min[0] = texture->vertices[i].xpixel;
        if (texture->vertices[i].ypixel < min[1])
            min[1] = texture->vertices[i].ypixel;
    }
    uint8_t width = max[0] - min[0];
    uint8_t height = max[1] - min[1];

    // See whether it already exists
    long canonical_index = -1;
    struct canonical_object_texture_s *canonical = NULL;
    for (long i = 0; i < atlas->number_canonical_object_textures; i++)
    {
        struct canonical_object_texture_s *canonical_candidate = &(atlas->canonical_object_textures[i]);

        if (canonical_candidate->original_page == (texture->tile_and_flag & TR_TEXTURE_INDEX_MASK)
            && canonical_candidate->original_x == min[0]
            && canonical_candidate->original_y == min[1]
            && canonical_candidate->width == width
            && canonical_candidate->height == height)
        {
            canonical_index = i;
            canonical = canonical_candidate;
            break;
        }
    }

    // Create it if not.
    if (canonical_index < 0)
    {
        if (atlas->capacity_canonical_object_textures < atlas->number_canonical_object_textures + 1)
        {
            atlas->capacity_canonical_object_textures += ARRAY_CAPACITY_INCREASE_STEP;
            atlas->canonical_object_textures = (struct canonical_object_texture_s *) realloc(atlas->canonical_object_textures, sizeof(atlas->canonical_object_textures[0]) * atlas->capacity_canonical_object_textures);
        }

        canonical_index = atlas->number_canonical_object_textures;
        atlas->number_canonical_object_textures += 1;

        canonical = atlas->canonical_object_textures + canonical_index;
        canonical->width = width;
        canonical->height = height;
        canonical->original_page = texture->tile_and_flag & TR_TEXTURE_INDEX_MASK;
        canonical->original_x = min[0];
        canonical->original_y = min[1];
    }

    // Create file object texture.
    if (atlas->capacity_file_object_textures < atlas->number_file_object_textures + 1)
    {
        atlas->capacity_file_object_textures += ARRAY_CAPACITY_INCREASE_STEP;
        atlas->file_object_textures = (struct file_object_texture_s *) realloc(atlas->file_object_textures, sizeof(atlas->file_object_textures[0]) * atlas->capacity_file_object_textures);
    }

    struct file_object_texture_s *file_object_texture = (struct file_object_texture_s *) atlas->file_object_textures + atlas->number_file_object_textures;
    atlas->number_file_object_textures += 1;

    file_object_texture->canonical_texture_index = canonical_index;
    for (int i = 0; i < 4; i++)
    {
        if (texture->vertices[i].xpixel == min[0])
        {
            if (texture->vertices[i].ypixel == min[1])
                file_object_texture->corner_locations[i] = TOP_LEFT;
            else
                file_object_texture->corner_locations[i] = BOTTOM_LEFT;
        }
        else
        {
            if (texture->vertices[i].ypixel == min[1])
                file_object_texture->corner_locations[i] = TOP_RIGHT;
            else
                file_object_texture->corner_locations[i] = BOTTOM_RIGHT;
        }
    }
}

void BorderedTextureAtlas_AddSpriteTexture(bordered_texture_atlas_p atlas,
                                           const tr_sprite_texture_t *texture)
{
    assert(!atlas->data_has_been_laid_out);

    // Determine the canonical texture for this texture.
    unsigned x = texture->x0;
    unsigned y = texture->y0;
    unsigned width = texture->x1 - texture->x0;
    unsigned height = texture->y1 - texture->y0;

    // See whether it already exists
    long canonical_index = -1;
    struct canonical_object_texture_s *canonical = NULL;
    for (long i = 0; i < atlas->number_canonical_object_textures; i++)
    {
        struct canonical_object_texture_s *canonical_candidate = &(atlas->canonical_object_textures[i]);

        if (canonical_candidate->original_page == (texture->tile & TR_TEXTURE_INDEX_MASK)
            && canonical_candidate->original_x == x
            && canonical_candidate->original_y == y
            && canonical_candidate->width == width
            && canonical_candidate->height == height)
        {
            canonical_index = i;
            canonical = canonical_candidate;
            break;
        }
    }

    // Create it if not.
    if (canonical_index < 0)
    {
        if (atlas->capacity_canonical_object_textures < atlas->number_canonical_object_textures + 1)
        {
            atlas->capacity_canonical_object_textures += ARRAY_CAPACITY_INCREASE_STEP;
            atlas->canonical_object_textures = (struct canonical_object_texture_s *) realloc(atlas->canonical_object_textures, sizeof(atlas->canonical_object_textures[0]) * atlas->capacity_canonical_object_textures);
        }

        canonical_index = atlas->number_canonical_object_textures;
        atlas->number_canonical_object_textures += 1;

        canonical = atlas->canonical_object_textures + canonical_index;
        canonical->width = width;
        canonical->height = height;
        canonical->original_page = texture->tile & TR_TEXTURE_INDEX_MASK;
        canonical->original_x = x;
        canonical->original_y = y;
    }

    // Create sprite texture assignmen.
    if (atlas->capacity_sprite_textures < atlas->number_sprite_textures + 1)
    {
        atlas->capacity_sprite_textures += ARRAY_CAPACITY_INCREASE_STEP;
        atlas->canonical_textures_for_sprite_textures = (unsigned long *) realloc(atlas->canonical_textures_for_sprite_textures, sizeof(atlas->canonical_textures_for_sprite_textures[0]) * atlas->capacity_sprite_textures);
    }

    unsigned long *assignment = atlas->canonical_textures_for_sprite_textures + atlas->number_sprite_textures;
    *assignment = canonical_index;
    atlas->number_sprite_textures += 1;
}

unsigned long BorderedTextureAtlas_GetTextureHeight(bordered_texture_atlas_p atlas,
                                                    unsigned long texture)
{
    assert(texture < atlas->number_file_object_textures);
    
    struct file_object_texture_s *file_object_texture = atlas->file_object_textures + texture;
    struct canonical_object_texture_s *canonical = atlas->canonical_object_textures + file_object_texture->canonical_texture_index;
    
    return canonical->width;
}

///@FIXME - use polygon_p to replace vertex and numCoordinates (maybe texture in / out))
void BorderedTextureAtlas_GetCoordinates(bordered_texture_atlas_p atlas,
                                         unsigned long texture,
                                         int reverse,
                                         polygon_p poly,
                                         signed shift,
                                         bool split)
{
    assert(poly->vertex_count <= 4);

    if (!atlas->data_has_been_laid_out)
        borderedTextureAtlas_LayOutTextures(atlas);

    assert(texture < atlas->number_file_object_textures);
    struct file_object_texture_s *file_object_texture = atlas->file_object_textures + texture;
    struct canonical_object_texture_s *canonical = atlas->canonical_object_textures + file_object_texture->canonical_texture_index;

    poly->tex_index = canonical->new_page;
    for (unsigned long i = 0; i < poly->vertex_count; i++)
    {
        unsigned x_coord;
        unsigned y_coord;

        switch (file_object_texture->corner_locations[i])
        {
            case TOP_LEFT:
                x_coord = canonical->new_x_with_border + atlas->border_width;
                y_coord = canonical->new_y_with_border + atlas->border_width - shift;
                
                if(split)
                {
                    y_coord += (canonical->height / 2);
                }
                break;
            case TOP_RIGHT:
                x_coord = canonical->new_x_with_border + atlas->border_width + canonical->width;
                y_coord = canonical->new_y_with_border + atlas->border_width - shift;
                
                if(split)
                {
                    y_coord += (canonical->height / 2);
                }
                break;
            case BOTTOM_LEFT:
                x_coord = canonical->new_x_with_border + atlas->border_width;
                y_coord = canonical->new_y_with_border + atlas->border_width + canonical->height - shift;
                break;
            case BOTTOM_RIGHT:
                x_coord = canonical->new_x_with_border + atlas->border_width + canonical->width;
                y_coord = canonical->new_y_with_border + atlas->border_width + canonical->height - shift;
                break;
            default:
                assert(0);
        }

        unsigned long index = reverse ? (poly->vertex_count - i-1) : i;

        poly->vertices[index].tex_coord[0] = (GLfloat) x_coord / (GLfloat) atlas->result_page_width;
        poly->vertices[index].tex_coord[1] = (GLfloat) y_coord / (GLfloat) atlas->result_page_height[canonical->new_page];
    }
}

void BorderedTextureAtlas_GetSpriteCoordinates(bordered_texture_atlas_p atlas, unsigned long sprite_texture, uint32_t *outPage, GLfloat *coordinates)
{
    if (!atlas->data_has_been_laid_out)
        borderedTextureAtlas_LayOutTextures(atlas);

    assert(sprite_texture < atlas->number_sprite_textures);

    unsigned long *canonical_index = atlas->canonical_textures_for_sprite_textures + sprite_texture;
    struct canonical_object_texture_s *canonical = atlas->canonical_object_textures + *canonical_index;

    *outPage = canonical->new_page;

    unsigned pixel_coordinates[8] = {
        // top right
        canonical->new_x_with_border + atlas->border_width + canonical->width,
        canonical->new_y_with_border + atlas->border_width + canonical->height,

        // top left
        canonical->new_x_with_border + atlas->border_width,
        canonical->new_y_with_border + atlas->border_width + canonical->height,

        // bottom left
        canonical->new_x_with_border + atlas->border_width,
        canonical->new_y_with_border + atlas->border_width,

        // bottom right
        canonical->new_x_with_border + atlas->border_width + canonical->width,
        canonical->new_y_with_border + atlas->border_width,
    };

    for (int i = 0; i < 4; i++) {
        coordinates[i*2 + 0] = (GLfloat) pixel_coordinates[i*2 + 0] / (GLfloat) atlas->result_page_width;
        coordinates[i*2 + 1] = (GLfloat) pixel_coordinates[i*2 + 1] / (GLfloat) atlas->result_page_height[canonical->new_page];
    }
}

unsigned long BorderedTextureAtlas_GetNumAtlasPages(bordered_texture_atlas_p atlas)
{
    if (!atlas->data_has_been_laid_out)
        borderedTextureAtlas_LayOutTextures(atlas);

    return atlas->number_result_pages;
}

void BorderedTextureAtlas_CreateTextures(bordered_texture_atlas_p atlas, GLuint *textureNames, GLuint additionalTextureNames)
{
    if (!atlas->data_has_been_laid_out)
        borderedTextureAtlas_LayOutTextures(atlas);

    char *data = (char *) malloc(4 * atlas->result_page_width * atlas->result_page_width);

    glGenTextures((GLsizei) atlas->number_result_pages + additionalTextureNames, textureNames);

    for (unsigned long page = 0; page < atlas->number_result_pages; page++)
    {
        for (unsigned long texture = 0; texture < atlas->number_canonical_object_textures; texture++)
        {
            const struct canonical_object_texture_s *canonical = atlas->canonical_object_textures + texture;
            if (canonical->new_page != page)
                continue;

            const char *original = (char *) atlas->original_pages[canonical->original_page];

            // Add top border
            for (int border = 0; border < atlas->border_width; border++)
            {
                unsigned x = canonical->new_x_with_border;
                unsigned y = canonical->new_y_with_border + border;
                unsigned old_x = canonical->original_x;
                unsigned old_y = canonical->original_y;

                // expand top-left pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x) * 4],
                       &(original[(old_y * 256 + old_x) * 4]),
                       4 * atlas->border_width);
                // copy top line
                memcpy(&data[(y*atlas->result_page_width + x + atlas->border_width) * 4],
                       &original[(old_y * 256 + old_x) * 4],
                       canonical->width * 4);
                // expand top-right pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x + atlas->border_width + canonical->width) * 4],
                       &(original[(old_y * 256 + old_x + canonical->width) * 4]),
                       4 * atlas->border_width);
            }

            // Copy main content
            for (int line = 0; line < canonical->height; line++)
            {
                unsigned x = canonical->new_x_with_border;
                unsigned y = canonical->new_y_with_border + atlas->border_width + line;
                unsigned old_x = canonical->original_x;
                unsigned old_y = canonical->original_y + line;

                // expand left pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x) * 4],
                       &(original[(old_y * 256 + old_x) * 4]),
                       4 * atlas->border_width);
                // copy line
                memcpy(&data[(y*atlas->result_page_width + x + atlas->border_width) * 4],
                       &original[(old_y * 256 + old_x) * 4],
                       canonical->width * 4);
                // expand right pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x + atlas->border_width + canonical->width) * 4],
                       &(original[(old_y * 256 + old_x + canonical->width) * 4]),
                       4 * atlas->border_width);
            }

            // Add bottom border
            for (int border = 0; border < atlas->border_width; border++)
            {
                unsigned x = canonical->new_x_with_border;
                unsigned y = canonical->new_y_with_border + canonical->height + atlas->border_width + border;
                unsigned old_x = canonical->original_x;
                unsigned old_y = canonical->original_y + canonical->height;

                // expand bottom-left pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x) * 4],
                       &(original[(old_y * 256 + old_x) * 4]),
                       4 * atlas->border_width);
                // copy bottom line
                memcpy(&data[(y*atlas->result_page_width + x + atlas->border_width) * 4],
                       &original[(old_y * 256 + old_x) * 4],
                       canonical->width * 4);
                // expand bottom-right pixel
                memset_pattern4(&data[(y*atlas->result_page_width + x + atlas->border_width + canonical->width) * 4],
                       &(original[(old_y * 256 + old_x + canonical->width) * 4]),
                       4 * atlas->border_width);
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureNames[page]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, (GLsizei) atlas->result_page_width, (GLsizei) atlas->result_page_height[page], GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    free(data);
}
