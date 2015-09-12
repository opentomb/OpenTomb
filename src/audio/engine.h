#pragma once

#include "audio.h"
#include "effect.h"
#include "emitter.h"
#include "fxmanager.h"
#include "settings.h"
#include "source.h"
#include "streamtrack.h"

namespace audio
{

class Engine
{
public:
    void pauseAllSources();
    void stopAllSources();
    void resumeAllSources();
    int getFreeSource() const;
    bool endStreams(StreamType stream_type = StreamType::Any);
    bool stopStreams(StreamType stream_type = StreamType::Any);
    bool isTrackPlaying(int32_t track_index = -1) const;
    int findSource(int effect_ID = -1, EmitterType entity_type = EmitterType::Any, int entity_ID = -1) const;
    int getFreeStream() const;
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
    Error kill(int effect_ID, EmitterType entity_type = EmitterType::Global, int entity_ID = 0);
    bool isInRange(EmitterType entity_type, int entity_ID, float range, float gain);
    Error send(int effect_ID, EmitterType entity_type = EmitterType::Global, int entity_ID = 0);

    ALuint getBuffer(size_t index) const
    {
        assert( index < m_buffers.size() );
        return m_buffers[index];
    }

    size_t getBufferCount() const
    {
        return m_buffers.size();
    }

    const Emitter& getEmitter(size_t index) const
    {
        assert( index < m_emitters.size() );
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
        assert( index < m_effectMap.size() );
        return m_effectMap[index]>=0 && static_cast<size_t>(m_effectMap[index])<m_effects.size();
    }

    size_t getMappedSampleCount(size_t index) const
    {
        assert( isBufferMapped(index) );
        return m_effects[m_effectMap[index]].sample_count;
    }

    void load(const world::World *world, const std::unique_ptr<loader::Level>& tr);

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
            throw std::runtime_error("FX Manager not initialized");
        return *m_fxManager;
    }

    void init(uint32_t num_Sources = MaxChannels)
    {
        // FX should be inited first, as source constructor checks for FX slot to be created.

        if(m_settings.use_effects)
        {
            m_fxManager.reset(new FxManager(true));
        }

        // Generate new source array.

        num_Sources -= StreamSourceCount;          // Subtract sources reserved for music.
        setSourceCount(num_Sources);

        // Generate stream tracks array.

        setStreamTrackCount(StreamSourceCount);

        // Reset last room type used for assigning reverb.

        m_fxManager->last_room_type = TR_AUDIO_FX_LASTINDEX;
    }

private:
    std::vector<Emitter> m_emitters;        //!< Audio emitters.
    std::vector<int16_t> m_effectMap;       //!< Effect indexes.
    std::vector<Effect> m_effects;          //!< Effects and their parameters.

    std::vector<ALuint> m_buffers;          //!< Samples.
    std::vector<Source> m_sources;          //!< Channels.
    std::vector<StreamTrack> m_tracks;      //!< Stream tracks.
    std::vector<uint8_t> m_trackMap;        //!< Stream track flag map.

    btVector3 m_listenerPosition = {0,0,0};

    Settings m_settings;

    std::unique_ptr<FxManager> m_fxManager{ new FxManager() };
};

} // namespace audio
