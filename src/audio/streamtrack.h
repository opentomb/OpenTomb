#pragma once

#include "util/helpers.h"

#include <sndfile.h>
#include <AL/al.h>

#include <boost/optional.hpp>

#include <array>

namespace audio
{
class FxManager;
class Engine;

// Audio stream type defines stream behaviour. While background track
// loops forever until interrupted by other background track, one-shot
// and chat tracks doesn't interrupt them, playing in parallel instead.
// However, all stream types could be interrupted by next pending track
// with same type.
enum class StreamType
{
    Any,
    Background,    // BGM tracks.
    Oneshot,       // One-shot music pieces.
    Chat          // Chat tracks.
};

// Stream loading method describes the way audiotracks are loaded.
// There are either seperate track files or single CDAUDIO.WAD file.
enum class StreamMethod
{
    Any,
    Track,  // Separate tracks. Used in TR 1, 2, 4, 5.
    WAD    // WAD file.  Used in TR3.
};

// Main stream track class is used to create multi-channel soundtrack player,
// which differs from classic TR scheme, where each new soundtrack interrupted
// previous one. With flexible class handling, we now can implement multitrack
// player with automatic channel and crossfade management.
class StreamTrack
{
    DISABLE_COPY(StreamTrack);
public:
    explicit StreamTrack(audio::Engine* engine);
    ~StreamTrack();

    explicit StreamTrack(StreamTrack&& rhs)
        : m_audioEngine(rhs.m_audioEngine)
        , m_wadFile(rhs.m_wadFile)
        , m_sndFile(rhs.m_sndFile)
        , m_sfInfo(std::move(rhs.m_sfInfo))
        , m_source(rhs.m_source)
        , m_buffers(rhs.m_buffers)
        , m_format(rhs.m_format)
        , m_rate(rhs.m_rate)
        , m_currentVolume(rhs.m_currentVolume)
        , m_dampedVolume(rhs.m_dampedVolume)
        , m_active(rhs.m_active)
        , m_fadeoutAndStop(rhs.m_fadeoutAndStop)
        , m_dampable(rhs.m_dampable)
        , m_streamType(rhs.m_streamType)
        , m_currentTrack(rhs.m_currentTrack)
        , m_method(rhs.m_method)
    {
        rhs.m_audioEngine = nullptr;
        rhs.m_wadFile = nullptr;
        rhs.m_sndFile = nullptr;
        rhs.m_source = 0;
        rhs.m_buffers.fill(0);
        rhs.m_format = 0;
        rhs.m_rate = 0;
        rhs.m_currentVolume = 0;
        rhs.m_dampedVolume = 0;
        rhs.m_active = false;
        rhs.m_fadeoutAndStop = false;
        rhs.m_dampable = false;
        rhs.m_streamType = StreamType::Oneshot;
        rhs.m_currentTrack = boost::none;
        rhs.m_method = StreamMethod::Any;
    }

     // Load routine prepares track for playing. Arguments are track index,
     // stream type (background, one-shot or chat) and load method, which
     // differs for TR1-2, TR3 and TR4-5.

    bool load(const char *path, size_t index, const StreamType type, const StreamMethod load_method);
    bool unload();

    bool play(bool fade_in = false);     // Begins to play track.
    //! Pauses track, preserving position.
    void pause();

    //! End track with fade-out.
    void fadeOutAndStop();

    //! Immediately stop track.
    void stop();

    void update();                       // Update track and manage streaming.

    bool isTrack(size_t track_index) const; // Checks desired track's index.
    bool isType(const StreamType track_type) const;   // Checks desired track's type.
    bool isPlaying() const;                    // Checks if track is playing.
    bool isActive() const;                     // Checks if track is still active.
    bool isDampable() const;                   // Checks if track is dampable.

    void setFX();                        // Set reverb FX, according to room flag.
    void unsetFX();                      // Remove any reverb FX from source.

    static bool damp_active;             // Global flag for damping BGM tracks.

private:
    audio::Engine* m_audioEngine;

    // NUMBUFFERS is a number of buffers cyclically used for each stream.
    // Double is enough, but we use quad for further stability.

    static constexpr int StreamBufferCount = 4;

    bool loadTrack(const char *path);                     // Track loading.
    bool loadWad(uint8_t index, const char *filename);    // Wad loading.

    bool stream(ALuint buffer);          // General stream routine.

    FILE*           m_wadFile = nullptr;   //!< General handle for opened wad file.
    SNDFILE*        m_sndFile = nullptr;   //!< Sndfile file reader needs its own handle.
    SF_INFO         m_sfInfo;

    // General OpenAL fields

    ALuint          m_source = 0;
    std::array<ALuint, StreamBufferCount> m_buffers;
    ALenum          m_format = 0;
    ALsizei         m_rate = 0;
    ALfloat         m_currentVolume = 0;     //!< Stream volume, considering fades.
    ALfloat         m_dampedVolume = 0;      //!< Additional damp volume multiplier.

    bool            m_active = false;            //!< If track is active or not.
    bool            m_fadeoutAndStop = false;            //!< Used when track is being faded by other one.
    bool            m_dampable = false;          //!< Specifies if track can be damped by others.
    StreamType      m_streamType = StreamType::Oneshot;        //!< Either BACKGROUND, ONESHOT or CHAT.
    boost::optional<size_t> m_currentTrack = boost::none;      //!< Needed to prevent same track sending.
    StreamMethod    m_method = StreamMethod::Any;            //!< TRACK (TR1-2/4-5) or WAD (TR3).
};
} // namespace audio
