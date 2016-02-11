#pragma once

#include "effect.h"
#include "emitter.h"
#include "fxmanager.h"
#include "settings.h"
#include "source.h"
#include "streamtrack.h"
#include "util/helpers.h"
#include "world/object.h"


#include <AL/alc.h>

#include <boost/assert.hpp>
#include <boost/optional.hpp>

#include <memory>

namespace world
{
class World;
} // namespace world

namespace loader
{
class Level;
} // namespace loader

namespace audio
{
// Possible types of errors returned by Audio_Send / Audio_Kill functions.
enum class Error
{
    NoSample,
    NoChannel,
    Ignored,
    Processed
};

// Possible errors produced by Audio_StreamPlay / Audio_StreamStop functions.
enum class StreamError
{
    PlayError,
    LoadError,
    WrongTrack,
    NoFreeStream,
    Ignored,
    Processed
};

class Engine
{
    DISABLE_COPY(Engine);
    TRACK_LIFETIME();

    struct DeviceManager
    {
        DISABLE_COPY(DeviceManager);
        TRACK_LIFETIME();

        explicit DeviceManager(Engine* engine);
        ~DeviceManager();

    private:
        Engine* m_engine;
        ALCdevice* m_device = nullptr;
        ALCcontext* m_context = nullptr;
    };

public:
    explicit Engine(engine::Engine* engine, boost::property_tree::ptree& config);
    ~Engine();

    engine::Engine* getEngine() const
    {
        return m_engine;
    }

    void pauseAllSources();
    void stopAllSources();
    void resumeAllSources();
    boost::optional<size_t> getFreeSource() const;
    bool endStreams(StreamType stream_type = StreamType::Any);
    bool stopStreams(StreamType stream_type = StreamType::Any);
    bool isTrackPlaying(const boost::optional<int32_t>& track_index = boost::none) const;
    boost::optional<size_t> findSource(const boost::optional<SoundId>& soundId = boost::none, EmitterType entity_type = EmitterType::Any, const boost::optional<world::ObjectId>& entity_ID = boost::none) const;
    boost::optional<size_t> getFreeStream() const;
    // Update routine for all streams. Should be placed into main loop.
    void updateStreams();
    bool trackAlreadyPlayed(uint32_t track_index, int8_t mask);
    void updateSources();
    void updateAudio();
    // General soundtrack playing routine. All native TR CD triggers and commands should ONLY
    // call this one.
    StreamError streamPlay(const uint32_t track_index, const uint8_t mask);
    bool deInitDelay();
    // If exist, immediately stop and destroy all effects with given parameters.
    Error kill(audio::SoundId soundId, EmitterType entityType = EmitterType::Global, const boost::optional<world::ObjectId>& entityId = boost::none);
    bool isInRange(EmitterType entityType, const boost::optional<world::ObjectId>& entityId, float range, float gain);
    Error send(const boost::optional<SoundId>& soundId, EmitterType entityType = EmitterType::Global, const boost::optional<world::ObjectId>& entityId = boost::none);

    ALuint getBuffer(size_t index) const
    {
        BOOST_ASSERT(index < m_buffers.size());
        return m_buffers[index];
    }

    size_t getBufferCount() const
    {
        return m_buffers.size();
    }

    const Emitter& getEmitter(size_t index) const
    {
        BOOST_ASSERT(index < m_emitters.size());
        return m_emitters[index];
    }

    // General damping update procedure. Constantly checks if damp condition exists, and
    // if so, it lowers the volume of tracks which are dampable.
    // FIXME Should be moved to engine
    void updateStreamsDamping()
    {
        StreamTrack::damp_active = false;   // Reset damp activity flag.

        // Scan for any tracks that can provoke damp. Usually it's any tracks that are
        // NOT background. So we simply check this condition and set damp activity flag
        // if condition is met.

        for(const StreamTrack& track : m_tracks)
        {
            if(track.isPlaying())
            {
                if(!track.isType(StreamType::Background))
                {
                    StreamTrack::damp_active = true;
                    return; // No need to check more, we found at least one condition.
                }
            }
        }
    }

    void clear()
    {
        if(!m_buffers.empty())
        {
            alDeleteBuffers(m_buffers.size(), m_buffers.data());
            DEBUG_CHECK_AL_ERROR();
            m_buffers.clear();
        }
        m_effects.clear();
        m_trackMap.clear();
    }

    size_t getSoundIdMapSize() const
    {
        return m_soundIdMap.size();
    }

    bool isBufferMapped(size_t index) const
    {
        BOOST_ASSERT(index < m_soundIdMap.size());
        return m_soundIdMap[index] >= 0 && static_cast<size_t>(m_soundIdMap[index]) < m_effects.size();
    }

    size_t getMappedSampleCount(size_t index) const
    {
        BOOST_ASSERT(isBufferMapped(index));
        return m_effects[m_soundIdMap[index]].sample_count;
    }

    void load(const world::World& world, const std::unique_ptr<loader::Level>& tr);

    const Settings& getSettings() const
    {
        return m_settings;
    }

    Settings& settings()
    {
        return m_settings;
    }

    void loadSampleOverrideInfo();

    // MAX_CHANNELS defines maximum amount of sound sources (channels)
    // that can play at the same time. Contemporary devices can play
    // up to 256 channels, but we set it to 32 for compatibility
    // reasons.

    static constexpr int MaxChannels = 32;

    // NUMSOURCES tells the engine how many sources we should reserve for
    // in-game music and BGMs, considering crossfades. By default, it's 6,
    // as it's more than enough for typical TR audio setup (one BGM track
    // plus one one-shot track or chat track in TR5).

    static constexpr int StreamSourceCount = 6;

    const FxManager& getFxManager() const
    {
        return m_fxManager;
    }

    FxManager& getFxManager()
    {
        return m_fxManager;
    }

private:
    // MAP_SIZE is similar to sound map size, but it is used to mark
    // already played audiotracks. Note that audiotracks CAN play several
    // times, if they were consequently called with increasing activation
    // flags (e.g., at first we call it with 00001 flag, then with 00101,
    // and so on). If all activation flags were set, including only once
    // flag, audiotrack won't play anymore.

    static constexpr int StreamMapSize = 256;

    engine::Engine* m_engine;
    Settings m_settings; // Must be initialized *before* anything else
    DeviceManager m_deviceManager; // ... and this must be initialized *after* the settings, but *before* any other things

    FxManager m_fxManager; // Must be initialized *after* m_settings

    std::vector<Emitter> m_emitters;        //!< Audio emitters.
    std::vector<int16_t> m_soundIdMap;       //!< Effect indexes.
    std::vector<Effect> m_effects;          //!< Effects and their parameters.

    std::vector<ALuint> m_buffers;          //!< Samples.
    std::vector<Source> m_sources;          //!< Channels.
    std::vector<StreamTrack> m_tracks;      //!< Stream tracks.
    std::vector<uint8_t> m_trackMap;        //!< Stream track flag map.

    glm::vec3 m_listenerPosition = { 0,0,0 };

    bool loadALbufferFromFile(ALuint buf_number, const std::string& fname);
};
} // namespace audio
