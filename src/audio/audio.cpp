#include "audio.h"

#include <chrono>
#include <cstdio>

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "engine/system.h"
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
#ifndef AL_ALEXT_PROTOTYPES

extern "C"
{
    // Effect objects
    LPALGENEFFECTS alGenEffects = nullptr;
    LPALDELETEEFFECTS alDeleteEffects = nullptr;
    LPALISEFFECT alIsEffect = nullptr;
    LPALEFFECTI alEffecti = nullptr;
    LPALEFFECTIV alEffectiv = nullptr;
    LPALEFFECTF alEffectf = nullptr;
    LPALEFFECTFV alEffectfv = nullptr;
    LPALGETEFFECTI alGetEffecti = nullptr;
    LPALGETEFFECTIV alGetEffectiv = nullptr;
    LPALGETEFFECTF alGetEffectf = nullptr;
    LPALGETEFFECTFV alGetEffectfv = nullptr;

    //Filter objects
    LPALGENFILTERS alGenFilters = nullptr;
    LPALDELETEFILTERS alDeleteFilters = nullptr;
    LPALISFILTER alIsFilter = nullptr;
    LPALFILTERI alFilteri = nullptr;
    LPALFILTERIV alFilteriv = nullptr;
    LPALFILTERF alFilterf = nullptr;
    LPALFILTERFV alFilterfv = nullptr;
    LPALGETFILTERI alGetFilteri = nullptr;
    LPALGETFILTERIV alGetFilteriv = nullptr;
    LPALGETFILTERF alGetFilterf = nullptr;
    LPALGETFILTERFV alGetFilterfv = nullptr;

    // Auxiliary slot object
    LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
    LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
    LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
    LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
    LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
    LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
    LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
    LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
    LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
    LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
    LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;
}

void loadALExtFunctions(ALCdevice* device)
{
    static bool isLoaded = false;
    if(isLoaded)
        return;

    printf("\nOpenAL device extensions: %s\n", alcGetString(device, ALC_EXTENSIONS));
    assert(alcIsExtensionPresent(device, ALC_EXT_EFX_NAME) == ALC_TRUE);

    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects"); assert(alGenEffects);
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
    alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
    alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
    alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
    alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
    alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
    alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
    alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
    alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
    alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
    alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
    alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
    alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
    alGetFilteri = (LPALGETFILTERI)alGetProcAddress("alGetFilteri");
    alGetFilteriv = (LPALGETFILTERIV)alGetProcAddress("alGetFilteriv");
    alGetFilterf = (LPALGETFILTERF)alGetProcAddress("alGetFilterf");
    alGetFilterfv = (LPALGETFILTERFV)alGetProcAddress("alGetFilterfv");
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
    alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
    alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
    alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
    alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
    alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
    alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

    isLoaded = true;
}
#else
void Audio_LoadALExtFunctions(ALCdevice* device)
{
    // we have the functions already provided by native extensions
}
#endif

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

FxManager fxManager;

// ======== Audio source global methods ========

void loadOverridedSamples(world::World *world)
{
    int  num_samples, num_sounds;
    int  sample_index, sample_count;
    char sample_name_mask[256];
    char sample_name[256];

    if(engine_lua.getOverridedSamplesInfo(&num_samples, &num_sounds, sample_name_mask))
    {
        int buffer_counter = 0;

        for(uint32_t i = 0; i < world->audio_buffers.size(); i++)
        {
            if(world->audio_map[i] != -1)
            {
                if(engine_lua.getOverridedSample(i, &sample_index, &sample_count))
                {
                    for(int j = 0; j < sample_count; j++, buffer_counter++)
                    {
                        sprintf(sample_name, sample_name_mask, (sample_index + j));
                        if(engine::fileExists(sample_name))
                        {
                            loadALbufferFromFile(world->audio_buffers[buffer_counter], sample_name);
                        }
                    }
                }
                else
                {
                    buffer_counter += world->audio_effects[world->audio_map[i]].sample_count;
                }
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

void initFX()
{
    if(audio_settings.effects_initialized)
        return;

    memset(&fxManager, 0, sizeof(FxManager));

    // Set up effect slots, effects and filters.

    alGenAuxiliaryEffectSlots(MaxSlots, fxManager.al_slot);
    alGenEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    alGenFilters(1, &fxManager.al_filter);

    alFilteri(fxManager.al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    loadReverbToFX(TR_AUDIO_FX_OUTSIDE, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    loadReverbToFX(TR_AUDIO_FX_SMALLROOM, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    loadReverbToFX(TR_AUDIO_FX_MEDIUMROOM, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    loadReverbToFX(TR_AUDIO_FX_LARGEROOM, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    loadReverbToFX(TR_AUDIO_FX_PIPE, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    loadReverbToFX(TR_AUDIO_FX_WATER, &reverb6);

    audio_settings.effects_initialized = true;
}

int loadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    ALuint effect = fxManager.al_effect[effect_index];

    if(alIsEffect(effect))
    {
        alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

        alEffectf(effect, AL_REVERB_DENSITY, reverb->flDensity);
        alEffectf(effect, AL_REVERB_DIFFUSION, reverb->flDiffusion);
        alEffectf(effect, AL_REVERB_GAIN, reverb->flGain);
        alEffectf(effect, AL_REVERB_GAINHF, reverb->flGainHF);
        alEffectf(effect, AL_REVERB_DECAY_TIME, reverb->flDecayTime);
        alEffectf(effect, AL_REVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
        alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
        alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
        alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
        alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
        alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
        alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
        alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
    }
    else
    {
        engine::Sys_DebugLog(LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return 0;
    }

    return 1;
}

void init(uint32_t num_Sources)
{
    // FX should be inited first, as source constructor checks for FX slot to be created.

    if(audio_settings.use_effects)
        initFX();

    // Generate new source array.

    num_Sources -= StreamSourceCount;          // Subtract sources reserved for music.
    engine::engine_world.audio_sources.resize(num_Sources);

    // Generate stream tracks array.

    engine::engine_world.stream_tracks.resize(StreamSourceCount);

    // Reset last room type used for assigning reverb.

    fxManager.last_room_type = TR_AUDIO_FX_LASTINDEX;
}

int deInit()
{
    engine::engine_world.deInitAudio();

    if(audio_settings.effects_initialized)
    {
        for(int i = 0; i < MaxSlots; i++)
        {
            if(fxManager.al_slot[i])
            {
                alAuxiliaryEffectSloti(fxManager.al_slot[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
                alDeleteAuxiliaryEffectSlots(1, &fxManager.al_slot[i]);
            }
        }

        alDeleteFilters(1, &fxManager.al_filter);
        alDeleteEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
        audio_settings.effects_initialized = false;
    }

    return 1;
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
            fxManager.current_room_type = TR_AUDIO_FX_WATER;
        }
        else
        {
            fxManager.current_room_type = cam->m_currentRoom->reverb_info;
        }

        if(fxManager.water_state != static_cast<bool>(cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER))
        {
            fxManager.water_state = (cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER) != 0;

            if(fxManager.water_state)
            {
                engine::engine_world.send(TR_AUDIO_SOUND_UNDERWATER);
            }
            else
            {
                engine::engine_world.kill(TR_AUDIO_SOUND_UNDERWATER);
            }
        }
    }
}

void updateListenerByEntity(std::shared_ptr<world::Entity> /*ent*/)
{
    ///@FIXME: Add entity listener updater here.
}

} // namespace audio
