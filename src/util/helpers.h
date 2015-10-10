#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

inline glm::float_t wrapAngle(const glm::float_t value)
{
    int i = static_cast<int>(value / 360.0);
    i = (value < 0.0)?(i-1):(i);
    return value - 360.0f * i;
}

template<typename T>
inline bool fuzzyZero(T value) noexcept
{
    return value <= glm::epsilon<T>();
}

template<typename T>
inline bool fuzzyEqual(T a, T b) noexcept
{
    return fuzzyZero(a-b);
}

template<typename T>
inline bool fuzzyOne(T value) noexcept
{
    return fuzzyEqual(value, static_cast<T>(1));
}

void writeTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);

} // namespace util
