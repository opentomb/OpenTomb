#include "streamtrack.h"

#include "alext.h"
#include "audio.h"
#include "engine/engine.h"
#include "fxmanager.h"
#include "gui/console.h"
#include "settings.h"
#include "strings.h"
#include "util/helpers.h"

#include <boost/log/trivial.hpp>

using gui::Console;

namespace audio
{
// Crossfades for different track types are also different,
// since background ones tend to blend in smoothly, while one-shot
// tracks should be switched fastly.
constexpr float CrossfadeOneshot = 1 / world::animation::GameLogicFrameRate / 0.3f;
constexpr float CrossfadeBackground = 1 / world::animation::GameLogicFrameRate / 1.0f;
constexpr float CrossfadeChat = 1 / world::animation::GameLogicFrameRate / 0.1f;

// Damp coefficient specifies target volume level on a tracks
// that are being silenced (background music). The larger it is, the bigger
// silencing is.
constexpr float StreamDampLevel = 0.6f;

// Damp fade speed is used when dampable track is either being
// damped or un-damped.
constexpr float StreamDampSpeed = 1 / world::animation::GameLogicFrameRate;

// CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
// info is placed. Also CDAUDIO count specifies static amount of tracks existing
// in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
constexpr int WADStride     = 268;
constexpr int WADNameLength = 260;
constexpr int WADCount      = 130;


bool StreamTrack::damp_active = false;


StreamTrack::StreamTrack()
{
    alGenBuffers(StreamBufferCount, m_buffers);              // Generate all buffers at once.
    alGenSources(1, &m_source);
    m_format = 0x00;
    m_rate = 0;
    m_dampable = false;

    m_wadFile = nullptr;
    m_sndFile = nullptr;

    if(alIsSource(m_source))
    {
        alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, -1.0f); // OpenAL tut says this.
        alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
        alSourcef(m_source, AL_ROLLOFF_FACTOR, 0.0f);
        alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcei(m_source, AL_LOOPING, AL_FALSE); // No effect, but just in case...

        m_currentTrack = boost::none;
        m_currentVolume = 0.0f;
        m_dampedVolume = 0.0f;
        m_active = false;
        m_ending = false;
        m_streamType = StreamType::Oneshot;

        // Setting method to -1 at init is required to prevent accidental
        // ov_clear call, which results in crash, if no vorbis file was
        // associated with given vorbis file structure.

        m_method = StreamMethod::Any;
    }
}

StreamTrack::~StreamTrack()
{
    stop(); // In case we haven't stopped yet.

    alDeleteSources(1, &m_source);
    alDeleteBuffers(StreamBufferCount, m_buffers);
}

bool StreamTrack::load(const char *path, size_t index, const StreamType type, const StreamMethod load_method)
{
    if(path == nullptr)
    {
        return false;   // Do not load, if path, type or method are incorrect.
    }

    m_currentTrack = index;
    m_streamType = type;
    m_method = load_method;
    m_dampable = m_streamType == StreamType::Background;   // Damp only looped (BGM) tracks.

    // Select corresponding stream loading method.

    if(m_method == StreamMethod::Track)
    {
        return loadTrack(path);
    }
    else
    {
        return loadWad(static_cast<uint8_t>(index), path);
    }
}

bool StreamTrack::unload()
{
    bool result = false;

    if(alIsSource(m_source))  // Stop and unlink all associated buffers.
    {
        int queued;
        alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);

        while(queued--)
        {
            ALuint buffer;
            alSourceUnqueueBuffers(m_source, 1, &buffer);
        }
    }

    if(m_sndFile)
    {
        sf_close(m_sndFile);
        m_sndFile = nullptr;
        result = true;
    }

    if(m_wadFile)
    {
        fclose(m_wadFile);
        m_wadFile = nullptr;
        result = true;
    }

    return result;
}

bool StreamTrack::loadTrack(const char *path)
{
    memset(&m_sfInfo, 0, sizeof(m_sfInfo));
    m_sndFile = sf_open(path, SFM_READ, &m_sfInfo);
    if(m_sndFile == nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Load_Track: Couldn't open file: " << path;
        m_method = StreamMethod::Any;    // T4Larson <t4larson@gmail.com>: stream is uninitialised, avoid clear.
        return false;
    }

    Console::instance().notify(SYSNOTE_TRACK_OPENED, path, m_sfInfo.channels, m_sfInfo.samplerate);

#ifdef AUDIO_OPENAL_FLOAT
    if(m_sfInfo.channels == 1)
        m_format = AL_FORMAT_MONO_FLOAT32;
    else
        m_format = AL_FORMAT_STEREO_FLOAT32;
#else
    if(m_sfInfo.channels == 1)
        m_format = AL_FORMAT_MONO16;
    else
        m_format = AL_FORMAT_STEREO16;
#endif

    m_rate = m_sfInfo.samplerate;

    return true;    // Success!
}

bool StreamTrack::loadWad(uint8_t index, const char* filename)
{
    if(index >= WADCount)
    {
        Console::instance().warning(SYSWARN_WAD_OUT_OF_BOUNDS, WADCount);
        return false;
    }
    else
    {
        m_wadFile = fopen(filename, "rb");

        if(!m_wadFile)
        {
            Console::instance().warning(SYSWARN_FILE_NOT_FOUND, filename);
            return false;
        }
        else
        {
            char track_name[WADNameLength];
            uint32_t offset = 0;
            uint32_t length = 0;

            setbuf(m_wadFile, nullptr);
            fseek(m_wadFile, index * WADStride, 0);
            fread(static_cast<void*>(track_name), WADNameLength, 1, m_wadFile);
            fread(static_cast<void*>(&length), sizeof(uint32_t), 1, m_wadFile);
            fread(static_cast<void*>(&offset), sizeof(uint32_t), 1, m_wadFile);

            fseek(m_wadFile, offset, 0);
            
            m_sndFile = sf_open_fd(fileno(m_wadFile), SFM_READ, &m_sfInfo, false);
            if(m_sndFile == nullptr)
            {
                Console::instance().warning(SYSWARN_WAD_SEEK_FAILED, offset);
                m_method = StreamMethod::Any;
                return false;
            }
            else
            {
                Console::instance().notify(SYSNOTE_WAD_PLAYING, filename, offset, length);
                Console::instance().notify(SYSNOTE_TRACK_OPENED, track_name,
                                               m_sfInfo.channels, m_sfInfo.samplerate);
            }

#ifdef AUDIO_OPENAL_FLOAT
            if(m_sfInfo.channels == 1)
                m_format = AL_FORMAT_MONO_FLOAT32;
            else
                m_format = AL_FORMAT_STEREO_FLOAT32;
#else
            if(m_sfInfo.channels == 1)
                m_format = AL_FORMAT_MONO16;
            else
                m_format = AL_FORMAT_STEREO16;
#endif

            m_rate = m_sfInfo.samplerate;

            return true;    // Success!
        }
    }
}

bool StreamTrack::play(FxManager& manager, bool fade_in)
{
    int buffers_to_play = 0;

    // At start-up, we fill all available buffers.
    // TR soundtracks contain a lot of short tracks, like Lara speech etc., and
    // there is high chance that such short tracks won't fill all defined buffers.
    // For this reason, we count amount of filled buffers, and immediately stop
    // allocating them as long as Stream() routine returns false. Later, we use
    // this number for queuing buffers to source.

    for(int i = 0; i < StreamBufferCount; i++, buffers_to_play++)
    {
        if(!stream(m_buffers[i]))
        {
            if(!i)
            {
                BOOST_LOG_TRIVIAL(debug) << "StreamTrack: error preparing buffers";
                return false;
            }
            else
            {
                break;
            }
        }
    }

    if(fade_in)     // If fade-in flag is set, do it.
    {
        m_currentVolume = 0.0;
    }
    else
    {
        m_currentVolume = 1.0;
    }

    if(engine::engine_world.audioEngine.getSettings().use_effects)
    {
        if(m_streamType == StreamType::Chat)
        {
            setFX(manager);
        }
        else
        {
            unsetFX();
        }
    }

    alSourcef(m_source, AL_GAIN, m_currentVolume * engine::engine_world.audioEngine.getSettings().music_volume);
    alSourceQueueBuffers(m_source, buffers_to_play, m_buffers);
    alSourcePlay(m_source);

    m_ending = false;
    m_active = true;
    return   true;
}

void StreamTrack::pause()
{
    if(alIsSource(m_source))
        alSourcePause(m_source);
}

void StreamTrack::end()     // Smoothly end track with fadeout.
{
    m_ending = true;
}

void StreamTrack::stop()    // Immediately stop track.
{
    if(alIsSource(m_source))  // Stop and unlink all associated buffers.
    {
        if(isPlaying()) alSourceStop(m_source);
    }
}

bool StreamTrack::update()
{
    int  processed = 0;
    bool buffered = true;
    bool change_gain = false;

    if(!m_active) return true; // Nothing to do here.

    if(!isPlaying())
    {
        unload();
        m_active = false;
        return true;
    }

    // Update damping, if track supports it.

    if(m_dampable)
    {
        // We check if damp condition is active, and if so, is it already at low-level or not.

        if(damp_active && m_dampedVolume < StreamDampLevel)
        {
            m_dampedVolume += StreamDampSpeed;

            // Clamp volume.
            m_dampedVolume = glm::clamp(m_dampedVolume, 0.0f, StreamDampLevel);
            change_gain   = true;
        }
        else if(!damp_active && m_dampedVolume > 0)    // If damp is not active, but it's still at low, restore it.
        {
            m_dampedVolume -= StreamDampSpeed;

            // Clamp volume.
            m_dampedVolume = glm::clamp(m_dampedVolume, 0.0f, StreamDampLevel);
            change_gain   = true;
        }
    }

    if(m_ending)     // If track is ending, crossfade it.
    {
        switch(m_streamType)
        {
            case StreamType::Any:
                BOOST_ASSERT(false);
                break;

            case StreamType::Background:
                m_currentVolume -= CrossfadeBackground;
                break;

            case StreamType::Oneshot:
                m_currentVolume -= CrossfadeOneshot;
                break;

            case StreamType::Chat:
                m_currentVolume -= CrossfadeChat;
                break;
        }

        // Crossfade has ended, we can now kill the stream.
        if(m_currentVolume <= 0.0)
        {
            stop();
            return true;    // Stop track, although return success, as everything is normal.
        }
        else
        {
            change_gain = true;
        }
    }
    else
    {
        // If track is not ending and playing, restore it from crossfade.
        if(m_currentVolume < 1.0)
        {
            switch(m_streamType)
            {
                case StreamType::Any:
                    BOOST_ASSERT(false);
                    break;

                case StreamType::Background:
                    m_currentVolume += CrossfadeBackground;
                    break;

                case StreamType::Oneshot:
                    m_currentVolume += CrossfadeOneshot;
                    break;

                case StreamType::Chat:
                    m_currentVolume += CrossfadeChat;
                    break;
            }

            // Clamp volume.
            m_currentVolume = glm::clamp(m_currentVolume, 0.0f, 1.0f);
            change_gain    = true;
        }
    }

    if(change_gain) // If any condition which modify track gain was met, call AL gain change.
    {
        alSourcef(m_source, AL_GAIN, m_currentVolume              *  // Global track volume.
                  (1.0f - m_dampedVolume)       *  // Damp volume.
                  engine::engine_world.audioEngine.getSettings().music_volume);  // Global music volume setting.
    }

    // Check if any track buffers were already processed.

    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)  // Manage processed buffers.
    {
        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);     // Unlink processed buffer.
        buffered = stream(buffer);                      // Refill processed buffer.
        if(buffered)
            alSourceQueueBuffers(m_source, 1, &buffer);   // Relink processed buffer.
    }

    return buffered;
}

bool StreamTrack::isTrack(size_t track_index) const    // Check if track has specific index.
{
    return m_currentTrack && *m_currentTrack == track_index;
}

bool StreamTrack::isType(const StreamType track_type) const      // Check if track has specific type.
{
    return track_type == m_streamType;
}

bool StreamTrack::isActive() const                         // Check if track is still active.
{
    return m_active;
}

bool StreamTrack::isDampable() const                      // Check if track is dampable.
{
    return m_dampable;
}

bool StreamTrack::isPlaying() const                       // Check if track is playing.
{
    if(alIsSource(m_source))
    {
        ALenum state = AL_STOPPED;
        alGetSourcei(m_source, AL_SOURCE_STATE, &state);

        // Paused state and existing file pointers also counts as playing.
        return state == AL_PLAYING || state == AL_PAUSED;
    }
    else
    {
        return false;
    }
}

bool StreamTrack::stream(ALuint buffer)
{
    BOOST_ASSERT(engine::engine_world.audioEngine.getSettings().stream_buffer_size >= m_sfInfo.channels - 1);
#ifdef AUDIO_OPENAL_FLOAT
    std::vector<ALfloat> pcm(engine::engine_world.audioEngine.getSettings().stream_buffer_size);
#else
    std::vector<ALshort> pcm(engine::engine_world.audioEngine.getSettings().stream_buffer_size);
#endif
    size_t size = 0;

    // SBS - C + 1 is important to avoid endless loops if the buffer size isn't a multiple of the channels
    while(size < pcm.size() - m_sfInfo.channels + 1)
    {
        // we need to read a multiple of sf_info.channels here
        const size_t samplesToRead = (engine::engine_world.audioEngine.getSettings().stream_buffer_size - size) / m_sfInfo.channels * m_sfInfo.channels;
#ifdef AUDIO_OPENAL_FLOAT
        const sf_count_t samplesRead = sf_read_float(m_sndFile, pcm.data() + size, samplesToRead);
#else
        const sf_count_t samplesRead = sf_read_short(m_sndFile, pcm.data() + size, samplesToRead);
#endif

        if(samplesRead > 0)
        {
            BOOST_ASSERT(samplesRead <= std::numeric_limits<size_t>::max());
            size += static_cast<size_t>(samplesRead);
        }
        else
        {
            int error = sf_error(m_sndFile);
            if(error != SF_ERR_NO_ERROR)
            {
                logSndfileError(error);
                return false;
            }
            else
            {
                if(m_streamType == StreamType::Background)
                {
                    sf_seek(m_sndFile, 0, SEEK_SET);
                }
                else
                {
                    break;   // Stream is ending - do nothing.
                }
            }
        }
    }

    if(size == 0)
        return false;

    alBufferData(buffer, m_format, pcm.data(), static_cast<ALsizei>(size * sizeof(pcm[0])), m_rate);
    return true;
}

void StreamTrack::setFX(FxManager& manager)
{
    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    ALuint slot;
    if(manager.current_room_type != manager.last_room_type)  // Switch audio send.
    {
        manager.last_room_type = manager.current_room_type;
        manager.current_slot = ++manager.current_slot > FxManager::MaxSlots - 1 ? 0 : manager.current_slot;

        ALuint effect = manager.al_effect[static_cast<int>(manager.current_room_type)];
        slot = manager.al_slot[manager.current_slot];

        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = manager.al_slot[manager.current_slot];
    }

    // Assign global reverb FX to channel.

    alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}

void StreamTrack::unsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(m_source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
}

} // namespace audio
