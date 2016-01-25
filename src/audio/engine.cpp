#include "engine.h"

#include "alext.h"
#include "audio.h"
#include "engine/engine.h"
#include "gui/console.h"
#include "loader/level.h"
#include "settings.h"
#include "script/script.h"
#include "strings.h"
#include "world/entity.h"

#include <chrono>

#include <glm/gtc/type_ptr.hpp>
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using gui::Console;

namespace audio
{
// Audio de-initialization delay gives some time to OpenAL to shut down its
// currently active sources. If timeout is reached, it means that something is
// really wrong with audio subsystem; usually five seconds is enough.
constexpr float AudioDeinitDelay = 5.0f;

namespace
{
bool fillALBuffer(ALuint buf_number, SNDFILE *wavFile, Uint32 frameCount, SF_INFO *sfInfo)
{
    if(sfInfo->channels > 1)   // We can't use non-mono samples.
    {
        BOOST_LOG_TRIVIAL(error) << "Sample " << buf_number << " is not mono";
        return false;
    }

#ifdef AUDIO_OPENAL_FLOAT
    std::vector<ALfloat> frames(frameCount);
    /*const sf_count_t samplesRead =*/ sf_readf_float(wavFile, frames.data(), frames.size());

    alBufferData(buf_number, AL_FORMAT_MONO_FLOAT32, &frames.front(), frameCount * sizeof(frames[0]), sfInfo->samplerate);
#else
    std::vector<ALshort> frames(frameCount);
    /*const sf_count_t samplesRead =*/ sf_readf_short(wavFile, frames.data(), frames.size());

    alBufferData(buf_number, AL_FORMAT_MONO16, &frames.front(), frameCount * sizeof(frames[0]), sfInfo->samplerate);
#endif
    checkALError(__FUNCTION__);
    return true;
}

bool loadALbufferFromMem(ALuint buf_number, uint8_t *sample_pointer, size_t sample_size, size_t uncomp_sample_size = 0)
{
    struct MemBufferFileIo : public SF_VIRTUAL_IO
    {
        MemBufferFileIo(const uint8_t* data, sf_count_t dataSize)
            : SF_VIRTUAL_IO()
            , m_data(data)
            , m_dataSize(dataSize)
        {
            BOOST_ASSERT(data != nullptr);

            get_filelen = &MemBufferFileIo::getFileLength;
            seek = &MemBufferFileIo::doSeek;
            read = &MemBufferFileIo::doRead;
            write = &MemBufferFileIo::doWrite;
            tell = &MemBufferFileIo::doTell;
        }

        static sf_count_t getFileLength(void *user_data)
        {
            auto self = static_cast<MemBufferFileIo*>(user_data);
            return self->m_dataSize;
        }

        static sf_count_t doSeek(sf_count_t offset, int whence, void *user_data)
        {
            auto self = static_cast<MemBufferFileIo*>(user_data);
            switch(whence)
            {
                case SEEK_SET:
                    BOOST_ASSERT(offset >= 0 && offset <= self->m_dataSize);
                    self->m_where = offset;
                    break;
                case SEEK_CUR:
                    BOOST_ASSERT(self->m_where + offset <= self->m_dataSize && self->m_where + offset >= 0);
                    self->m_where += offset;
                    break;
                case SEEK_END:
                    BOOST_ASSERT(offset >= 0 && offset <= self->m_dataSize);
                    self->m_where = self->m_dataSize - offset;
                    break;
                default:
                    BOOST_ASSERT(false);
            }
            return self->m_where;
        }

        static sf_count_t doRead(void *ptr, sf_count_t count, void *user_data)
        {
            auto self = static_cast<MemBufferFileIo*>(user_data);
            if(self->m_where + count > self->m_dataSize)
                count = self->m_dataSize - self->m_where;

            BOOST_ASSERT(self->m_where + count <= self->m_dataSize);

            uint8_t* buf = static_cast<uint8_t*>(ptr);
            std::copy(self->m_data + self->m_where, self->m_data + self->m_where + count, buf);
            self->m_where += count;
            return count;
        }

        static sf_count_t doWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
        {
            return 0; // read-only
        }

        static sf_count_t doTell(void *user_data)
        {
            auto self = static_cast<MemBufferFileIo*>(user_data);
            return self->m_where;
        }

    private:
        const uint8_t* const m_data;
        const sf_count_t m_dataSize;
        sf_count_t m_where = 0;
    };

    MemBufferFileIo wavMem(sample_pointer, sample_size);
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    SNDFILE* sample = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

    if(!sample)
    {
        BOOST_LOG_TRIVIAL(error) << "Can't load sample #" << buf_number << " from sample block";
        return false;
    }

    // Uncomp_sample_size explicitly specifies amount of raw sample data
    // to load into buffer. It is only used in TR4/5 with ADPCM samples,
    // because full-sized ADPCM sample contains a bit of silence at the end,
    // which should be removed. That's where uncomp_sample_size comes into
    // business.
    // Note that we also need to compare if uncomp_sample_size is smaller
    // than native wav length, because for some reason many TR5 uncomp sizes
    // are messed up and actually more than actual sample size.

    size_t real_size = sfInfo.frames * sizeof(uint16_t);

    if(uncomp_sample_size == 0 || real_size < uncomp_sample_size)
    {
        uncomp_sample_size = real_size;
    }

    // We need to change buffer size, as we're using floats here.

    const auto frameCount = uncomp_sample_size / sizeof(uint16_t);

    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats in same level.

    bool result = fillALBuffer(buf_number, sample, static_cast<Uint32>(frameCount), &sfInfo);

    sf_close(sample);

    return result;   // Zero means success.
}

bool loadALbufferFromFile(ALuint buf_number, const std::string& fname)
{
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(fname.c_str(), SFM_READ, &sfInfo);

    if(!file)
    {
        Console::instance().warning(SYSWARN_CANT_OPEN_FILE);
        BOOST_LOG_TRIVIAL(error) << "SndFile cannot open file '" << fname << "': " << sf_strerror(file);
        return false;
    }

    bool result = fillALBuffer(buf_number, file, static_cast<Uint32>(sfInfo.frames), &sfInfo);

    sf_close(file);

    return result;   // Zero means success.
}
} // anonymous namespace

void Engine::pauseAllSources()
{
    for(Source& source : m_sources)
    {
        if(source.isActive())
        {
            source.pause();
        }
    }
}

void Engine::stopAllSources()
{
    for(Source& source : m_sources)
    {
        source.stop();
    }
}

void Engine::resumeAllSources()
{
    for(Source& source : m_sources)
    {
        if(source.isActive())
        {
            source.play(fxManager());
        }
    }
}

boost::optional<size_t> Engine::getFreeSource() const  ///@FIXME: add condition (compare max_dist with new source dist)
{
    for(size_t i = 0; i < m_sources.size(); i++)
    {
        if(!m_sources[i].isActive())
        {
            return i;
        }
    }

    return boost::none;
}

bool Engine::endStreams(StreamType stream_type)
{
    bool result = false;

    for(StreamTrack& track : m_tracks)
    {
        if(stream_type == StreamType::Any ||                              // End ALL streams at once.
           (track.isPlaying() &&
            track.isType(stream_type)))
        {
            result = true;
            track.fadeOutAndStop();
        }
    }

    return result;
}

bool Engine::stopStreams(StreamType stream_type)
{
    bool result = false;

    for(StreamTrack& track : m_tracks)
    {
        if(track.isPlaying() &&
           (track.isType(stream_type) ||
            stream_type == StreamType::Any)) // Stop ALL streams at once.
        {
            result = true;
            track.stop();
        }
    }

    return result;
}

bool Engine::isTrackPlaying(const boost::optional<int32_t>& track_index) const
{
    for(const StreamTrack& track : m_tracks)
    {
        if((!track_index || track.isTrack(*track_index)) && track.isPlaying())
        {
            return true;
        }
    }

    return false;
}

boost::optional<size_t> Engine::findSource(const boost::optional<SoundId>& soundId, EmitterType entity_type, const boost::optional<world::ObjectId>& entityId) const
{
    for(size_t i = 0; i < m_sources.size(); i++)
    {
        if((entity_type == EmitterType::Any || m_sources[i].getEmitterType() == entity_type)
           && (!entityId || m_sources[i].getEmitterId() == *entityId)
           && (!soundId || m_sources[i].getSoundId() == *soundId))
        {
            if(m_sources[i].isPlaying())
                return i;
        }
    }

    return boost::none;
}

boost::optional<size_t> Engine::getFreeStream() const
{
    for(uint32_t i = 0; i < m_tracks.size(); i++)
    {
        if(!m_tracks[i].isPlaying() && !m_tracks[i].isActive())
        {
            return i;
        }
    }

    return boost::none;  // If no free source, return error.
}

void Engine::updateStreams()
{
    updateStreamsDamping();

    for(StreamTrack& track : m_tracks)
    {
        track.update();
    }
}

bool Engine::trackAlreadyPlayed(uint32_t track_index, int8_t mask)
{
    if(!mask)
    {
        return false;   // No mask, play in any case.
    }

    if(track_index >= m_trackMap.size())
    {
        return true;    // No such track, hence "already" played.
    }

    mask &= 0x3F;   // Clamp mask just in case.

    if(m_trackMap[track_index] == mask)
    {
        return true;    // Immediately return true, if flags are directly equal.
    }

    int8_t played = m_trackMap[track_index] & mask;
    if(played == mask)
    {
        return true;    // Bits were set, hence already played.
    }

    m_trackMap[track_index] |= mask;
    return false;   // Not yet played, set bits and return false.
}

void Engine::updateSources()
{
    if(m_sources.empty())
    {
        return;
    }

    alGetListenerfv(AL_POSITION, glm::value_ptr(m_listenerPosition));

    for(uint32_t i = 0; i < m_emitters.size(); i++)
    {
        send(m_emitters[i].soundId, EmitterType::SoundSource, i);
    }

    for(Source& src : m_sources)
    {
        src.update(fxManager());
    }
}

void Engine::updateAudio()
{
    updateSources();
    updateStreams();

    if(m_settings.listener_is_player)
    {
        m_fxManager->updateListener(*engine::Engine::instance.m_world.m_character);
    }
    else
    {
        m_fxManager->updateListener(*render::renderer.camera());
    }
}

StreamError Engine::streamPlay(const uint32_t track_index, const uint8_t mask)
{
    StreamMethod load_method = StreamMethod::Any;
    StreamType stream_type = StreamType::Any;

    char   file_path[256];          // Should be enough, and this is not the full path...

    // Don't even try to do anything with track, if its index is greater than overall amount of
    // soundtracks specified in a stream track map count (which is derived from script).

    if(track_index >= m_trackMap.size())
    {
        Console::instance().warning(SYSWARN_TRACK_OUT_OF_BOUNDS, track_index);
        return StreamError::WrongTrack;
    }

    // Don't play track, if it is already playing.
    // This should become useless option, once proper one-shot trigger functionality is implemented.

    if(isTrackPlaying(track_index))
    {
        Console::instance().warning(SYSWARN_TRACK_ALREADY_PLAYING, track_index);
        return StreamError::Ignored;
    }

    // lua_GetSoundtrack returns stream type, file path and load method in last three
    // provided arguments. That is, after calling this function we receive stream type
    // in "stream_type" argument, file path into "file_path" argument and load method into
    // "load_method" argument. Function itself returns false, if script wasn't found or
    // request was broken; in this case, we quit.

    if(!engine_lua.getSoundtrack(track_index, file_path, &load_method, &stream_type))
    {
        Console::instance().warning(SYSWARN_TRACK_WRONG_INDEX, track_index);
        return StreamError::WrongTrack;
    }

    // Don't try to play track, if it was already played by specified bit mask.
    // Additionally, TrackAlreadyPlayed function applies specified bit mask to track map.
    // Also, bit mask is valid only for non-looped tracks, since looped tracks are played
    // in any way.

    if(stream_type != StreamType::Background &&
       trackAlreadyPlayed(track_index, mask))
    {
        return StreamError::Ignored;
    }

    // Entry found, now process to actual track loading.

    boost::optional<size_t> target_stream = getFreeStream();            // At first, we need to get free stream.

    bool do_fade_in;
    if(!target_stream)
    {
        do_fade_in = stopStreams(stream_type);  // If no free track found, hardly stop all tracks.
        target_stream = getFreeStream();        // Try again to assign free stream.

        if(!target_stream)
        {
            Console::instance().warning(SYSWARN_NO_FREE_STREAM);
            return StreamError::NoFreeStream;  // No success, exit and don't play anything.
        }
    }
    else
    {
        endStreams(stream_type);   // End all streams of this type with fadeout.

        // Additionally check if track type is looped. If it is, force fade in in any case.
        // This is needed to smooth out possible pop with gapless looped track at a start-up.

        do_fade_in = stream_type == StreamType::Background;
    }

    // Finally - load our track.

    if(!m_tracks[*target_stream].load(file_path, track_index, stream_type, load_method))
    {
        Console::instance().warning(SYSWARN_STREAM_LOAD_ERROR);
        return StreamError::LoadError;
    }

    // Try to play newly assigned and loaded track.

    if(!m_tracks[*target_stream].play(fxManager(), do_fade_in))
    {
        Console::instance().warning(SYSWARN_STREAM_PLAY_ERROR);
        return StreamError::PlayError;
    }

    return StreamError::Processed;   // Everything is OK!
}

bool Engine::deInitDelay()
{
    const std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();

    while(isTrackPlaying() || findSource())
    {
        auto curr_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1.0e6;

        if(curr_time > AudioDeinitDelay)
        {
            BOOST_LOG_TRIVIAL(error) << "Audio deinit timeout reached! Something is wrong with audio driver.";
            break;
        }
    }

    return true;
}

void Engine::deInitAudio()
{
    stopAllSources();
    stopStreams();

    deInitDelay();

    m_sources.clear();
    m_tracks.clear();
    m_trackMap.clear();

    ///@CRITICAL: You must delete all sources before deleting buffers!

    alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
    m_buffers.clear();

    m_effects.clear();
    m_soundIdMap.clear();

    m_fxManager.reset();
}

Error Engine::kill(audio::SoundId soundId, EmitterType entityType, const boost::optional<world::ObjectId>& entityId)
{
    boost::optional<size_t> playing_sound = findSource(soundId, entityType, entityId);

    if(playing_sound)
    {
        m_sources[*playing_sound].stop();
        return Error::Processed;
    }

    return Error::Ignored;
}

bool Engine::isInRange(EmitterType entityType, const boost::optional<world::ObjectId>& entityId, float range, float gain)
{
    glm::vec3 vec;
    switch(entityType)
    {
        case EmitterType::Entity:
            BOOST_ASSERT(entityId.is_initialized());
            if(std::shared_ptr<world::Entity> ent = engine::Engine::instance.m_world.getEntityByID(*entityId))
            {
                vec = glm::vec3(ent->m_transform[3]);
            }
            else
            {
                return false;
            }
            break;

        case EmitterType::SoundSource:
            BOOST_ASSERT(entityId.is_initialized());
            if(*entityId >= m_emitters.size())
            {
                return false;
            }
            vec = m_emitters[*entityId].position;
            break;

        case EmitterType::Global:
            return true;

        default:
            return false;
    }

    glm::float_t dist = glm::distance(m_listenerPosition, vec);
    dist *= dist;

    // We add 1/4 of overall distance to fix up some issues with
    // pseudo-looped sounds that are called at certain frames in animations.

    dist /= gain + 1.25f;

    return dist < range * range;
}

Error Engine::send(const boost::optional<SoundId>& soundId, EmitterType entityType, const boost::optional<world::ObjectId>& entityId)
{
    // If there are no audio buffers or effect index is wrong, don't process.

    if(m_buffers.empty() || !soundId)
        return Error::Ignored;

    // Remap global engine effect ID to local effect ID.

    if(*soundId >= m_soundIdMap.size())
    {
        return Error::NoSample;  // Sound is out of bounds; stop.
    }

    int real_ID = static_cast<int>(m_soundIdMap[*soundId]);

    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.

    if(real_ID < 0)
    {
        return Error::Ignored;
    }

    Effect* effect = &m_effects[real_ID];

    // Pre-step 2: check if sound non-looped and chance to play isn't zero,
    // then randomly select if it should be played or not.

    if(effect->loop != loader::LoopType::Forward && effect->chance > 0)
    {
        auto randomValue = rand() % 0x7FFF;
        if(effect->chance < randomValue)
        {
            // Bypass audio send, if chance test is not passed.
            return Error::Ignored;
        }
    }

    // Pre-step 3: Calculate if effect's hearing sphere intersect listener's hearing sphere.
    // If it's not, bypass audio send (cause we don't want it to occupy channel, if it's not
    // heard).

    if(isInRange(entityType, entityId, effect->range, effect->gain) == false)
    {
        return Error::Ignored;
    }

    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and stop it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.

    boost::optional<size_t> source_number = findSource(soundId, entityType, entityId);

    if(source_number)
    {
        if(effect->loop == loader::LoopType::PingPong)
        {
            m_sources[*source_number].stop();
        }
        else if(effect->loop != loader::LoopType::None) // Any other looping case (Wait / Loop).
        {
            return Error::Ignored;
        }
    }
    else
    {
        source_number = getFreeSource();  // Get free source.
    }

    if(!source_number)
        return Error::NoChannel;

    int buffer_index;

    // Step 1. Assign buffer to source.

    if(effect->sample_count > 1)
    {
        // Select random buffer, if effect info contains more than 1 assigned samples.
        auto randomValue = rand() % effect->sample_count;
        buffer_index = randomValue + effect->sample_index;
    }
    else
    {
        // Just assign buffer to source, if there is only one assigned sample.
        buffer_index = effect->sample_index;
    }

    Source *source = &m_sources[*source_number];

    source->setBuffer(buffer_index);

    // Step 2. Check looped flag, and if so, set source type to looped.

    if(effect->loop == loader::LoopType::Forward)
    {
        source->setLooping(AL_TRUE);
    }
    else
    {
        source->setLooping(AL_FALSE);
    }

    // Step 3. Apply internal sound parameters.

    source->set(*soundId, entityType, entityId);

    // Step 4. Apply sound effect properties.

    if(effect->rand_pitch)  // Vary pitch, if flag is set.
    {
        auto random_float = static_cast<ALfloat>(rand() % effect->rand_pitch_var);
        random_float = effect->pitch + (random_float - 25.0f) / 200.0f;
        source->setPitch(random_float);
    }
    else
    {
        source->setPitch(effect->pitch);
    }

    if(effect->rand_gain)   // Vary gain, if flag is set.
    {
        auto random_float = static_cast<ALfloat>(rand() % effect->rand_gain_var);
        random_float = effect->gain + (random_float - 25.0f) / 200.0f;
        source->setGain(random_float);
    }
    else
    {
        source->setGain(effect->gain);
    }

    source->setRange(effect->range);    // Set audible range.

    source->play(fxManager());                     // Everything is OK, play sound now!

    return Error::Processed;
}

void Engine::load(const world::World& world, const std::unique_ptr<loader::Level>& tr)
{
    // Generate new buffer array.
    m_buffers.resize(tr->m_samplesCount, 0);
    alGenBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());

    // Generate stream track map array.
    // We use scripted amount of tracks to define map bounds.
    // If script had no such parameter, we define map bounds by default.
    m_trackMap.resize(engine_lua.getNumTracks(), 0);
    if(m_trackMap.empty())
        m_trackMap.resize(StreamMapSize, 0);

    // Generate new audio effects array.
    m_effects.resize(tr->m_soundDetails.size());

    // Generate new audio emitters array.
    m_emitters.resize(tr->m_soundSources.size());

    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(!tr->m_samplesData.empty())
    {
        auto pointer = tr->m_samplesData.data();
        switch(tr->m_gameVersion)
        {
            case loader::Game::TR1:
            case loader::Game::TR1Demo:
            case loader::Game::TR1UnfinishedBusiness:
                m_soundIdMap = tr->m_soundmap;

                for(size_t i = 0; i < tr->m_sampleIndices.size() - 1; i++)
                {
                    pointer = tr->m_samplesData.data() + tr->m_sampleIndices[i];
                    uint32_t size = tr->m_sampleIndices[(i + 1)] - tr->m_sampleIndices[i];
                    loadALbufferFromMem(m_buffers[i], pointer, size);
                }
                break;

            case loader::Game::TR2:
            case loader::Game::TR2Demo:
            case loader::Game::TR3:
            {
                m_soundIdMap = tr->m_soundmap;
                size_t ind1 = 0;
                size_t ind2 = 0;
                bool flag = false;
                size_t i = 0;
                while(pointer < tr->m_samplesData.data() + tr->m_samplesData.size() - 4)
                {
                    pointer = tr->m_samplesData.data() + ind2;
                    if(0x46464952 == *reinterpret_cast<int32_t*>(pointer))
                    {
                        // RIFF
                        if(!flag)
                        {
                            ind1 = ind2;
                            flag = true;
                        }
                        else
                        {
                            size_t uncomp_size = ind2 - ind1;
                            auto* srcData = tr->m_samplesData.data() + ind1;
                            loadALbufferFromMem(m_buffers[i], srcData, uncomp_size);
                            i++;
                            if(i >= m_buffers.size())
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                size_t uncomp_size = tr->m_samplesData.size() - ind1;
                pointer = tr->m_samplesData.data() + ind1;
                if(i < m_buffers.size())
                {
                    loadALbufferFromMem(m_buffers[i], pointer, uncomp_size);
                }
                break;
            }

            case loader::Game::TR4:
            case loader::Game::TR4Demo:
            case loader::Game::TR5:
                m_soundIdMap = tr->m_soundmap;

                for(size_t i = 0; i < tr->m_samplesCount; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    size_t uncomp_size = *reinterpret_cast<uint32_t*>(pointer);
                    pointer += 4;
                    size_t comp_size = *reinterpret_cast<uint32_t*>(pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    loadALbufferFromMem(m_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                m_soundIdMap.clear();
                tr->m_samplesData.clear();
                return;
        }
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(size_t i = 0; i < m_effects.size(); i++)
    {
        if(tr->m_gameVersion < loader::Game::TR3)
        {
            m_effects[i].gain = tr->m_soundDetails[i].volume / 32767.0f; // Max. volume in TR1/TR2 is 32767.
            m_effects[i].chance = tr->m_soundDetails[i].chance;
        }
        else if(tr->m_gameVersion > loader::Game::TR3)
        {
            m_effects[i].gain = tr->m_soundDetails[i].volume / 255.0f; // Max. volume in TR3 is 255.
            m_effects[i].chance = tr->m_soundDetails[i].chance * 255;
        }
        else
        {
            m_effects[i].gain = tr->m_soundDetails[i].volume / 255.0f; // Max. volume in TR3 is 255.
            m_effects[i].chance = tr->m_soundDetails[i].chance * 127;
        }

        m_effects[i].rand_gain_var = 50;
        m_effects[i].rand_pitch_var = 50;

        m_effects[i].pitch = tr->m_soundDetails[i].pitch / 127.0f + 1.0f;
        m_effects[i].range = tr->m_soundDetails[i].sound_range * 1024.0f;

        m_effects[i].rand_pitch = tr->m_soundDetails[i].useRandomPitch();
        m_effects[i].rand_gain = tr->m_soundDetails[i].useRandomVolume();

        m_effects[i].loop = tr->m_soundDetails[i].getLoopType(loader::gameToEngine(tr->m_gameVersion));

        m_effects[i].sample_index = tr->m_soundDetails[i].sample;
        m_effects[i].sample_count = tr->m_soundDetails[i].getSampleCount();
    }

    // Try to override samples via script.
    // If there is no script entry exist, we just leave default samples.
    // NB! We need to override samples AFTER audio effects array is inited, as override
    //     routine refers to existence of certain audio effect in level.

    loadSampleOverrideInfo();

    // Hardcoded version-specific fixes!

    switch(world.m_engineVersion)
    {
        case loader::Engine::TR1:
            // Fix for underwater looped sound.
            if(m_soundIdMap[SoundUnderwater] >= 0)
            {
                m_effects[m_soundIdMap[SoundUnderwater]].loop = loader::LoopType::Forward;
            }
            break;
        case loader::Engine::TR2:
            // Fix for helicopter sound range.
            if(m_soundIdMap[297] >= 0)
            {
                m_effects[m_soundIdMap[297]].range *= 10.0;
            }
            break;
        default:
            break;
    }

    // Cycle through sound emitters and
    // parse them to native OpenTomb sound emitters structure.

    for(size_t i = 0; i < m_emitters.size(); i++)
    {
        m_emitters[i].emitter_index = static_cast<ALuint>(i);
        m_emitters[i].soundId = tr->m_soundSources[i].sound_id;
        m_emitters[i].position = glm::vec3(tr->m_soundSources[i].x, tr->m_soundSources[i].z, -tr->m_soundSources[i].y);
        m_emitters[i].flags = tr->m_soundSources[i].flags;
    }
}

void Engine::loadSampleOverrideInfo()
{
    int num_samples, num_sounds;
    std::string sample_name_mask;
    if(!engine_lua.getOverridedSamplesInfo(num_samples, num_sounds, sample_name_mask))
        return;

    size_t buffer_counter = 0;

    for(size_t i = 0; i < getBufferCount(); i++)
    {
        if(!isBufferMapped(i))
            continue;

        int sample_index, sample_count;
        if(engine_lua.getOverridedSample(i, sample_index, sample_count))
        {
            for(int j = 0; j < sample_count; j++, buffer_counter++)
            {
                std::string sampleName = (boost::format(sample_name_mask) % (sample_index + j)).str();
                if(!boost::filesystem::is_regular_file(sampleName))
                {
                    loadALbufferFromFile(getBuffer(buffer_counter), sampleName);
                }
            }
        }
        else
        {
            buffer_counter += getMappedSampleCount(i);
        }
    }
}

void Engine::init(size_t num_Sources)
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

    m_fxManager->resetLastRoomType();
}

void Engine::initDevice()
{
#ifndef NO_AUDIO

    ALCint paramList[] = {
        ALC_STEREO_SOURCES,  StreamSourceCount,
        ALC_MONO_SOURCES,   (MaxChannels - StreamSourceCount),
        ALC_FREQUENCY,       44100, 0 };

    BOOST_LOG_TRIVIAL(info) << "Probing OpenAL devices...";

    const char *devlist = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

    if(!devlist)
    {
        BOOST_LOG_TRIVIAL(warning) << "InitAL: No AL audio devices";
        return;
    }

    while(*devlist)
    {
        BOOST_LOG_TRIVIAL(info) << " Device: " << devlist;
        ALCdevice* dev = alcOpenDevice(devlist);

        if(m_settings.use_effects)
        {
            if(alcIsExtensionPresent(dev, ALC_EXT_EFX_NAME) == ALC_TRUE)
            {
                BOOST_LOG_TRIVIAL(info) << " EFX supported!";
                m_device = dev;
                m_context = alcCreateContext(m_device, paramList);
                // fails e.g. with Rapture3D, where EFX is supported
                if(m_context)
                {
                    break;
                }
            }
            alcCloseDevice(dev);
            devlist += std::strlen(devlist) + 1;
        }
        else
        {
            m_device = dev;
            m_context = alcCreateContext(m_device, paramList);
            break;
        }
    }

    if(!m_context)
    {
        BOOST_LOG_TRIVIAL(warning) << " Failed to create OpenAL context.";
        alcCloseDevice(m_device);
        m_device = nullptr;
        return;
    }

    alcMakeContextCurrent(m_context);

    loadALExtFunctions(m_device);

    std::string driver = "OpenAL library: ";
    driver += alcGetString(m_device, ALC_DEVICE_SPECIFIER);
    Console::instance().addLine(driver, gui::FontStyle::ConsoleInfo);

    alSpeedOfSound(330.0 * 512.0);
    alDopplerVelocity(330.0 * 510.0);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
#endif
}

void Engine::closeDevice()
{
    if(m_context)  // T4Larson <t4larson@gmail.com>: fixed
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }

    if(m_device)
    {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
}
} // namespace audio
