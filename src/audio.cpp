#include "audio.h"

#include <cmath>
#include <cstdio>

#include "console.h"
#include "camera.h"
#include "engine.h"
#include "vmath.h"
#include "entity.h"
#include "character_controller.h"
#include "system.h"
#include "render.h"
#include "strings.h"
#include "helpers.h"

#include <SDL2/SDL.h>

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

void AudioManager::loadALExtFunctions(ALCdevice* device)
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
    alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
    alGetFilteriv = (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
    alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
    alGetFilterfv = (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
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
void AudioManager::loadALExtFunctions(ALCdevice* device)
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
        switch(whence) {
        case SEEK_SET:
            assert(offset>=0 && offset<=self->m_dataSize);
            self->m_where = offset;
            break;
        case SEEK_CUR:
            assert(self->m_where+offset <= self->m_dataSize && self->m_where+offset >= 0);
            self->m_where += offset;
            break;
        case SEEK_END:
            assert(offset >=0 && offset <=self->m_dataSize);
            self->m_where = self->m_dataSize-offset;
            break;
        default:
            assert(false);
        }
        return self->m_where;
    }

    static sf_count_t doRead(void *ptr, sf_count_t count, void *user_data)
    {
        auto self = static_cast<MemBufferFileIo*>(user_data);
        if(self->m_where+count > self->m_dataSize)
            count = self->m_dataSize - self->m_where;

        assert(self->m_where+count <= self->m_dataSize);

        uint8_t* buf = static_cast<uint8_t*>(ptr);
        std::copy(self->m_data+self->m_where, self->m_data+self->m_where+count, buf);
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

btVector3 listener_position;

bool                        StreamTrack::damp_active = false;

// ======== AUDIOSOURCE CLASS IMPLEMENTATION ========

AudioSource::AudioSource(AudioManager* manager)
    : m_manager(manager)
{
    assert(manager != nullptr);

    alGenSources(1, &source_index);

    if(!alIsSource(source_index))
        return;

    alSourcef(source_index, AL_MIN_GAIN, 0.0);
    alSourcef(source_index, AL_MAX_GAIN, 1.0);

    if(manager->audio_settings.use_effects)
    {
        alSourcef(source_index, AL_ROOM_ROLLOFF_FACTOR, 1.0);
        alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,   AL_TRUE);
        alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
        alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 0.1);
    }
    else
    {
        alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 0.0);
    }
}


AudioSource::~AudioSource()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
        alDeleteSources(1, &source_index);
    }
}


bool AudioSource::IsActive() const
{
    return active;
}

bool AudioSource::IsLooping() const
{
    if(alIsSource(source_index))
    {
        ALint looping;
        alGetSourcei(source_index, AL_LOOPING, &looping);
        return (looping != AL_FALSE);
    }
    else
    {
        return false;
    }
}

bool AudioSource::IsPlaying() const
{
    if(alIsSource(source_index))
    {
        ALenum state = AL_STOPPED;
        alGetSourcei(source_index, AL_SOURCE_STATE, &state);

        // Paused state and existing file pointers also counts as playing.
        return ((state == AL_PLAYING) || (state == AL_PAUSED));
    }
    else
    {
        return false;
    }
}


void AudioSource::Play()
{
    if(alIsSource(source_index))
    {
        if(emitter_type == TR_AUDIO_EMITTER_GLOBAL)
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(source_index, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(source_index, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            if(m_manager->audio_settings.use_effects)
            {
                UnsetFX();
            }
        }
        else
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_FALSE);
            LinkEmitter();

            if(m_manager->audio_settings.use_effects)
            {
                SetFX();
                SetUnderwater();
            }
        }

        alSourcePlay(source_index);
        active = true;
    }
}


void AudioSource::Pause()
{
    if(alIsSource(source_index))
    {
        alSourcePause(source_index);
    }
}


void AudioSource::Stop()
{
    if(alIsSource(source_index))
    {
        alSourceStop(source_index);
    }
}


void AudioSource::Update()
{
    // Bypass any non-active source.
    if(!active) return;

    // Disable and bypass source, if it is stopped.
    if(!IsPlaying())
    {
        active = false;
        return;
    }

    // Bypass source, if it is global.
    if(emitter_type == TR_AUDIO_EMITTER_GLOBAL) return;

    ALfloat range, gain;

    alGetSourcef(source_index, AL_GAIN, &gain);
    alGetSourcef(source_index, AL_MAX_DISTANCE, &range);

    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.

    if(m_manager->isInRange(emitter_type, emitter_ID, range, gain))
    {
        LinkEmitter();

        if( (m_manager->audio_settings.use_effects) && (is_water != m_manager->fxManager.water_state) )
        {
            SetUnderwater();
        }
    }
    else
    {
        // Immediately stop source only if track is looped. It allows sounds which
        // were activated for already destroyed entities to finish (e.g. grenade
        // explosions, ricochets, and so on).

        if(IsLooping()) Stop();
    }
}


void AudioSource::SetBuffer(ALint buffer)
{
    AudioBuffer buffer_index = m_manager->audio_buffers[buffer];

    if(alIsSource(source_index) && alIsBuffer(buffer_index.handle))
    {
        alSourcei(source_index, AL_BUFFER, buffer_index.handle);

        // For some reason, OpenAL sometimes produces "Invalid Operation" error here,
        // so there's extra debug info - maybe it'll help some day.

        /*
        if(Audio_LogALError(1))
        {
            int channels, bits, freq;

            alGetBufferi(buffer_index, AL_CHANNELS,  &channels);
            alGetBufferi(buffer_index, AL_BITS,      &bits);
            alGetBufferi(buffer_index, AL_FREQUENCY, &freq);

            Sys_DebugLog(LOG_FILENAME, "Erroneous buffer %d info: CH%d, B%d, F%d", buffer_index, channels, bits, freq);
        }
        */
    }
}


void AudioSource::SetLooping(ALboolean is_looping)
{
    alSourcei(source_index, AL_LOOPING, is_looping);
}


void AudioSource::SetGain(ALfloat gain_value)
{
    alSourcef(source_index, AL_GAIN, Clamp(gain_value, 0.0, 1.0) * m_manager->audio_settings.sound_volume);
}


void AudioSource::SetPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    alSourcef(source_index, AL_PITCH, Clamp(pitch_value, 0.1, 2.0));
}


void AudioSource::SetRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(source_index, AL_REFERENCE_DISTANCE, range_value / 6.0);
    alSourcef(source_index, AL_MAX_DISTANCE, range_value);
}


void AudioSource::SetPosition(const ALfloat pos_vector[])
{
    alSourcefv(source_index, AL_POSITION, pos_vector);
}


void AudioSource::SetVelocity(const ALfloat vel_vector[])
{
    alSourcefv(source_index, AL_VELOCITY, vel_vector);
}


void AudioSource::SetFX()
{
    ALuint effect;
    ALuint slot;

    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    if(m_manager->fxManager.current_room_type != m_manager->fxManager.last_room_type)  // Switch audio send.
    {
        m_manager->fxManager.last_room_type = m_manager->fxManager.current_room_type;
        m_manager->fxManager.current_slot   = (++m_manager->fxManager.current_slot > (TR_AUDIO_MAX_SLOTS-1))?(0):(m_manager->fxManager.current_slot);

        effect = m_manager->fxManager.al_effect[m_manager->fxManager.current_room_type];
        slot   = m_manager->fxManager.al_slot[m_manager->fxManager.current_slot];

        assert( alIsAuxiliaryEffectSlot != nullptr );
        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = m_manager->fxManager.al_slot[m_manager->fxManager.current_slot];
    }

    // Assign global reverb FX to channel.

    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}


void AudioSource::UnsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source_index, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
}

void AudioSource::SetUnderwater()
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(m_manager->fxManager.water_state)
    {
        alSourcei(source_index, AL_DIRECT_FILTER, m_manager->fxManager.al_filter);
        is_water = true;
    }
    else
    {
        alSourcei(source_index, AL_DIRECT_FILTER, AL_FILTER_NULL);
        is_water = false;
    }
}


void AudioSource::LinkEmitter()
{
    switch(emitter_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            if(std::shared_ptr<Entity> ent = engine_world.getEntityByID(emitter_ID)) {
                SetPosition(ent->m_transform.getOrigin());
                SetVelocity(ent->m_speed);
            }
            return;

        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            SetPosition(m_manager->audio_emitters[emitter_ID].position);
            return;
    }
}

// ======== STREAMTRACK CLASS IMPLEMENTATION ========

StreamTrack::StreamTrack(AudioManager *manager)
    : m_manager(manager)
{
    alGenBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);              // Generate all buffers at once.
    alGenSources(1, &source);

    if(!alIsSource(source))
        return;

    alSource3f(source, AL_POSITION,        0.0f,  0.0f, -1.0f); // OpenAL tut says this.
    alSource3f(source, AL_VELOCITY,        0.0f,  0.0f,  0.0f);
    alSource3f(source, AL_DIRECTION,       0.0f,  0.0f,  0.0f);
    alSourcef (source, AL_ROLLOFF_FACTOR,  0.0f              );
    alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE           );
    alSourcei (source, AL_LOOPING,         AL_FALSE          ); // No effect, but just in case...

    // Setting method to -1 at init is required to prevent accidental
    // ov_clear call, which results in crash, if no vorbis file was
    // associated with given vorbis file structure.
}


StreamTrack::~StreamTrack()
{

    Stop(); // In case we haven't stopped yet.

    alDeleteSources(1, &source);
    alDeleteBuffers(TR_AUDIO_STREAM_NUMBUFFERS, buffers);
}

bool StreamTrack::Load(const char *path, const int index, const int type, const int load_method)
{
    if( (path        == NULL)                             ||
        (load_method >= TR_AUDIO_STREAM_METHOD_LASTINDEX) ||
        (type        >= TR_AUDIO_STREAM_TYPE_LASTINDEX)    )
    {
        return false;   // Do not load, if path, type or method are incorrect.
    }

    current_track = index;
    stream_type   = type;
    method        = load_method;
    dampable      = (stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND);   // Damp only looped (BGM) tracks.

    // Select corresponding stream loading method.

    if(method == TR_AUDIO_STREAM_METHOD_TRACK)
    {
        return (Load_Track(path));
    }
    else
    {
        return (Load_Wad((uint8_t)index, path));
    }
}

bool StreamTrack::Unload()
{
    bool result = false;

    if(alIsSource(source))  // Stop and unlink all associated buffers.
    {
        int queued;
        alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

        while(queued--)
        {
            ALuint buffer;
            alSourceUnqueueBuffers(source, 1, &buffer);
        }
    }

    if(snd_file)
    {
        sf_close(snd_file);
        snd_file = nullptr;
        result = true;
    }

    if(wad_file)
    {
        fclose(wad_file);
        wad_file = nullptr;
        result = true;
    }

    return result;
}

bool StreamTrack::Load_Track(const char *path)
{
    if(!(snd_file = sf_open(path, SFM_READ, &sf_info)))
    {
        Sys_DebugLog(LOG_FILENAME, "Load_Track: Couldn't open file: %s.", path);
        method = -1;    // T4Larson <t4larson@gmail.com>: stream is uninitialised, avoid clear.
        return false;
    }

    ConsoleInfo::instance().notify(SYSNOTE_TRACK_OPENED, path,
               sf_info.channels, sf_info.samplerate);

#ifdef AUDIO_OPENAL_FLOAT
    if(sf_info.channels == 1)
        format = AL_FORMAT_MONO_FLOAT32;
    else
        format = AL_FORMAT_STEREO_FLOAT32;
#else
    if(sf_info.channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;
#endif

    rate = sf_info.samplerate;

    return true;    // Success!
}

bool StreamTrack::Load_Wad(uint8_t index, const char* filename)
{
    if(index >= TR_AUDIO_STREAM_WAD_COUNT)
    {
        ConsoleInfo::instance().warning(SYSNOTE_WAD_OUT_OF_BOUNDS, TR_AUDIO_STREAM_WAD_COUNT);
        return false;
    }
    else
    {
        wad_file = fopen(filename, "rb");
        setbuf(wad_file, NULL);

        if(!wad_file)
        {
            ConsoleInfo::instance().warning(SYSWARN_FILE_NOT_FOUND, filename);
            return false;
        }
        else
        {
            char track_name[TR_AUDIO_STREAM_WAD_NAMELENGTH];
            uint32_t offset = 0;
            uint32_t length = 0;

            fseek(wad_file, (index * TR_AUDIO_STREAM_WAD_STRIDE), 0);
            fread((void*)track_name, TR_AUDIO_STREAM_WAD_NAMELENGTH, 1, wad_file);
            fread((void*)&length, sizeof(uint32_t), 1, wad_file);
            fread((void*)&offset, sizeof(uint32_t), 1, wad_file);

            fseek(wad_file, offset, 0);

            if(!(snd_file = sf_open_fd(fileno(wad_file), SFM_READ, &sf_info, false)))
            {
                ConsoleInfo::instance().warning(SYSNOTE_WAD_SEEK_FAILED, offset);
                method = -1;
                return false;
            }
            else
            {
                ConsoleInfo::instance().notify(SYSNOTE_WAD_PLAYING, filename, offset, length);
                ConsoleInfo::instance().notify(SYSNOTE_TRACK_OPENED, track_name,
                            sf_info.channels, sf_info.samplerate);
            }

#ifdef AUDIO_OPENAL_FLOAT
            if(sf_info.channels == 1)
                format = AL_FORMAT_MONO_FLOAT32;
            else
                format = AL_FORMAT_STEREO_FLOAT32;
#else
            if(sf_info.channels == 1)
                format = AL_FORMAT_MONO16;
            else
                format = AL_FORMAT_STEREO16;
#endif

            rate = sf_info.samplerate;

            return true;    // Success!
        }
    }
}

bool StreamTrack::Play(bool fade_in)
{
    int buffers_to_play = 0;

    // At start-up, we fill all available buffers.
    // TR soundtracks contain a lot of short tracks, like Lara speech etc., and
    // there is high chance that such short tracks won't fill all defined buffers.
    // For this reason, we count amount of filled buffers, and immediately stop
    // allocating them as long as Stream() routine returns false. Later, we use
    // this number for queuing buffers to source.

    for(int i = 0; i < TR_AUDIO_STREAM_NUMBUFFERS; i++, buffers_to_play++)
    {
        if(!Stream(buffers[i]))
        {
            if(!i)
            {
                Sys_DebugLog(LOG_FILENAME, "StreamTrack: error preparing buffers.");
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
        current_volume = 0.0;
    }
    else
    {
        current_volume = 1.0;
    }

    if(m_manager->audio_settings.use_effects)
    {
        if(stream_type == TR_AUDIO_STREAM_TYPE_CHAT)
        {
            SetFX();
        }
        else
        {
            UnsetFX();
        }
    }

    alSourcef(source, AL_GAIN, current_volume * m_manager->audio_settings.music_volume);
    alSourceQueueBuffers(source, buffers_to_play, buffers);
    alSourcePlay(source);

    ending = false;
    active = true;
    return   true;
}

void StreamTrack::Pause()
{
    if(alIsSource(source))
        alSourcePause(source);
}

void StreamTrack::End()     // Smoothly end track with fadeout.
{
    ending = true;
}

void StreamTrack::Stop()    // Immediately stop track.
{
    if(alIsSource(source))  // Stop and unlink all associated buffers.
    {
        if(IsPlaying()) alSourceStop(source);
    }
}

bool StreamTrack::Update()
{
    int  processed     = 0;
    bool buffered      = true;
    bool change_gain   = false;

    if(!active) return true; // Nothing to do here.

    if(!IsPlaying())
    {
       Unload();
       active = false;
       return true;
    }

    // Update damping, if track supports it.

    if(dampable)
    {
        // We check if damp condition is active, and if so, is it already at low-level or not.

        if(damp_active && (damped_volume < TR_AUDIO_STREAM_DAMP_LEVEL))
        {
            damped_volume += TR_AUDIO_STREAM_DAMP_SPEED;

            // Clamp volume.
            damped_volume = Clamp(damped_volume, 0.0, TR_AUDIO_STREAM_DAMP_LEVEL);
            change_gain   = true;
        }
        else if(!damp_active && (damped_volume > 0))    // If damp is not active, but it's still at low, restore it.
        {
            damped_volume -= TR_AUDIO_STREAM_DAMP_SPEED;

            // Clamp volume.
            damped_volume = Clamp(damped_volume, 0.0, TR_AUDIO_STREAM_DAMP_LEVEL);
            change_gain   = true;
        }
    }

    if(ending)     // If track is ending, crossfade it.
    {
        switch(stream_type)
        {
            case TR_AUDIO_STREAM_TYPE_BACKGROUND:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_BACKGROUND;
                break;

            case TR_AUDIO_STREAM_TYPE_ONESHOT:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_ONESHOT;
                break;

            case TR_AUDIO_STREAM_TYPE_CHAT:
                current_volume -= TR_AUDIO_STREAM_CROSSFADE_CHAT;
                break;
        }

        // Crossfade has ended, we can now kill the stream.
        if(current_volume <= 0.0)
        {
            Stop();
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
        if(current_volume < 1.0)
        {
            switch(stream_type)
            {
                case TR_AUDIO_STREAM_TYPE_BACKGROUND:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_BACKGROUND;
                    break;

                case TR_AUDIO_STREAM_TYPE_ONESHOT:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_ONESHOT;
                    break;

                case TR_AUDIO_STREAM_TYPE_CHAT:
                    current_volume += TR_AUDIO_STREAM_CROSSFADE_CHAT;
                    break;
            }

            // Clamp volume.
            current_volume = Clamp(current_volume, 0.0, 1.0);
            change_gain    = true;
        }
    }

    if(change_gain) // If any condition which modify track gain was met, call AL gain change.
    {
        alSourcef(source, AL_GAIN, current_volume              *  // Global track volume.
                                   (1.0 - damped_volume)       *  // Damp volume.
                                   m_manager->audio_settings.music_volume);  // Global music volume setting.
    }

    // Check if any track buffers were already processed.

    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)  // Manage processed buffers.
    {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);     // Unlink processed buffer.
        buffered = Stream(buffer);                      // Refill processed buffer.
        if(buffered)
            alSourceQueueBuffers(source, 1, &buffer);   // Relink processed buffer.
    }

    return buffered;
}

bool StreamTrack::IsTrack(const int track_index)    // Check if track has specific index.
{
    return (current_track == track_index);
}

bool StreamTrack::IsType(const int track_type)      // Check if track has specific type.
{
    return (track_type == stream_type);
}

bool StreamTrack::IsActive()                         // Check if track is still active.
{
    return active;
}

bool StreamTrack::IsDampable()                      // Check if track is dampable.
{
    return dampable;
}

bool StreamTrack::IsPlaying()                       // Check if track is playing.
{
    if(alIsSource(source))
    {
        ALenum state = AL_STOPPED;
        alGetSourcei(source, AL_SOURCE_STATE, &state);

        // Paused state and existing file pointers also counts as playing.
        return ((state == AL_PLAYING) || (state == AL_PAUSED));
    }
    else
    {
        return false;
    }
}

bool StreamTrack::Stream(ALuint buffer)
{
    assert(m_manager->audio_settings.stream_buffer_size >= sf_info.channels - 1);
#ifdef AUDIO_OPENAL_FLOAT
    std::vector<ALfloat> pcm(m_manager->audio_settings.stream_buffer_size);
#else
    std::vector<ALshort> pcm(m_manager->audio_settings.stream_buffer_size);
#endif
    size_t size = 0;

    // SBS - C + 1 is important to avoid endless loops if the buffer size isn't a multiple of the channels
    while(size < pcm.size() - sf_info.channels + 1)
    {
        // we need to read a multiple of sf_info.channels here
#ifdef AUDIO_OPENAL_FLOAT
        const size_t samplesToRead = (m_manager->audio_settings.stream_buffer_size - size) / sizeof(ALfloat);
        const sf_count_t samplesRead = sf_read_float(snd_file, pcm.data() + size, samplesToRead);
#else
        const size_t samplesToRead = (m_manager->audio_settings.stream_buffer_size - size) / sizeof(ALshort);
        const sf_count_t samplesRead = sf_read_short(snd_file, pcm.data() + size, samplesToRead);
#endif

        if(samplesRead > 0)
        {
#ifdef AUDIO_OPENAL_FLOAT
            size += samplesRead * sizeof(ALfloat);
#else
            size += samplesRead * sizeof(ALshort);
#endif
        }
        else
        {
            int error = sf_error(snd_file);
            if(error != SF_ERR_NO_ERROR)
            {
                AudioManager::logSndfileError( error );
            }
            else
            {
                if(stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND)
                {
                   sf_seek(snd_file, 0, SEEK_SET);
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

    alBufferData(buffer, format, pcm.data(), size, rate);
    return true;
}

void StreamTrack::SetFX()
{
    ALuint effect;
    ALuint slot;

    // Reverb FX is applied globally through audio send. Since player can
    // jump between adjacent rooms with different reverb info, we assign
    // several (2 by default) interchangeable audio sends, which are switched
    // every time current room reverb is changed.

    if(m_manager->fxManager.current_room_type != m_manager->fxManager.last_room_type)  // Switch audio send.
    {
        m_manager->fxManager.last_room_type = m_manager->fxManager.current_room_type;
        m_manager->fxManager.current_slot   = (++m_manager->fxManager.current_slot > (TR_AUDIO_MAX_SLOTS-1))?(0):(m_manager->fxManager.current_slot);

        effect = m_manager->fxManager.al_effect[m_manager->fxManager.current_room_type];
        slot   = m_manager->fxManager.al_slot[m_manager->fxManager.current_slot];

        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = m_manager->fxManager.al_slot[m_manager->fxManager.current_slot];
    }

    // Assign global reverb FX to channel.

    alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}


void StreamTrack::UnsetFX()
{
    // Remove any audio sends and direct filters from channel.

    alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
}

// ======== END STREAMTRACK CLASS IMPLEMENTATION ========


// General soundtrack playing routine. All native TR CD triggers and commands should ONLY
// call this one.

int AudioManager::streamPlay(const uint32_t track_index, const uint8_t mask)
{
    int    target_stream = -1;
    bool   do_fade_in    =  false;
    int    load_method   =  0;
    int    stream_type   =  0;

    char   file_path[256];          // Should be enough, and this is not the full path...

    // Don't even try to do anything with track, if its index is greater than overall amount of
    // soundtracks specified in a stream track map count (which is derived from script).

    if(track_index >= stream_track_map.size())
    {
        ConsoleInfo::instance().addLine("StreamPlay: CANCEL, track index is out of bounds.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_WRONGTRACK;
    }

    // Don't play track, if it is already playing.
    // This should become useless option, once proper one-shot trigger functionality is implemented.

    if(isTrackPlaying(track_index))
    {
        ConsoleInfo::instance().addLine("StreamPlay: CANCEL, stream already playing.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_IGNORED;
    }

    // lua_GetSoundtrack returns stream type, file path and load method in last three
    // provided arguments. That is, after calling this function we receive stream type
    // in "stream_type" argument, file path into "file_path" argument and load method into
    // "load_method" argument. Function itself returns false, if script wasn't found or
    // request was broken; in this case, we quit.

    if(!lua_GetSoundtrack(engine_lua, track_index, file_path, &load_method, &stream_type))
    {
        ConsoleInfo::instance().addLine("StreamPlay: CANCEL, wrong track index or broken script.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_WRONGTRACK;
    }

    // Don't try to play track, if it was already played by specified bit mask.
    // Additionally, TrackAlreadyPlayed function applies specified bit mask to track map.
    // Also, bit mask is valid only for non-looped tracks, since looped tracks are played
    // in any way.

    if((stream_type != TR_AUDIO_STREAM_TYPE_BACKGROUND) &&
        trackAlreadyPlayed(track_index, mask))
    {
        return TR_AUDIO_STREAMPLAY_IGNORED;
    }

    // Entry found, now process to actual track loading.

    target_stream = getFreeStream();            // At first, we need to get free stream.

    if(target_stream == -1)
    {
        do_fade_in = stopStreams(stream_type);  // If no free track found, hardly stop all tracks.
        target_stream = getFreeStream();        // Try again to assign free stream.

        if(target_stream == -1)
        {
            ConsoleInfo::instance().addLine("StreamPlay: CANCEL, no free stream.", FONTSTYLE_CONSOLE_WARNING);
            return TR_AUDIO_STREAMPLAY_NOFREESTREAM;  // No success, exit and don't play anything.
        }
    }
    else
    {
        do_fade_in = endStreams(stream_type);   // End all streams of this type with fadeout.

        // Additionally check if track type is looped. If it is, force fade in in any case.
        // This is needed to smooth out possible pop with gapless looped track at a start-up.

        do_fade_in = (stream_type == TR_AUDIO_STREAM_TYPE_BACKGROUND)?(true):(false);
    }

    // Finally - load our track.

    if(!stream_tracks[target_stream].Load(file_path, track_index, stream_type, load_method))
    {
        ConsoleInfo::instance().addLine("StreamPlay: CANCEL, stream load error.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_LOADERROR;
    }

    // Try to play newly assigned and loaded track.

    if(!stream_tracks[target_stream].Play(do_fade_in))
    {
        ConsoleInfo::instance().addLine("StreamPlay: CANCEL, stream play error.", FONTSTYLE_CONSOLE_WARNING);
        return TR_AUDIO_STREAMPLAY_PLAYERROR;
    }

    return TR_AUDIO_STREAMPLAY_PROCESSED;   // Everything is OK!
}


// General damping update procedure. Constantly checks if damp condition exists, and
// if so, it lowers the volume of tracks which are dampable.

void AudioManager::updateStreamsDamping()
{
    StreamTrack::damp_active = false;   // Reset damp activity flag.

    // Scan for any tracks that can provoke damp. Usually it's any tracks that are
    // NOT background. So we simply check this condition and set damp activity flag
    // if condition is met.

    for(uint32_t i = 0; i < stream_tracks.size(); i++)
    {
        if(stream_tracks[i].IsPlaying())
        {
            if(!stream_tracks[i].IsType(TR_AUDIO_STREAM_TYPE_BACKGROUND))
            {
                StreamTrack::damp_active = true;
                return; // No need to check more, we found at least one condition.
            }
        }
    }
}


// Update routine for all streams. Should be placed into main loop.

void AudioManager::updateStreams()
{
    updateStreamsDamping();

    for(StreamTrack& track : stream_tracks)
    {
        track.Update();
    }
}

bool AudioManager::isTrackPlaying(int32_t track_index)
{
    for(StreamTrack& track : stream_tracks)
    {
        if( (track_index == -1 || track.IsTrack(track_index)) &&
            track.IsPlaying() )
        {
            return true;
        }
    }

    return false;
}

bool AudioManager::trackAlreadyPlayed(uint32_t track_index, int8_t mask)
{
    if(!mask)
    {
        return false;   // No mask, play in any case.
    }

    if(track_index >= stream_track_map.size())
    {
        return true;    // No such track, hence "already" played.
    }
    else
    {
        mask &= 0x3F;   // Clamp mask just in case.

        if(stream_track_map[track_index] == mask)
        {
            return true;    // Immediately return true, if flags are directly equal.
        }
        else
        {
            int8_t played = stream_track_map[track_index] & mask;
            if(played == mask)
            {
                return true;    // Bits were set, hence already played.
            }
            else
            {
                stream_track_map[track_index] |= mask;
                return false;   // Not yet played, set bits and return false.
            }
        }
    }
}

int AudioManager::getFreeStream()
{
    for(size_t i = 0; i < stream_tracks.size(); i++)
    {
        if( (!stream_tracks[i].IsPlaying()) &&
            (!stream_tracks[i].IsActive())   )
        {
            return i;
        }
    }

    return TR_AUDIO_STREAMPLAY_NOFREESTREAM;  // If no free source, return error.
}

bool AudioManager::stopStreams(int stream_type)
{
    bool result = false;

    for(StreamTrack& track : stream_tracks)
    {
        if( track.IsPlaying() &&
           (track.IsType(stream_type) || stream_type == -1)) // Stop ALL streams at once.
        {
            result = true;
            track.Stop();
        }
    }

    return result;
}

bool AudioManager::endStreams(int stream_type)
{
    bool result = false;

    for(StreamTrack& track : stream_tracks)
    {
        if( (stream_type == -1) ||                              // End ALL streams at once.
            (track.IsPlaying() && track.IsType(stream_type)) )
        {
            result = true;
            track.End();
        }
    }

    return result;
}

// ======== Audio source global methods ========

bool AudioManager::isInRange(int entity_type, int entity_ID, float range, float gain)
{
    btVector3 vec{0,0,0};

    switch(entity_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            if(std::shared_ptr<Entity> ent = engine_world.getEntityByID(entity_ID))
            {
                vec = ent->m_transform.getOrigin();
            }
            else
            {
                return false;
            }
            break;

        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            if((uint32_t)entity_ID > audio_emitters.size())
            {
                return false;
            }
            vec = audio_emitters[entity_ID].position;
            break;

        case TR_AUDIO_EMITTER_GLOBAL:
            return true;

        default:
            return false;
    }

    auto dist = listener_position.distance2(vec);

    // We add 1/4 of overall distance to fix up some issues with
    // pseudo-looped sounds that are called at certain frames in animations.

    dist /= (gain + 1.25);

    return dist < range * range;
}


void AudioManager::updateSources()
{
    if(audio_sources.empty())
    {
        return;
    }

    alGetListenerfv(AL_POSITION, listener_position);

    for(size_t i = 0; i < audio_emitters.size(); i++)
    {
        send(audio_emitters[i].sound_index, TR_AUDIO_EMITTER_SOUNDSOURCE, i);
    }

    for(size_t i = 0; i < audio_sources.size(); i++)
    {
        audio_sources[i].Update();
    }
}


void AudioManager::pauseAllSources()
{
    for(AudioSource& source : audio_sources)
    {
        if(source.IsActive())
        {
            source.Pause();
        }
    }
}

void AudioManager::stopAllSources()
{
    for(AudioSource& source : audio_sources)
    {
        source.Stop();
    }
}

void AudioManager::resumeAllSources()
{
    for(AudioSource& source : audio_sources)
    {
        if(source.IsActive())
        {
            source.Play();
        }
    }
}


/// @fixme Add condition (compare max_dist with new source dist)
int AudioManager::getFreeSource() const
{
    for(size_t i = 0; i < audio_sources.size(); i++)
    {
        if(!audio_sources[i].IsActive())
        {
            return i;
        }
    }

    return -1;
}


int AudioManager::findPlayingSource(int effect_ID, int entity_type, int entity_ID)
{
    int i;
    for(const AudioSource& source : audio_sources)
    {
        if( ((entity_type == -1) || (source.emitter_type == (uint32_t)entity_type)) &&
            ((entity_ID   == -1) || (source.emitter_ID   == ( int32_t)entity_ID  )) &&
            ((effect_ID   == -1) || (source.effect_index == (uint32_t)effect_ID  ))  )
        {
            if(source.IsPlaying())
                return i;
        }
        ++i;
    }

    return -1;
}


int AudioManager::send(int effect_ID, int entity_type, int entity_ID)
{
    int32_t         source_number;
    uint16_t        random_value;
    ALfloat         random_float;
    AudioEffect*  effect = NULL;
    AudioSource    *source = NULL;

    // If there are no audio buffers or effect index is wrong, don't process.

    if(audio_buffers.empty() || effect_ID < 0) return TR_AUDIO_SEND_IGNORED;

    // Remap global engine effect ID to local effect ID.

    if((uint32_t)effect_ID >= audio_map.size())
    {
        return TR_AUDIO_SEND_NOSAMPLE;  // Sound is out of bounds; stop.
    }

    int16_t real_ID = audio_map[effect_ID];

    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.

    if(real_ID<0 || real_ID>static_cast<int>(audio_effects.size()))
    {
        return TR_AUDIO_SEND_NOSAMPLE;
    }
    else
    {
        effect = &audio_effects[real_ID];
    }

    // Pre-step 2: check if sound non-looped and chance to play isn't zero,
    // then randomly select if it should be played or not.

    if((effect->loop != TR_AUDIO_LOOP_LOOPED) && (effect->chance > 0))
    {
        random_value = rand() % 0x7FFF;
        if(effect->chance < random_value)
        {
            // Bypass audio send, if chance test is not passed.
            return TR_AUDIO_SEND_IGNORED;
        }
    }

    // Pre-step 3: Calculate if effect's hearing sphere intersect listener's hearing sphere.
    // If it's not, bypass audio send (cause we don't want it to occupy channel, if it's not
    // heard).

    if(isInRange(entity_type, entity_ID, effect->range, effect->gain ) == false)
    {
        return TR_AUDIO_SEND_IGNORED;
    }

    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and stop it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.

    source_number = findPlayingSource(effect_ID, entity_type, entity_ID);

    if(source_number != -1)
    {
        if(effect->loop == TR_AUDIO_LOOP_REWIND)
        {
            audio_sources[source_number].Stop();
        }
        else if(effect->loop) // Any other looping case (Wait / Loop).
        {
            return TR_AUDIO_SEND_IGNORED;
        }
    }
    else
    {
        source_number = getFreeSource();  // Get free source.
    }

    if(source_number != -1)  // Everything is OK, we're sending audio to channel.
    {
        int buffer_index;

        // Step 1. Assign buffer to source.

        if(effect->sample_count > 1)
        {
            // Select random buffer, if effect info contains more than 1 assigned samples.
            random_value = rand() % (effect->sample_count);
            buffer_index = random_value + effect->sample_index;
        }
        else
        {
            // Just assign buffer to source, if there is only one assigned sample.
            buffer_index = effect->sample_index;
        }

        source = &audio_sources[source_number];

        source->SetBuffer(buffer_index);

        // Step 2. Check looped flag, and if so, set source type to looped.

        if(effect->loop == TR_AUDIO_LOOP_LOOPED)
        {
            source->SetLooping(AL_TRUE);
        }
        else
        {
            source->SetLooping(AL_FALSE);
        }

        // Step 3. Apply internal sound parameters.

        source->emitter_ID   = entity_ID;
        source->emitter_type = entity_type;
        source->effect_index = effect_ID;

        // Step 4. Apply sound effect properties.

        if(effect->rand_pitch)  // Vary pitch, if flag is set.
        {
            random_float = rand() % effect->rand_pitch_var;
            random_float = effect->pitch + ((random_float - 25.0) / 200.0);
            source->SetPitch(random_float);
        }
        else
        {
            source->SetPitch(effect->pitch);
        }

        if(effect->rand_gain)   // Vary gain, if flag is set.
        {
            random_float = rand() % effect->rand_gain_var;
            random_float = effect->gain + (random_float - 25.0) / 200.0;
            source->SetGain(random_float);
        }
        else
        {
            source->SetGain(effect->gain);
        }

        source->SetRange(effect->range);    // Set audible range.

        source->Play();                     // Everything is OK, play sound now!

        return TR_AUDIO_SEND_PROCESSED;
    }
    else
    {
        return TR_AUDIO_SEND_NOCHANNEL;
    }
}


int AudioManager::kill(int effect_ID, int entity_type, int entity_ID)
{
    int playing_sound = findPlayingSource(effect_ID, entity_type, entity_ID);

    if(playing_sound != -1)
    {
        audio_sources[playing_sound].Stop();
        return TR_AUDIO_SEND_PROCESSED;
    }

    return TR_AUDIO_SEND_IGNORED;
}


void Audio_LoadOverridedSamples(struct World *world)
{
    int  num_samples, num_sounds;
    int  sample_index, sample_count;
    char sample_name_mask[256];
    char sample_name[256];

    if(lua_GetOverridedSamplesInfo(engine_lua, &num_samples, &num_sounds, sample_name_mask))
    {
        int buffer_counter = 0;

        for(uint32_t i = 0; i < world->audio_manager.audio_buffers.size(); i++)
        {
            if(world->audio_manager.audio_map[i] != -1)
            {
                if(lua_GetOverridedSample(engine_lua, i, &sample_index, &sample_count))
                {
                    for(int j = 0; j < sample_count; j++, buffer_counter++)
                    {
                        sprintf(sample_name, sample_name_mask, (sample_index + j));
                        if(Engine_FileFound(sample_name))
                        {
                            world->audio_manager.audio_buffers[buffer_counter].loadFromFile(sample_name);
                        }
                    }
                }
                else
                {
                    buffer_counter += world->audio_manager.audio_effects[world->audio_manager.audio_map[i]].sample_count;
                }
            }
        }
    }
}

void AudioManager::initFX()
{
    if( audio_settings.effects_initialized )
        return;

    memset(&fxManager, 0, sizeof(AudioFxManager));

    // Set up effect slots, effects and filters.

    alGenAuxiliaryEffectSlots(TR_AUDIO_MAX_SLOTS, fxManager.al_slot);
    alGenEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    alGenFilters(1, &fxManager.al_filter);

    alFilteri(fxManager.al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.

    // Fill up effects with reverb presets.

    EFXEAXREVERBPROPERTIES reverb1 = EFX_REVERB_PRESET_CITY;
    fxManager.loadReverbToFX(TR_AUDIO_FX_OUTSIDE, &reverb1);

    EFXEAXREVERBPROPERTIES reverb2 = EFX_REVERB_PRESET_LIVINGROOM;
    fxManager.loadReverbToFX(TR_AUDIO_FX_SMALLROOM, &reverb2);

    EFXEAXREVERBPROPERTIES reverb3 = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    fxManager.loadReverbToFX(TR_AUDIO_FX_MEDIUMROOM, &reverb3);

    EFXEAXREVERBPROPERTIES reverb4 = EFX_REVERB_PRESET_DOME_TOMB;
    fxManager.loadReverbToFX(TR_AUDIO_FX_LARGEROOM, &reverb4);

    EFXEAXREVERBPROPERTIES reverb5 = EFX_REVERB_PRESET_PIPE_LARGE;
    fxManager.loadReverbToFX(TR_AUDIO_FX_PIPE, &reverb5);

    EFXEAXREVERBPROPERTIES reverb6 = EFX_REVERB_PRESET_UNDERWATER;
    fxManager.loadReverbToFX(TR_AUDIO_FX_WATER, &reverb6);

    audio_settings.effects_initialized = true;
}

bool AudioFxManager::loadReverbToFX(int effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    ALuint effect = al_effect[effect_index];

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
        Sys_DebugLog(LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return false;
    }

    return true;
}

void AudioManager::init(size_t num_Sources)
{
    // FX should be inited first, as source constructor checks for FX slot to be created.

    if(audio_settings.use_effects)
        initFX();

    // Generate new source array.
    num_Sources -= TR_AUDIO_STREAM_NUMSOURCES;          // Subtract sources reserved for music.

    // loops are necessary because we don't have a default constructor

    assert( audio_sources.empty() );
    for(size_t i=0; i<num_Sources; ++i)
        audio_sources.emplace_back( this );

    // Generate stream tracks array.

    assert( stream_tracks.empty() );
    for(size_t i=0; i<TR_AUDIO_STREAM_NUMSOURCES; ++i)
        stream_tracks.emplace_back( this );

    fxManager.last_room_type = TR_AUDIO_FX_LASTINDEX;
}

void AudioManager::deinit()
{
    stopAllSources();
    stopStreams();

    deInitDelay();

    audio_sources.clear();
    stream_tracks.clear();
    stream_track_map.clear();

    ///@CRITICAL: You must delete all sources before deleting buffers!

    audio_buffers.clear();

    audio_effects.clear();
    audio_map.clear();

    if(audio_settings.effects_initialized)
    {
        for(int i = 0; i < TR_AUDIO_MAX_SLOTS; i++)
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
}

bool AudioManager::deInitDelay()
{
    float begin_time   = Sys_FloatTime();
    float curr_time    = 0.0;

    while( isTrackPlaying() || findPlayingSource() >= 0 )
    {
        curr_time = Sys_FloatTime() - begin_time;

        if(curr_time > TR_AUDIO_DEINIT_DELAY)
        {
            Sys_DebugLog(LOG_FILENAME, "Audio deinit timeout reached! Something is wrong with audio driver.");
            break;
        }
    }

    return true;
}


bool AudioManager::logALError(int error_marker)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        Sys_DebugLog(LOG_FILENAME, "OpenAL error: %s / %d", alGetString(err), error_marker);
        return true;
    }
    return false;
}


void AudioManager::logSndfileError(int code)
{
    Sys_DebugLog(LOG_FILENAME, sf_error_number(code));
}

int AudioBuffer::loadFromMem(uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size)
{
    MemBufferFileIo wavMem(sample_pointer, sample_size);
    SF_INFO sfInfo;
    SNDFILE* sample = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

    if(!sample)
    {
        Sys_DebugLog(LOG_FILENAME, "Error: can't load sample #%03d from sample block!", handle);
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

    uint32_t real_size = sfInfo.frames * sizeof(uint16_t);

    if((uncomp_sample_size == 0) || (real_size < uncomp_sample_size))
    {
        uncomp_sample_size = real_size;
    }

    // We need to change buffer size, as we're using floats here.

#ifdef AUDIO_OPENAL_FLOAT
    uncomp_sample_size = (uncomp_sample_size / sizeof(uint16_t)) * sizeof(ALfloat);
#else
    uncomp_sample_size = (uncomp_sample_size / sizeof(uint16_t)) * sizeof(ALshort);
#endif

    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats in same level.

    bool result = fill(sample, uncomp_sample_size, &sfInfo);

    sf_close(sample);

    return (result)?(0):(-3);   // Zero means success.
}


int AudioBuffer::loadFromFile(const char *fname)
{
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(fname, SFM_READ, &sfInfo);

    if(!file)
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_OPEN_FILE);
        return -1;
    }

#ifdef AUDIO_OPENAL_FLOAT
    bool result = fill(file, sfInfo.frames * sizeof(ALfloat), &sfInfo);
#else
    bool result = fill(file, sfInfo.frames * sizeof(ALshort), &sfInfo);
#endif

    sf_close(file);

    return (result)?(0):(-3);   // Zero means success.
}

bool AudioBuffer::fill(SNDFILE *wavFile, Uint32 buffer_size, SF_INFO *sfInfo)
{
    if(sfInfo->channels > 1)   // We can't use non-mono samples.
    {
        Sys_DebugLog(LOG_FILENAME, "Error: sample %03d is not mono!", handle);
        return false;
    }

#ifdef AUDIO_OPENAL_FLOAT
    std::vector<ALfloat> frames( buffer_size / sizeof(ALfloat));
    /*const sf_count_t samplesRead =*/ sf_readf_float(wavFile, frames.data(), frames.size());

    alBufferData(handle, AL_FORMAT_MONO_FLOAT32, &frames.front(), buffer_size, sfInfo->samplerate);
#else
    std::vector<ALshort> frames( buffer_size / sizeof(ALshort));
    /*const sf_count_t samplesRead =*/ sf_readf_short(wavFile, frames.data(), frames.size());

    alBufferData(handle, AL_FORMAT_MONO16, &frames.front(), buffer_size, sfInfo->samplerate);
#endif
    AudioManager::logALError(0);
    return true;
}


/**
 * Updates listener parameters by camera structure. For correct speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void AudioManager::updateListenerByCamera(struct Camera *cam)
{
    ALfloat v[6] = {
        cam->m_viewDir[0], cam->m_viewDir[1], cam->m_viewDir[2],
        cam->m_upDir[0], cam->m_upDir[1], cam->m_upDir[2]
    };

    alListenerfv(AL_ORIENTATION, v);

    alListenerfv(AL_POSITION, cam->m_pos);

    btVector3 v2 = cam->m_pos - cam->m_prevPos;
    v2[3] = 1.0 / engine_frame_time;
    v2 *= v2[3];
    alListenerfv(AL_VELOCITY, v2);
    cam->m_prevPos = cam->m_pos;

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

        if(fxManager.water_state != (int8_t)(cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER))
        {
            fxManager.water_state = cam->m_currentRoom->flags & TR_ROOM_FLAG_WATER;

            if(fxManager.water_state)
            {
                send(TR_AUDIO_SOUND_UNDERWATER);
            }
            else
            {
                kill(TR_AUDIO_SOUND_UNDERWATER);
            }
        }
    }
}

void AudioManager::updateListenerByEntity(std::shared_ptr<Entity> /*ent*/)
{
    ///@FIXME: Add entity listener updater here.
}

void AudioManager::update()
{
    updateSources();
    updateStreams();

    if(audio_settings.listener_is_player)
    {
        updateListenerByEntity(engine_world.character);
    }
    else
    {
        updateListenerByCamera(renderer.camera());
    }
}

void AudioManager::initEmitters(const tr_sound_source_t* sources, size_t count)
{
    // Cycle through sound emitters and
    // parse them to native OpenTomb sound emitters structure.

    for(size_t i = 0; i < count; i++)
    {
        audio_emitters.emplace_back();
        AudioEmitter& emitter = audio_emitters.back();
        emitter.emitter_index = i;
        emitter.sound_index   =  sources[i].sound_id;
        emitter.position[0]   =  sources[i].x;
        emitter.position[1]   =  sources[i].z;
        emitter.position[2]   = -sources[i].y;
        emitter.flags         =  sources[i].flags;
    }
}
