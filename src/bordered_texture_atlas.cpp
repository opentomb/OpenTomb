//
//  bordered_texture_atlas.cpp
//  tr1engine
//
//  Created by Torsten Kammer on 16.07.13.
//  Copyright (c) 2013 Torsten Kammer. All rights reserved.
//

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "vt/vt_level.h"

#include "bordered_texture_atlas.h"
#include "bsp_tree_2d.h"
#include "gl_util.h"
#include "polygon.h"

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

namespace {
inline GLuint NextPowerOf2(GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}
}

/*!
 * Compare function for std::sort. It interprets the the parameters as pointers
 * to indices into the canonical object textures of the atlas currently stored
 * in compare_context. It returns true or false if the first texture is logically
 * ordered before, the same or after the second texture.
 *
 * A texture comes before another texture if it is higher. If both have the same
 * height, the wider texture comes first. If both have the same height and width,
 * they are ordered the same.
 */

struct BorderedTextureAtlas::TextureSizeComparator
{
    const BorderedTextureAtlas* const context;

    explicit TextureSizeComparator(const BorderedTextureAtlas* context_)
        : context(context_)
    {
    }

    bool operator()(size_t index1, size_t index2) const
    {
        const BorderedTextureAtlas::CanonicalObjectTexture &texture1 = context->m_canonicalObjectTextures[index1];
        const BorderedTextureAtlas::CanonicalObjectTexture &texture2 = context->m_canonicalObjectTextures[index2];

        // First order by height.
        if (texture1.height > texture2.height)
            return true;
        else if (texture1.height < texture2.height)
            return false;

        // Then order by width
        if (texture1.width > texture2.width)
            return true;
        else if (texture1.width < texture2.width)
            return false;

        // If they have the same height and width then their order does not matter.
        return false;
    }
};

/*!
 * Lays out the texture data and switches the atlas to laid out mode. This makes
 * use of a bsp_tree_2d to handle all the really annoying stuff.
 */
void BorderedTextureAtlas::layOutTextures()
{
    // First step: Sort the canonical textures by size.
    std::vector<size_t> sorted_indices(m_canonicalObjectTextures.size());
    for (size_t i = 0; i < m_canonicalObjectTextures.size(); i++)
        sorted_indices[i] = i;

    std::sort(sorted_indices.begin(), sorted_indices.end(), TextureSizeComparator(this));

    // Find positions for the canonical textures
    std::vector<BSPTree2DNode> result_pages;
    m_resultPageHeights.clear();

    for (size_t texture = 0; texture < m_canonicalObjectTextures.size(); texture++)
    {
        struct CanonicalObjectTexture &canonical = m_canonicalObjectTextures[sorted_indices[texture]];

        // Try to find space in an existing page.
        bool found_place = false;
        for (size_t page = 0; page < m_resultPageHeights.size(); page++)
        {
            found_place = result_pages[page].findSpaceFor(canonical.width  + 2*m_borderWidth,
                                                          canonical.height + 2*m_borderWidth,
                                                          &canonical.new_x_with_border,
                                                          &canonical.new_y_with_border);
            if (found_place)
            {
                canonical.new_page = page;

                size_t highest_y = canonical.new_y_with_border + canonical.height + m_borderWidth * 2;
                if (highest_y + 1 > m_resultPageHeights[page])
                    m_resultPageHeights[page] = static_cast<unsigned int>(highest_y);

                break;
            }
        }

        // No existing page has enough remaining space so open new one.
        if (!found_place)
        {
            result_pages.emplace_back(0, 0, m_resultPageWidth, m_resultPageWidth);
            canonical.new_page = m_resultPageHeights.size();

            result_pages.back().findSpaceFor(canonical.width  + 2*m_borderWidth,
                                             canonical.height + 2*m_borderWidth,
                                             &canonical.new_x_with_border,
                                             &canonical.new_y_with_border);

            m_resultPageHeights.emplace_back(canonical.new_y_with_border + canonical.height + m_borderWidth * 2);
        }
    }

    // Fix up heights if necessary
    for (uint32_t& height : m_resultPageHeights)
    {
        height = NextPowerOf2(height);
    }
}

BorderedTextureAtlas::BorderedTextureAtlas(int border,
                                           bool conserve_memory,
                                           const std::vector<tr4_textile32_t>& pages,
                                           const std::vector<tr4_object_texture_t>& object_textures,
                                           const std::vector<tr_sprite_texture_t>& sprite_textures)
    : m_borderWidth(border)
    , m_resultPageWidth(0)
    , m_resultPageHeights()
    , m_originalPages(pages)
    , m_fileObjectTextures()
    , m_canonicalTexturesForSpriteTextures()
    , m_canonicalObjectTextures()
{
    GLint max_texture_edge_length = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_edge_length);
    if (max_texture_edge_length > 4096)
        max_texture_edge_length = 4096; // That is already 64 MB and covers up to 256 pages.

    if(conserve_memory)
    {
        // Idea: sqrt(sum(areas)) * sqrt(2) >= needed area
        size_t areaSum = 0;
        for(const tr4_object_texture_t& t : object_textures)
            areaSum += t.x_size * t.y_size;
        for(const tr_sprite_texture_t& t : sprite_textures)
            areaSum += std::abs( (t.x1-t.x0) * (t.y1-t.y0) );

        m_resultPageWidth = std::min( max_texture_edge_length, static_cast<GLint>(NextPowerOf2(static_cast<GLuint>(std::sqrt(areaSum)*1.41))) );
    }
    else
    {
        m_resultPageWidth = NextPowerOf2(max_texture_edge_length);
    }

    for(const tr4_object_texture_t& tex : object_textures)
    {
        addObjectTexture(tex);
    }

    for (const tr_sprite_texture_t& tex : sprite_textures)
    {
        addSpriteTexture(tex);
    }

    layOutTextures();
}

void BorderedTextureAtlas::addObjectTexture(const tr4_object_texture_t &texture)
{
    // Determine the canonical texture for this texture.
    // Use only first three vertices to find min, max, because for triangles the last will be 0,0 with no other marker that this is a triangle. As long as all textures are axis-aligned rectangles, this will always return the right result anyway.
    uint8_t max[2] = { 0, 0 }, min[2] = { 255, 255 };
    for (int i = 0; i < 3; i++)
    {
        max[0] = std::max(max[0], texture.vertices[i].xpixel);
        max[1] = std::max(max[1], texture.vertices[i].ypixel);
        min[0] = std::min(min[0], texture.vertices[i].xpixel);
        min[1] = std::min(min[1], texture.vertices[i].ypixel);
    }
    const uint8_t width = max[0] - min[0];
    const uint8_t height = max[1] - min[1];

    // See whether it already exists
    size_t canonical_index = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < m_canonicalObjectTextures.size(); i++)
    {
        CanonicalObjectTexture *canonical_candidate = &m_canonicalObjectTextures[i];

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
    if (canonical_index == std::numeric_limits<size_t>::max())
    {
        canonical_index = m_canonicalObjectTextures.size();
        m_canonicalObjectTextures.emplace_back();

        CanonicalObjectTexture &canonical = m_canonicalObjectTextures.back();
        canonical.width = width;
        canonical.height = height;
        canonical.original_page = texture.tile_and_flag & TR_TEXTURE_INDEX_MASK_TR4;
        canonical.original_x = min[0];
        canonical.original_y = min[1];
    }

    // Create file object texture.
    m_fileObjectTextures.emplace_back();
    FileObjectTexture& file_object_texture = m_fileObjectTextures.back();

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

void BorderedTextureAtlas::addSpriteTexture(const tr_sprite_texture_t &texture)
{
    // Determine the canonical texture for this texture.
    unsigned x = texture.x0;
    unsigned y = texture.y0;
    unsigned width = texture.x1 - texture.x0;
    unsigned height = texture.y1 - texture.y0;

    // See whether it already exists
    size_t canonical_index = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < m_canonicalObjectTextures.size(); i++)
    {
        CanonicalObjectTexture *canonical_candidate = &m_canonicalObjectTextures[i];

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
    if (canonical_index == std::numeric_limits<size_t>::max())
    {
        canonical_index = m_canonicalObjectTextures.size();
        m_canonicalObjectTextures.emplace_back();

        CanonicalObjectTexture &canonical = m_canonicalObjectTextures[canonical_index];
        canonical.width = width;
        canonical.height = height;
        canonical.original_page = texture.tile & TR_TEXTURE_INDEX_MASK_TR4;
        canonical.original_x = x;
        canonical.original_y = y;
    }

    // Create sprite texture assignmen.
    m_canonicalTexturesForSpriteTextures.emplace_back(canonical_index);
}

size_t BorderedTextureAtlas::getTextureHeight(size_t texture) const
{
    assert(texture < m_fileObjectTextures.size());

    const FileObjectTexture &file_object_texture = m_fileObjectTextures[texture];
    const CanonicalObjectTexture &canonical = m_canonicalObjectTextures[file_object_texture.canonical_texture_index];

    return canonical.height;
}

///@FIXME - use Polygon* to replace vertex and numCoordinates (maybe texture in / out))
void BorderedTextureAtlas::getCoordinates(size_t texture,
                                          bool reverse,
                                          struct Polygon *poly,
                                          int shift,
                                          bool split)  const
{
    assert(poly->vertices.size() <= 4);

    assert(texture < m_fileObjectTextures.size());
    const FileObjectTexture& file_object_texture = m_fileObjectTextures[texture];
    const CanonicalObjectTexture &canonical = m_canonicalObjectTextures[file_object_texture.canonical_texture_index];

    poly->tex_index = static_cast<uint16_t>( canonical.new_page );
    for (size_t i = 0; i < poly->vertices.size(); i++)
    {
        unsigned x_coord = 0;
        unsigned y_coord = 0;

        switch (file_object_texture.corner_locations[i])
        {
            case TOP_LEFT:
                x_coord = canonical.new_x_with_border + m_borderWidth;
                y_coord = canonical.new_y_with_border + m_borderWidth - shift;

                if(split)
                {
                    y_coord += (canonical.height / 2);
                }
                break;
            case TOP_RIGHT:
                x_coord = canonical.new_x_with_border + m_borderWidth + canonical.width;
                y_coord = canonical.new_y_with_border + m_borderWidth - shift;

                if(split)
                {
                    y_coord += (canonical.height / 2);
                }
                break;
            case BOTTOM_LEFT:
                x_coord = canonical.new_x_with_border + m_borderWidth;
                y_coord = canonical.new_y_with_border + m_borderWidth + canonical.height - shift;
                break;
            case BOTTOM_RIGHT:
                x_coord = canonical.new_x_with_border + m_borderWidth + canonical.width;
                y_coord = canonical.new_y_with_border + m_borderWidth + canonical.height - shift;
                break;
            default:
                assert(false);
        }

        size_t index = reverse ? (poly->vertices.size() - i-1) : i;

        poly->vertices[index].tex_coord[0] = (GLfloat) x_coord / (GLfloat) m_resultPageWidth;
        poly->vertices[index].tex_coord[1] = (GLfloat) y_coord / (GLfloat) m_resultPageHeights[canonical.new_page];
    }
}

void BorderedTextureAtlas::getSpriteCoordinates(size_t sprite_texture, uint32_t &outPage, GLfloat *coordinates) const
{
    assert(sprite_texture < m_canonicalTexturesForSpriteTextures.size());

    const size_t canonical_index = m_canonicalTexturesForSpriteTextures[sprite_texture];
    const CanonicalObjectTexture &canonical = m_canonicalObjectTextures[canonical_index];

    outPage = canonical.new_page;

    size_t pixel_coordinates[8] = {
        // top right
        canonical.new_x_with_border + m_borderWidth + canonical.width,
        canonical.new_y_with_border + m_borderWidth + canonical.height,

        // top left
        canonical.new_x_with_border + m_borderWidth,
        canonical.new_y_with_border + m_borderWidth + canonical.height,

        // bottom left
        canonical.new_x_with_border + m_borderWidth,
        canonical.new_y_with_border + m_borderWidth,

        // bottom right
        canonical.new_x_with_border + m_borderWidth + canonical.width,
        canonical.new_y_with_border + m_borderWidth,
    };

    for (int i = 0; i < 4; i++) {
        coordinates[i*2 + 0] = (GLfloat) pixel_coordinates[i*2 + 0] / (GLfloat) m_resultPageWidth;
        coordinates[i*2 + 1] = (GLfloat) pixel_coordinates[i*2 + 1] / (GLfloat) m_resultPageHeights[canonical.new_page];
    }
}

size_t BorderedTextureAtlas::getNumAtlasPages() const
{
    return m_resultPageHeights.size();
}

void BorderedTextureAtlas::createTextures(GLuint *textureNames, GLuint additionalTextureNames) const
{
    glGenTextures((GLsizei) m_resultPageHeights.size() + additionalTextureNames, textureNames);

    for (size_t page = 0; page < m_resultPageHeights.size(); page++)
    {
        std::vector<GLubyte> data(4 * m_resultPageWidth * m_resultPageWidth, 0);
        for (size_t texture = 0; texture < m_canonicalObjectTextures.size(); texture++)
        {
            const CanonicalObjectTexture &canonical = m_canonicalObjectTextures[texture];
            if (canonical.new_page != page)
                continue;

            const uint32_t* pixels = reinterpret_cast<const uint32_t*>( m_originalPages[canonical.original_page].pixels );

            // Add top border
            for (int border = 0; border < m_borderWidth; border++)
            {
                unsigned x = canonical.new_x_with_border;
                unsigned y = canonical.new_y_with_border + border;
                unsigned old_x = canonical.original_x;
                unsigned old_y = canonical.original_y;

                // expand top-left pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x) * 4],
                                &pixels[old_y * 256 + old_x],
                                4 * m_borderWidth);
                // copy top line
                memcpy(&data[(y*m_resultPageWidth + x + m_borderWidth) * 4],
                       &pixels[old_y * 256 + old_x],
                       canonical.width * 4);
                // expand top-right pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x + m_borderWidth + canonical.width) * 4],
                                &pixels[old_y * 256 + old_x + canonical.width],
                                4 * m_borderWidth);
            }

            // Copy main content
            for (int line = 0; line < canonical.height; line++)
            {
                unsigned x = canonical.new_x_with_border;
                unsigned y = canonical.new_y_with_border + m_borderWidth + line;
                unsigned old_x = canonical.original_x;
                unsigned old_y = canonical.original_y + line;

                // expand left pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x) * 4],
                                &pixels[old_y * 256 + old_x],
                                4 * m_borderWidth);
                // copy line
                memcpy(&data[(y*m_resultPageWidth + x + m_borderWidth) * 4],
                       &pixels[old_y * 256 + old_x],
                       canonical.width * 4);
                // expand right pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x + m_borderWidth + canonical.width) * 4],
                                &pixels[old_y * 256 + old_x + canonical.width],
                                4 * m_borderWidth);
            }

            // Add bottom border
            for (int border = 0; border < m_borderWidth; border++)
            {
                unsigned x = canonical.new_x_with_border;
                unsigned y = canonical.new_y_with_border + canonical.height + m_borderWidth + border;
                unsigned old_x = canonical.original_x;
                unsigned old_y = canonical.original_y + canonical.height;

                // expand bottom-left pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x) * 4],
                                &pixels[old_y * 256 + old_x],
                                4 * m_borderWidth);
                // copy bottom line
                memcpy(&data[(y*m_resultPageWidth + x + m_borderWidth) * 4],
                       &pixels[old_y * 256 + old_x],
                       canonical.width * 4);
                // expand bottom-right pixel
                memset_pattern4(&data[(y*m_resultPageWidth + x + m_borderWidth + canonical.width) * 4],
                                &pixels[old_y * 256 + old_x + canonical.width],
                                4 * m_borderWidth);
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureNames[page]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_resultPageWidth, (GLsizei) m_resultPageHeights[page], 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        if(glGenerateMipmap != NULL)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            int mip_level = 1;
            int w = m_resultPageWidth / 2;
            int h = m_resultPageHeights[page] / 2;
            std::vector<GLubyte> mip_data(4 * w * h);

            assert(w > 0 && h > 0);
            for(int i=0;i<h;i++)
            {
                for(int j=0;j<w;j++)
                {
                    mip_data[i * w * 4 + j * 4 + 0] = 0.25f * ((int)data[i * w * 16 + j * 8 + 0] + (int)data[i * w * 16 + j * 8 + 4 + 0] + (int)data[i * w * 16 + w * 8 + j * 8 + 0] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 0]);
                    mip_data[i * w * 4 + j * 4 + 1] = 0.25f * ((int)data[i * w * 16 + j * 8 + 1] + (int)data[i * w * 16 + j * 8 + 4 + 1] + (int)data[i * w * 16 + w * 8 + j * 8 + 1] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 1]);
                    mip_data[i * w * 4 + j * 4 + 2] = 0.25f * ((int)data[i * w * 16 + j * 8 + 2] + (int)data[i * w * 16 + j * 8 + 4 + 2] + (int)data[i * w * 16 + w * 8 + j * 8 + 2] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 2]);
                    mip_data[i * w * 4 + j * 4 + 3] = 0.25f * ((int)data[i * w * 16 + j * 8 + 3] + (int)data[i * w * 16 + j * 8 + 4 + 3] + (int)data[i * w * 16 + w * 8 + j * 8 + 3] + (int)data[i * w * 16 + w * 8 + j * 8 + 4 + 3]);
                }
            }

            //char tgan[128];
            //WriteTGAfile("mip_00.tga", data, result_page_width, result_page_height[page], 0);
            //sprintf(tgan, "mip_%0.2d.tga", mip_level);
            //WriteTGAfile(tgan, mip_data, w, h, 0);
            glTexImage2D(GL_TEXTURE_2D, mip_level, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip_data.data());

            while((w > 1) && (h > 1) /*&& (mip_level < 4)*/)
            {
                mip_level++;
                w /= 2; w = (w==0)?1:w;
                h /= 2; h = (h==0)?1:h;
                for(int i=0;i<h;i++)
                {
                    for(int j=0;j<w;j++)
                    {
                        mip_data[i * w * 4 + j * 4 + 0] = 0.25f * ((int)mip_data[i * w * 16 + j * 8 + 0] + (int)mip_data[i * w * 16 + j * 8 + 4 + 0] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 0] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 0]);
                        mip_data[i * w * 4 + j * 4 + 1] = 0.25f * ((int)mip_data[i * w * 16 + j * 8 + 1] + (int)mip_data[i * w * 16 + j * 8 + 4 + 1] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 1] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 1]);
                        mip_data[i * w * 4 + j * 4 + 2] = 0.25f * ((int)mip_data[i * w * 16 + j * 8 + 2] + (int)mip_data[i * w * 16 + j * 8 + 4 + 2] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 2] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 2]);
                        mip_data[i * w * 4 + j * 4 + 3] = 0.25f * ((int)mip_data[i * w * 16 + j * 8 + 3] + (int)mip_data[i * w * 16 + j * 8 + 4 + 3] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 3] + (int)mip_data[i * w * 16 + w * 8 + j * 8 + 4 + 3]);
                    }
                }
                //sprintf(tgan, "mip_%0.2d.tga", mip_level);
                //WriteTGAfile(tgan, mip_data, w, h, 0);
                glTexImage2D(GL_TEXTURE_2D, mip_level, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip_data.data());
            }
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}
