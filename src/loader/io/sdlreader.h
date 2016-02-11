#pragma once

#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_endian.h>

#include <zlib.h>

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_index.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace loader
{
namespace io
{
class SDLReader
{
    SDLReader(const SDLReader&) = delete;
    SDLReader& operator=(const SDLReader&) = delete;
public:
    explicit SDLReader(SDL_RWops* ops)
        : m_rwOps(ops)
        , m_memory()
    {
    }

    SDLReader(SDLReader&& rhs)
        : m_rwOps(rhs.m_rwOps)
        , m_memory(std::move(rhs.m_memory))
    {
        rhs.m_rwOps = nullptr;
    }

    explicit SDLReader(const std::string& filename)
        : m_rwOps(SDL_RWFromFile(filename.c_str(), "rb"))
        , m_memory()
    {
    }

    explicit SDLReader(const std::vector<uint8_t>& data)
        : m_rwOps(nullptr)
        , m_memory(data)
    {
        m_rwOps = SDL_RWFromConstMem(m_memory.data(), static_cast<int>(m_memory.size()));
    }

    explicit SDLReader(std::vector<uint8_t>&& data)
        : m_rwOps(nullptr)
        , m_memory(std::move(data))
    {
        m_rwOps = SDL_RWFromConstMem(m_memory.data(), static_cast<int>(m_memory.size()));
    }

    ~SDLReader()
    {
        if(m_rwOps)
            SDL_RWclose(m_rwOps);
    }

    static SDLReader decompress(const std::vector<uint8_t>& compressed, size_t uncompressedSize)
    {
        std::vector<uint8_t> uncomp_buffer(uncompressedSize);

        uLongf size = static_cast<uLongf>(uncompressedSize);
        if(uncompress(uncomp_buffer.data(), &size, compressed.data(), static_cast<uLong>(compressed.size())) != Z_OK)
            BOOST_THROW_EXCEPTION(std::runtime_error("Decompression failed"));

        if(size != uncompressedSize)
            BOOST_THROW_EXCEPTION(std::runtime_error("Decompressed size mismatch"));

        io::SDLReader reader(std::move(uncomp_buffer));
        if(!reader.isOpen())
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create reader from decompressed memory"));

        return reader;
    }

    bool isOpen() const
    {
        return m_rwOps != nullptr;
    }

    Sint64 tell() const
    {
        return SDL_RWseek(m_rwOps, 0, RW_SEEK_CUR);
    }

    Sint64 size() const
    {
        return SDL_RWsize(m_rwOps);
    }

    void skip(Sint64 delta)
    {
        SDL_RWseek(m_rwOps, delta, RW_SEEK_CUR);
    }

    void seek(Sint64 position)
    {
        SDL_RWseek(m_rwOps, position, RW_SEEK_SET);
    }

    template<typename T>
    void readBytes(T* dest, size_t n)
    {
        static_assert(std::is_integral<T>::value && sizeof(T) == 1, "readBytes() only allowed for byte-compatible data");
        if(SDL_RWread(m_rwOps, dest, 1, n) != n)
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("EOF unexpectedly reached"));
        }
    }

    template<typename T>
    using Producer = std::unique_ptr<T>(SDLReader&);

    template<typename T>
    void readVector(std::vector<T>& elements, size_t count, Producer<T> producer)
    {
        elements.clear();
        appendVector(elements, count, producer);
    }

    template<typename T>
    void appendVector(std::vector<T>& elements, size_t count, Producer<T> producer)
    {
        BOOST_LOG_TRIVIAL(debug) << "Appending " << count << " elements of type `" << boost::typeindex::type_id<T>().pretty_name() << "` (size " << sizeof(T) << ") to a vector of size " << elements.size();

        elements.reserve(elements.size() + count);
        for(size_t i = 0; i < count; ++i)
        {
            elements.emplace_back(*producer(*this));
        }
    }

    template<typename T>
    void readVector(std::vector<T>& elements, size_t count)
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading " << count << " elements of type `" << boost::typeindex::type_id<T>().pretty_name() << "` (size " << sizeof(T) << ")";

        elements.clear();
        elements.reserve(count);
        for(size_t i = 0; i < count; ++i)
        {
            elements.emplace_back(read<T>());
        }
    }

    void readVector(std::vector<uint8_t>& elements, size_t count)
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading " << count << " elements of type `" << boost::typeindex::type_id<uint8_t>().pretty_name() << "` (size " << sizeof(uint8_t) << ")";

        elements.clear();
        elements.resize(count);
        readBytes(elements.data(), count);
    }

    void readVector(std::vector<int8_t>& elements, size_t count)
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading " << count << " elements of type `" << boost::typeindex::type_id<int8_t>().pretty_name() << "` (size " << sizeof(int8_t) << ")";

        elements.clear();
        elements.resize(count);
        readBytes(elements.data(), count);
    }

    template<typename T>
    T read()
    {
        T result;
        if(SDL_RWread(m_rwOps, &result, sizeof(T), 1) != 1)
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("EOF unexpectedly reached"));
        }

        SwapTraits<T, sizeof(T), std::is_integral<T>::value || std::is_floating_point<T>::value>::doSwap(result);

        return result;
    }

    uint8_t readU8()
    {
        return read<uint8_t>();
    }
    int8_t readI8()
    {
        return read<int8_t>();
    }
    uint16_t readU16()
    {
        return read<uint16_t>();
    }
    int16_t readI16()
    {
        return read<int16_t>();
    }
    uint32_t readU32()
    {
        return read<uint32_t>();
    }
    int32_t readI32()
    {
        return read<int32_t>();
    }
    float readF()
    {
        return read<float>();
    }

private:
    SDL_RWops* m_rwOps;
    std::vector<uint8_t> m_memory;

    template<typename T, int dataSize, bool isIntegral>
    struct SwapTraits
    {
    };

    template<typename T>
    struct SwapTraits<T, 1, true>
    {
        static void doSwap(T&)
        {
            // no-op
        }
    };

    template<typename T>
    struct SwapTraits<T, 2, true>
    {
        static void doSwap(T& data)
        {
            data = SDL_SwapLE16(data);
        }
    };

    template<typename T>
    struct SwapTraits<T, 4, true>
    {
        static void doSwap(T& data)
        {
            data = SDL_SwapLE32(data);
        }
    };
};
} // namespace io
} // namespace loader
