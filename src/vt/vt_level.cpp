#include "vt_level.h"

#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>

#include <fstream>

using namespace loader;

StaticMesh *TR1Level::findStaticMeshById(uint32_t object_id)
{
    for (size_t i = 0; i < m_staticMeshes.size(); i++)
        if ((m_staticMeshes[i].object_id == object_id) && (m_meshIndices[m_staticMeshes[i].mesh]))
            return &m_staticMeshes[i];

    return nullptr;
}

Item *TR1Level::fineItemById(int32_t object_id)
{
    for (size_t i = 0; i < m_items.size(); i++)
        if (m_items[i].object_id == object_id)
            return &m_items[i];

    return nullptr;
}

Moveable *TR1Level::findMoveableById(uint32_t object_id)
{
    for (size_t i = 0; i < m_moveables.size(); i++)
        if (m_moveables[i].object_id == object_id)
            return &m_moveables[i];

    return nullptr;
}

void TR1Level::convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst)
{
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];

            if (col > 0)
                dst.pixels[y][x] = static_cast<int>(pal.colour[col].r) | (static_cast<int>(pal.colour[col].g) << 8) | (static_cast<int>(pal.colour[col].b) << 16) | (0xff << 24);
            else
                dst.pixels[y][x] = 0x00000000;
        }
    }
}

void TR1Level::convertTexture(WordTexture & tex, DWordTexture & dst)
{
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
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
    std::ofstream st(filename, std::ios::out|std::ios::binary);
    if(!st.is_open())
        return;

    // write the header
    // id_length
    char c = 0;
    st.write(&c, 1);
    // colormap_type
    c = 0;
    st.write(&c, 1);
    // image_type
    c = 2;
    st.write(&c, 1);
    // colormap_index
    uint16_t s = 0;
    st.write(reinterpret_cast<char*>(&s), 2);
    // colormap_length
    s = 0;
    st.write(reinterpret_cast<char*>(&s), 2);
    // colormap_size
    c = 0;
    st.write(&c, 1);
    // x_origin
    s = 0;
    st.write(reinterpret_cast<char*>(&s), 2);
    // y_origin
    s = 0;
    st.write(reinterpret_cast<char*>(&s), 2);
    // width
    s = SDL_SwapLE16(width);
    st.write(reinterpret_cast<char*>(&s), 2);
    // height
    s = SDL_SwapLE16(height);
    st.write(reinterpret_cast<char*>(&s), 2);
    // bits_per_pixel
    c = 32;
    st.write(&c, 1);
    // attributes
    c = 0;
    st.write(&c, 1);

    if(invY)
    {
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
            {
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 2]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 1]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 0]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 3]), 1);
            }
    }
    else
    {
        for (int y = height-1; y >= 0; y--)
            for (int x = 0; x < width; x++)
            {
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 2]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 1]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 0]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 3]), 1);
            }
    }
}

void TR1Level::dumpTextures()
{
    uint32_t i;
    char buffer[1024];

    for (i = 0; i < m_textile32.size(); i++)
    {
        snprintf(buffer, 1024, "dump/%03i_32.tga", i);
        WriteTGAfile(buffer, reinterpret_cast<uint8_t *>(&m_textile32[i].pixels), 256, 256, 0);
    }
}

