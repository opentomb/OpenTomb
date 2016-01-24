#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cstdint>
#include <chrono>

#define ENUM_TO_OSTREAM(name) \
    inline std::ostream& operator<<(std::ostream& str, name e) \
    { \
        return str << static_cast<int>(e); \
    } \

namespace util
{
inline glm::float_t wrapAngle(const glm::float_t value)
{
    int i = static_cast<int>(value / 360.0);
    i = value < 0.0 ? i - 1 : i;
    return value - 360.0f * i;
}

template<typename T>
inline bool fuzzyZero(T value) noexcept
{
    return glm::abs(value) <= glm::epsilon<T>();
}

template<typename T>
inline bool fuzzyEqual(T a, T b) noexcept
{
    return fuzzyZero(a - b);
}

template<typename T>
inline bool fuzzyOne(T value) noexcept
{
    return fuzzyEqual(value, static_cast<T>(1));
}

void writeTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);

using ClockType = std::chrono::high_resolution_clock;

using FloatDuration = float;
using Duration = std::chrono::duration<FloatDuration, std::chrono::nanoseconds::period>;
using Seconds = std::chrono::duration<FloatDuration, std::chrono::seconds::period>;
using MilliSeconds = std::chrono::duration<FloatDuration, std::chrono::milliseconds::period>;

using TimePoint = ClockType::time_point;

constexpr inline FloatDuration toSeconds(Duration d) noexcept
{
    return d.count() * static_cast<FloatDuration>(Duration::period::num) / static_cast<FloatDuration>(Duration::period::den);
}

constexpr inline util::Duration fromSeconds(FloatDuration d) noexcept
{
    return Duration(d * static_cast<FloatDuration>(Duration::period::den) / static_cast<FloatDuration>(Duration::period::num));
}

inline TimePoint now() noexcept
{
    return ClockType::now();
}
} // namespace util
