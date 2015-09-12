#include "audio.h"

#include <chrono>
#include <cstdio>

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "engine/system.h"
#include "fxmanager.h"
#include "gui/console.h"
#include "render/render.h"
#include "script/script.h"
#include "settings.h"
#include "strings.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world/camera.h"
#include "world/character.h"
#include "world/entity.h"
#include "world/room.h"

using gui::Console;

namespace audio
{

struct MemBufferFileIo : public SF_VIRTUAL_IO
{
    MemBufferFileIo(const uint8_t* data, sf_count_t dataSize)
        : SF_VIRTUAL_IO()
        , m_data(data)
        , m_dataSize(dataSize)
    {
        assert(data != nullptr);

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
                assert(offset >= 0 && offset <= self->m_dataSize);
                self->m_where = offset;
                break;
            case SEEK_CUR:
                assert(self->m_where + offset <= self->m_dataSize && self->m_where + offset >= 0);
                self->m_where += offset;
                break;
            case SEEK_END:
                assert(offset >= 0 && offset <= self->m_dataSize);
                self->m_where = self->m_dataSize - offset;
                break;
            default:
                assert(false);
        }
        return self->m_where;
    }

    static sf_count_t doRead(void *ptr, sf_count_t count, void *user_data)
    {
        auto self = static_cast<MemBufferFileIo*>(user_data);
        if(self->m_where + count > self->m_dataSize)
            count = self->m_dataSize - self->m_where;

        assert(self->m_where + count <= self->m_dataSize);

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

// ======== Audio source global methods ========

void loadOverridedSamples(const world::World *world)
{
    int  num_samples, num_sounds;
    int  sample_index, sample_count;
    char sample_name_mask[256];
    char sample_name[256];

    if(engine_lua.getOverridedSamplesInfo(&num_samples, &num_sounds, sample_name_mask))
    {
        size_t buffer_counter = 0;

        for(size_t i = 0; i < world->audioEngine.getBufferCount(); i++)
        {
            if(!world->audioEngine.isBufferMapped(i))
                continue;

            if(engine_lua.getOverridedSample(i, &sample_index, &sample_count))
            {
                for(int j = 0; j < sample_count; j++, buffer_counter++)
                {
                    sprintf(sample_name, sample_name_mask, (sample_index + j));
                    if(engine::fileExists(sample_name))
                    {
                        loadALbufferFromFile(world->audioEngine.getBuffer(buffer_counter), sample_name);
                    }
                }
            }
            else
            {
                buffer_counter += world->audioEngine.getMappedSampleCount(i);
            }
        }
    }
}

void initGlobals()
{
    audio_settings.music_volume = 0.7f;
    audio_settings.sound_volume = 0.8f;
    audio_settings.use_effects  = true;
    audio_settings.listener_is_player = false;
    audio_settings.stream_buffer_size = 32;
}

void init(uint32_t num_Sources)
{
    // FX should be inited first, as source constructor checks for FX slot to be created.

    if(audio_settings.use_effects)
        FxManager::instance()->init();

    // Generate new source array.

    num_Sources -= StreamSourceCount;          // Subtract sources reserved for music.
    engine::engine_world.audioEngine.setSourceCount(num_Sources);

    // Generate stream tracks array.

    engine::engine_world.audioEngine.setStreamTrackCount(StreamSourceCount);

    // Reset last room type used for assigning reverb.

    FxManager::instance()->last_room_type = TR_AUDIO_FX_LASTINDEX;
}

void deInit()
{
    engine::engine_world.audioEngine.deInitAudio();
    FxManager::instance().reset();
}

bool logALError(int error_marker)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        engine::Sys_DebugLog(LOG_FILENAME, "OpenAL error: %s / %d", alGetString(err), error_marker);
        return true;
    }
    return false;
}

void logSndfileError(int code)
{
    engine::Sys_DebugLog(LOG_FILENAME, sf_error_number(code));
}

float getByteDepth(SF_INFO sfInfo)
{
    switch(sfInfo.format & SF_FORMAT_SUBMASK)
    {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
            return 1;
        case SF_FORMAT_PCM_16:
            return 2;
        case SF_FORMAT_PCM_24:
            return 3;
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_FLOAT:
            return 4;
        case SF_FORMAT_DOUBLE:
            return 8;
        case SF_FORMAT_MS_ADPCM:
            return 0.5;
        default:
            return 1;
    }
}

int loadALbufferFromMem(ALuint buf_number, uint8_t *sample_pointer, size_t sample_size, size_t uncomp_sample_size)
{
    MemBufferFileIo wavMem(sample_pointer, sample_size);
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    SNDFILE* sample = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

    if(!sample)
    {
        engine::Sys_DebugLog(LOG_FILENAME, "Error: can't load sample #%03d from sample block!", buf_number);
        return -1;
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

    if((uncomp_sample_size == 0) || (real_size < uncomp_sample_size))
    {
        uncomp_sample_size = real_size;
    }

    // We need to change buffer size, as we're using floats here.

    const auto frameCount = uncomp_sample_size / sizeof(uint16_t);

    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats in same level.

    bool result = fillALBuffer(buf_number, sample, static_cast<Uint32>(frameCount), &sfInfo);

    sf_close(sample);

    return (result) ? (0) : (-3);   // Zero means success.
}

int loadALbufferFromFile(ALuint buf_number, const char *fname)
{
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(fname, SFM_READ, &sfInfo);

    if(!file)
    {
        Console::instance().warning(SYSWARN_CANT_OPEN_FILE);
        return -1;
    }

    bool result = fillALBuffer(buf_number, file, static_cast<Uint32>(sfInfo.frames), &sfInfo);

    sf_close(file);

    return (result) ? (0) : (-3);   // Zero means success.
}

bool fillALBuffer(ALuint buf_number, SNDFILE *wavFile, Uint32 frameCount, SF_INFO *sfInfo)
{
    if(sfInfo->channels > 1)   // We can't use non-mono samples.
    {
        engine::Sys_DebugLog(LOG_FILENAME, "Error: sample %03d is not mono!", buf_number);
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
    logALError(0);
    return true;
}

/**
 * Updates listener parameters by camera structure. For correct speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void updateListenerByCamera(world::Camera *cam)
{
    ALfloat v[6] = {
        cam->getViewDir()[0], cam->getViewDir()[1], cam->getViewDir()[2],
        cam->getUpDir()[0], cam->getUpDir()[1], cam->getUpDir()[2]
    };

    alListenerfv(AL_ORIENTATION, v);

    alListenerfv(AL_POSITION, cam->getPosition());

    btVector3 v2 = (cam->getPosition() - cam->m_prevPos) / engine::engine_frame_time;
    alListenerfv(AL_VELOCITY, v2);
    cam->m_prevPos = cam->getPosition();

    if(cam->m_currentRoom)
    {
        if(cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER)
        {
            FxManager::instance()->current_room_type = TR_AUDIO_FX_WATER;
        }
        else
        {
            FxManager::instance()->current_room_type = cam->m_currentRoom->reverb_info;
        }

        if(FxManager::instance()->water_state != static_cast<bool>(cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER))
        {
            FxManager::instance()->water_state = (cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER) != 0;

            if(FxManager::instance()->water_state)
            {
                engine::engine_world.audioEngine.send(TR_AUDIO_SOUND_UNDERWATER);
            }
            else
            {
                engine::engine_world.audioEngine.kill(TR_AUDIO_SOUND_UNDERWATER);
            }
        }
    }
}

void updateListenerByEntity(std::shared_ptr<world::Entity> /*ent*/)
{
    ///@FIXME: Add entity listener updater here.
}

} // namespace audio
