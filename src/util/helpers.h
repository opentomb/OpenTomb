#pragma once

#include <LinearMath/btScalar.h>

#include <cstdint>

namespace util
{

template<typename T>
inline T clamp(T v, T l, T h)
{
    if(v<l)
        return l;
    else if(v>h)
        return h;
    else
        return v;
}

inline btScalar wrapAngle(const btScalar value)
{
    int i = static_cast<int>(value / 360.0);
    i = (value < 0.0)?(i-1):(i);
    return value - 360.0f * i;
}

void writeTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);

} // namespace util
