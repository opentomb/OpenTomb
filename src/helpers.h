#pragma once

#include <LinearMath/btScalar.h>
#include <cstdint>

template<typename T>
inline T Clamp(T v, T l, T h)
{
    if(v<l)
        return l;
    else if(v>h)
        return h;
    else
        return v;
}

btScalar WrapAngle(const btScalar value);

void WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);
