#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <chrono>

#define ENUM_TO_OSTREAM(name) \
    inline std::ostream& operator<<(std::ostream& str, name e) \
    { \
        return str << static_cast<int>(e); \
    } \

#define DISABLE_COPY(classname) \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete;

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

template<typename T>
inline T getSetting(boost::property_tree::ptree& ptree, const char* path, const T& defaultValue)
{
    T result = ptree.get<T>(path, defaultValue);
    ptree.put(path, defaultValue);
    return result;
}

inline boost::property_tree::ptree& getSettingChild(boost::property_tree::ptree& ptree, const char* path)
{
    if(!ptree.get_child_optional(path))
    {
        ptree.add_child(path, boost::property_tree::ptree());
    }

    return ptree.get_child(path);
}

class LifetimeTracker final
{
    DISABLE_COPY(LifetimeTracker);
private:
    const std::string m_objectName;
    const uintptr_t m_id;

public:
    explicit LifetimeTracker(const std::string& name, const void* ptr)
        : m_objectName(name)
        , m_id(reinterpret_cast<const uintptr_t>(ptr))
    {
        BOOST_LOG_TRIVIAL(debug) << "Initializing: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
    explicit LifetimeTracker(std::string&& name, const void* ptr)
        : m_objectName(std::move(name))
        , m_id(reinterpret_cast<const uintptr_t>(ptr))
    {
        BOOST_LOG_TRIVIAL(debug) << "Initializing: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
    ~LifetimeTracker()
    {
        BOOST_LOG_TRIVIAL(debug) << "Shut down complete: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
};

#define TRACK_LIFETIME() \
    ::util::LifetimeTracker _S_lifetimeTracker{ boost::typeindex::type_id<decltype(*this)>().pretty_name(), this }

} // namespace util
