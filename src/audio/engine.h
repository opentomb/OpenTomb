#pragma once

#include "effect.h"
#include "emitter.h"
#include "fxmanager.h"
#include "settings.h"
#include "source.h"
#include "streamtrack.h"
#include "world/object.h"

#include <memory>

#include <AL/alc.h>
#include <boost/assert.hpp>
#include <boost/optional.hpp>

namespace world
{
struct World;
} // namespace world

namespace loader
{
class Level;
} // namespace loader

namespace audio
{
// Certain sound effect indexes were changed across different TR
// versions, despite remaining the same - mostly, it happened with
// menu sounds and some general sounds. For such effects, we specify
// additional remap enumeration list, which is fed into Lua script
// to get actual effect ID for current game version.

enum TR_AUDIO_SOUND_GLOBALID
{
    TR_AUDIO_SOUND_GLOBALID_MENUOPEN,
    TR_AUDIO_SOUND_GLOBALID_MENUCLOSE,
    TR_AUDIO_SOUND_GLOBALID_MENUROTATE,
    TR_AUDIO_SOUND_GLOBALID_MENUPAGE,
    TR_AUDIO_SOUND_GLOBALID_MENUSELECT,
    TR_AUDIO_SOUND_GLOBALID_MENUWEAPON,
    TR_AUDIO_SOUND_GLOBALID_MENUCLANG,
    TR_AUDIO_SOUND_GLOBALID_MENUAUDIOTEST,
    TR_AUDIO_SOUND_GLOBALID_LASTINDEX
};

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
public:
    void pauseAllSources();
    void stopAllSources();
    void resumeAllSources();
    boost::optional<size_t> getFreeSource() const;
    bool endStreams(StreamType stream_type = StreamType::Any);
    bool stopStreams(StreamType stream_type = StreamType::Any);
    bool isTrackPlaying(const boost::optional<int32_t>& track_index = boost::none) const;
    boost::optional<size_t> findSource(const boost::optional<uint32_t>& effect_ID = boost::none, EmitterType entity_type = EmitterType::Any, const boost::optional<world::ObjectId>& entity_ID = boost::none) const;
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
    void deInitAudio();
    // If exist, immediately stop and destroy all effects with given parameters.
    Error kill(int effect_ID, EmitterType entity_type = EmitterType::Global, world::ObjectId entity_ID = 0);
    bool isInRange(EmitterType entity_type, world::ObjectId entity_ID, float range, float gain);
    Error send(const boost::optional<uint32_t>& effect_ID, EmitterType entity_type = EmitterType::Global, world::ObjectId entity_ID = 0);

    ALuint getBuffer(size_t index) const
    {
        BOOST_ASSERT( index < m_buffers.size() );
        return m_buffers[index];
    }

    size_t getBufferCount() const
    {
        return m_buffers.size();
    }

    const Emitter& getEmitter(size_t index) const
    {
        BOOST_ASSERT( index < m_emitters.size() );
        return m_emitters[index];
    }

    void setSourceCount(size_t count)
    {
        m_sources.resize(count);
    }

    void setStreamTrackCount(size_t count)
    {
        m_tracks.resize(count);
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
        m_sources.clear();
        m_buffers.clear();
        m_effects.clear();
        m_tracks.clear();
        m_trackMap.clear();
    }

    size_t getAudioMapSize() const
    {
        return m_effectMap.size();
    }

    bool isBufferMapped(size_t index) const
    {
        BOOST_ASSERT( index < m_effectMap.size() );
        return m_effectMap[index]>=0 && static_cast<size_t>(m_effectMap[index])<m_effects.size();
    }

    size_t getMappedSampleCount(size_t index) const
    {
        BOOST_ASSERT( isBufferMapped(index) );
        return m_effects[m_effectMap[index]].sample_count;
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

    void resetSettings()
    {
        m_settings = Settings();
    }

    void loadSampleOverrideInfo();

    FxManager& fxManager()
    {
        if(!m_fxManager)
            BOOST_THROW_EXCEPTION( std::runtime_error("FX Manager not initialized") );
        return *m_fxManager;
    }

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

    void init(uint32_t num_Sources = MaxChannels);
    void initDevice();
    void closeDevice();

private:
    // MAP_SIZE is similar to sound map size, but it is used to mark
    // already played audiotracks. Note that audiotracks CAN play several
    // times, if they were consequently called with increasing activation
    // flags (e.g., at first we call it with 00001 flag, then with 00101,
    // and so on). If all activation flags were set, including only once
    // flag, audiotrack won't play anymore.

    static constexpr int StreamMapSize = 256;

    std::vector<Emitter> m_emitters;        //!< Audio emitters.
    std::vector<int16_t> m_effectMap;       //!< Effect indexes.
    std::vector<Effect> m_effects;          //!< Effects and their parameters.

    std::vector<ALuint> m_buffers;          //!< Samples.
    std::vector<Source> m_sources;          //!< Channels.
    std::vector<StreamTrack> m_tracks;      //!< Stream tracks.
    std::vector<uint8_t> m_trackMap;        //!< Stream track flag map.

    glm::vec3 m_listenerPosition = {0,0,0};

    Settings m_settings;

    std::unique_ptr<FxManager> m_fxManager{ new FxManager() };

    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
};

} // namespace audio
