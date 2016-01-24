#pragma once

#include <sndfile.h>
#include <AL/al.h>

#include <boost/optional.hpp>

namespace audio
{
class FxManager;

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
public:
    StreamTrack();      // Stream track constructor.
    ~StreamTrack();      // Stream track destructor.

     // Load routine prepares track for playing. Arguments are track index,
     // stream type (background, one-shot or chat) and load method, which
     // differs for TR1-2, TR3 and TR4-5.

    bool load(const char *path, size_t index, const StreamType type, const StreamMethod load_method);
    bool unload();

    bool play(FxManager &manager, bool fade_in = false);     // Begins to play track.
    //! Pauses track, preserving position.
    void pause();

    //! End track with fade-out.
    void fadeOutAndStop();

    //! Immediately stop track.
    void stop();

    bool update();                       // Update track and manage streaming.

    bool isTrack(size_t track_index) const; // Checks desired track's index.
    bool isType(const StreamType track_type) const;   // Checks desired track's type.
    bool isPlaying() const;                    // Checks if track is playing.
    bool isActive() const;                     // Checks if track is still active.
    bool isDampable() const;                   // Checks if track is dampable.

    void setFX(FxManager& manager);                        // Set reverb FX, according to room flag.
    void unsetFX();                      // Remove any reverb FX from source.

    static bool damp_active;             // Global flag for damping BGM tracks.

private:
    // NUMBUFFERS is a number of buffers cyclically used for each stream.
    // Double is enough, but we use quad for further stability.

    static constexpr int StreamBufferCount = 4;

    bool loadTrack(const char *path);                     // Track loading.
    bool loadWad(uint8_t index, const char *filename);    // Wad loading.

    bool stream(ALuint buffer);          // General stream routine.

    FILE*           m_wadFile;   //!< General handle for opened wad file.
    SNDFILE*        m_sndFile;   //!< Sndfile file reader needs its own handle.
    SF_INFO         m_sfInfo;

    // General OpenAL fields

    ALuint          m_source;
    ALuint          m_buffers[StreamBufferCount];
    ALenum          m_format;
    ALsizei         m_rate;
    ALfloat         m_currentVolume;     //!< Stream volume, considering fades.
    ALfloat         m_dampedVolume;      //!< Additional damp volume multiplier.

    bool            m_active;            //!< If track is active or not.
    bool            m_fadeoutAndStop;            //!< Used when track is being faded by other one.
    bool            m_dampable;          //!< Specifies if track can be damped by others.
    StreamType      m_streamType;        //!< Either BACKGROUND, ONESHOT or CHAT.
    boost::optional<size_t> m_currentTrack = boost::none;      //!< Needed to prevent same track sending.
    StreamMethod    m_method;            //!< TRACK (TR1-2/4-5) or WAD (TR3).
};
} // namespace audio
