#include "helpers.h"

#include <fstream>

#include <SDL2/SDL_endian.h>

namespace util
{
void writeTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY)
{
    std::ofstream st(filename, std::ios::out | std::ios::binary);
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
        for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
            {
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 2]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 1]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 0]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 3]), 1);
            }
    }
    else
    {
        for(int y = height - 1; y >= 0; y--)
            for(int x = 0; x < width; x++)
            {
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 2]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 1]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 0]), 1);
                st.write(reinterpret_cast<const char*>(&data[(y * width + x) * 4 + 3]), 1);
            }
    }
}
} // namespace util