
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
extern "C" {
#include "al/AL/al.h"
#include "al/AL/alc.h"
}

#include "audio.h"
#include "console.h"
#include "camera.h"
#include "vmath.h"
#include "entity.h"
#include "character_controller.h"

struct audio_settings_s     audio_settings = {0};

AudioSource::AudioSource()
{
    active = false;
    emitter_ID =  -1;
    emitter_type = TR_SOUND_EMITTER_ENTITY;
    effect_index = 0;
    sample_index = 0;
    sample_count = 0;
    is_water     = false;
}

AudioSource::~AudioSource()
{
    ALint val;
    
    alGetSourcei(source_index, AL_SOURCE_STATE, &val);
    if(val == AL_PLAYING)
    {
        alSourceStop(source_index);
        alDeleteSources(1, &source_index);
    }
}

void AudioSource::SetEmitterIndex(const uint32_t index, const uint32_t type)
{
    emitter_ID   = index;
    emitter_type = type;
}

void AudioSource::SetGain(ALfloat gain_value)
{
    gain_value = (gain_value > 1.0)?(1.0):(gain_value);
    gain_value = (gain_value < 0.0)?(0.0):(gain_value);
    
    alSourcef(source_index, AL_GAIN, gain_value);
}

void AudioSource::SetPitch(ALfloat pitch_value)
{
    pitch_value = (pitch_value > 1.0)?(1.0):(pitch_value);
    pitch_value = (pitch_value < 0.0)?(0.0):(pitch_value);
    
    alSourcef(source_index, AL_PITCH, pitch_value);
}

void AudioSource::SetRange(ALfloat range_value)
{
    range_value = (range_value > 1.0)?(1.0):(range_value);
    range_value = (range_value < 0.0)?(0.0):(range_value);
    
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

void AudioSource::Erase()
{
    active = false;
    alSourceStop(source_index);
}

void Audio_UpdateSources(struct world_s *world)
{
#if 0
    ALint val;
    ALfloat vec[3];
    
    if(world->audio_sources_count < 1)
    {
        return;
    }
        
    for(int i = 0; i < world->audio_sources_count; i++)
    {
        alGetSourcei(i, AL_SOURCE_STATE, &val);
        
        if(val == AL_STOPPED)
        {
            world->audio_sources[i].active = false;
        }
        else
        {
            entity_p ent = World_GetEntityByID(world, world->audio_sources[i].emitter_ID);

            if(ent && (world->audio_sources[i].emitter_ID == TR_SOUND_EMITTER_ENTITY))
            {
                ALfloat buf[3]; 
                vec3_copy(buf, ent->transform + 12);
                world->audio_sources[i].SetPosition(buf);
                if(ent->character)
                {
                    vec3_copy(buf, ent->character->speed.m_floats);
                    world->audio_sources[i].SetVelocity(buf);
                }
            }
            else if(world->audio_sources[i].emitter_ID == TR_SOUND_EMITTER_SOUNDSOURCE)
            {
                //world->audio_sources[i].SetPosition(world->sound_sources[world->audio_sources[i].emitter_ID]->position);
            }
            
            if(world->audio_sources[i].emitter_ID != TR_SOUND_EMITTER_GLOBAL)
            {
                alGetListenerfv(AL_POSITION, vec);
                world->audio_sources[i].SetPosition(vec);
            }
        }
    }
#endif
}

void Audio_PauseAllSources(struct world_s *world)
{
    /*for(int i = 0; i < world->audio_sources_count; i++)
    {
        alSourcei(i, AL_SOURCE_STATE, AL_PAUSED);
    }*/
}


void Audio_ResumeAllSources(struct world_s *world)
{
    ALint val;
    
    /*for(int i = 0; i < world->audio_sources_count; i++)
    {
        alGetSourcei(i, AL_SOURCE_STATE, &val);
        if(val == AL_PAUSED)
        {
            alSourcei(i, AL_SOURCE_STATE, AL_PLAYING);
        }
    }*/
}


int Audio_GetFreeSource(struct world_s *world)
{
    /*uint32_t free_source_number;
    
    for(int i = 0; i < world->audio_sources_count; i++)
    {
        if(!world->audio_sources[i].active)
        {
            return i;
        }
    }
    */
    return -1;
}

int Audio_IsEffectPlaying(int effect_ID, int entity_ID, int entity_type, struct world_s *world)
{
    /*for(int i = 0; i < world->audio_sources_count; i++)
    {
        if((world->audio_sources[i].emitter_type == entity_type) &&
           (world->audio_sources[i].emitter_ID   == entity_ID)   &&
           (world->audio_sources[i].effect_index == effect_ID)   &&
           (world->audio_sources[i].active))
        {
            return i;
        }
        else
        {
            return -1;
        }
    }*/
}

int Audio_Send(int effect_ID, int entity_ID, int entity_type, struct world_s *world)
{
    return -1;
    /*uint32_t source_number, playing_sound;
    uint16_t random_value;
    
    // Remap global engine effect ID to local effect ID.
    effect_ID = world->audio_map[effect_ID];
    
    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.
    
    if(effect_ID == -1)
    {
        return 0;
    }
    
    // Pre-step 2: check if sound non-looped and chance to play isn't zero,
    // then randomly select if it should be played or not.
    
    if((world->audio_effects[effect_ID].loop != TR_SOUND_LOOP_LOOPED) && 
       (world->audio_effects[effect_ID].chance))
    {
        random_value = rand() % 0x7FFF;
        if(world->audio_effects[effect_ID].chance < random_value)
        {
            // Bypass audio send, if chance is not passed.
            return 0;
        }
    }
    
    // Pre-step 3: Calculate if effect's hearing sphere intersect listener's hearing sphere.
    // If it's not, bypass audio send (cause we don't want it to occupy channel, if it's not
    // heard).
    
    switch(entity_type)
    {
    case TR_SOUND_EMITTER_ENTITY:
        // ����� ������ ���� ���, �����������, ������ �� ������ � �������� ����������, �. �
        // ������������ �� ����� ������� (����������� ���������� audio_effects[effect_ID].range �
        // ���������� ���������, �. �. entity) �� ������ ��������� (������).
        
        break;
        
    case TR_SOUND_EMITTER_SOUNDSOURCE:
        // �� �� �����, ��� � � ���������� ������, �� sound_source - �� entity, � ��������� ���������
        // �����, ��������������� ��������.
        break;
        
    case TR_SOUND_EMITTER_GLOBAL:
        // ���������� ����� (����, GUI, � �. �.) ������ ������, ���������� �� ��������� � ���� ����������.
        break;
    }
    
    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and rewind it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.
    
    playing_sound = Audio_IsEffectPlaying(effect_ID, entity_ID, entity_type, world);
    
    switch(world->audio_effects[effect_ID].loop)
    {
        case TR_SOUND_LOOP_REWIND:
            if(playing_sound >= 0)
            {
                alSourceRewind(playing_sound);
                return 1;
            }
            break;
            
        case TR_SOUND_LOOP_WAIT:
           if(playing_sound >= 0)
            {
                return 0;
            }
            break;
 
        case TR_SOUND_LOOP_LOOPED:
            if(playing_sound >= 0)
            {
                // Looped sample active flag is being resetted every frame, because looped
                // source constantly sends play requests to engine. When source is active,
                // flag gets re-activated every frame, so sample continues to play. When
                // source is deactivated, it stops sending it, hence, sample is immediately
                // stopped. 
                
                world->audio_sources[playing_sound].active = false;
                return 0;
            }
            break;
            
        default:
            break;  // Non-looped or any other case.
    }
    
    // Pre-step 5: Get free source. If we can't get it, bypass audio send.
    
    source_number = Audio_GetFreeSource(world);
    
    if(source_number >= 0)  // Everything is OK, we're sending audio to channel.
    {
        // Step 1. Assign buffer to source.
        
        if(world->audio_effects[effect_ID].sample_count > 1)
        {
            // Select random buffer, if effect info contains more than 1 assigned samples.
            
            random_value = rand() % (world->audio_effects[effect_ID].sample_count - 1);
            alSourcei(source_number, AL_BUFFER, random_value + world->audio_effects[effect_ID].sample_index);
        }
        else
        {
            // Just assign buffer to source, if there is only one assigned sample.
            
            alSourcei(source_number, AL_BUFFER, world->audio_effects[effect_ID].sample_index);
        }
        
        // Step 2. Check looped flag, and if so, set source type to looped.
        
        if(world->audio_effects[effect_ID].loop == TR_SOUND_LOOP_LOOPED)
        {
            alSourcei(source_number, AL_LOOPING, AL_TRUE);
        }
        else
        {
            alSourcei(source_number, AL_LOOPING, AL_FALSE);
        }

        world->audio_sources[source_number].SetEmitterIndex(entity_ID, entity_type);
        
        world->audio_sources[source_number].active = true;
        world->audio_sources[source_number].source_index = source_number;
        
        world->audio_sources[source_number].effect_index = effect_ID;
        
        world->audio_sources[source_number].SetPitch(world->audio_effects[effect_ID].pitch);
        world->audio_sources[source_number].SetGain(world->audio_effects[effect_ID].gain);
        world->audio_sources[source_number].SetRange(world->audio_effects[effect_ID].range);
        
        alSourcePlay(source_number);
        
        return 1;
    }
    else
    {
        return -1;
    }*/
}

int Audio_Init(const int num_Sources, struct world_s *world, class VT_Level *tr)
{
    uint8_t      *pointer = tr->samples;
    int8_t        flag;
    uint32_t      ind1, ind2;
    uint32_t      comp_size, uncomp_size;
    uint32_t      i;
    
    // Generate new buffer array.
    world->audio_buffers_count = tr->sample_indices_count;
    world->audio_buffers = (ALuint*)malloc(world->audio_buffers_count * sizeof(ALuint));
    memset(world->audio_buffers, 0, sizeof(ALuint) * world->audio_buffers_count);
    alGenBuffers(world->audio_buffers_count, world->audio_buffers);
         
    // Generate new source array.
    /*world->audio_sources_count = num_Sources;
    world->audio_sources = new AudioSource[num_Sources];*/
    
    // Generate new audio effects array.
    world->audio_effects_count = tr->sound_details_count;
    world->audio_effects =  (audio_effect_t*)malloc(tr->sound_details_count * sizeof(audio_effect_t));
    memset(world->audio_effects, 0xFF, sizeof(audio_effect_t) * tr->sound_details_count);
    
    // Copy sound map.
    world->audio_map = tr->soundmap;
    tr->soundmap = NULL;                                                        /// vithout it VT destructor free(tr->soundmap)
    
    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(pointer)
    {
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                world->audio_map_count = TR_SOUND_MAP_SIZE_TR1;

                for(i = 0; i < world->audio_buffers_count-1; i++)
                {
                    pointer = tr->samples + tr->sample_indices[i];
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, (tr->sample_indices[(i+1)] - tr->sample_indices[i]));
                }
                i = world->audio_buffers_count-1;
                Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, (tr->samples_count - tr->sample_indices[i]));
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
                {
                    //world->audio_map_count = (tr->game_version == TR_III)?(TR_SOUND_MAP_SIZE_TR3):(TR_SOUND_MAP_SIZE_TR2);
                    ind1 = 0;
                    ind2 = 0;
                    flag = 0;
                    while((pointer < tr->samples + tr->samples_count - 4) && (i < world->audio_buffers_count))
                    {
                        pointer = tr->samples + ind2;
                        if(0x46464952 == *((int32_t*)pointer))              // RIFF
                        {
                            if(flag == 0x00)
                            {
                                ind1 = ind2;
                                flag = 0x01;
                            }
                            else
                            {
                                uncomp_size = ind2 - ind1;
                                Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], tr->samples + ind1, uncomp_size);
                                i++;
                                ind1 = ind2;
                            }
                        }
                        ind2++;
                    }
                    uncomp_size = tr->samples_count - ind1;
                    if(i < world->audio_buffers_count)
                    {
                        Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, uncomp_size); 
                    }                    
                }
                break;
                
            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                world->audio_map_count = (tr->game_version == TR_V)?(TR_SOUND_MAP_SIZE_TR5):(TR_SOUND_MAP_SIZE_TR4);

                for(i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is useless.
                    uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, comp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                world->audio_map_count = TR_SOUND_MAP_SIZE_NONE;
                break;
        }
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    
    for(i = 0; i < world->audio_effects_count; i++)
    {
        world->audio_effects[i].pitch  = (float)(tr->sound_details[i].pitch);
        world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume / 65535);
        world->audio_effects[i].range  = (float)(tr->sound_details[i].sound_range);
        world->audio_effects[i].chance = tr->sound_details[i].chance;
        
        world->audio_effects[i].loop = tr->sound_details[i].num_samples_and_flags_1 & TR_SOUND_LOOP_LOOPED;
        world->audio_effects[i].rand_pitch = (tr->sound_details[i].flags_2 & TR_SOUND_FLAG_RAND_PITCH);
        world->audio_effects[i].rand_gain  = (tr->sound_details[i].flags_2 & TR_SOUND_FLAG_RAND_VOLUME);
        
        world->audio_effects[i].sample_index = tr->sound_details[i].sample;
        world->audio_effects[i].sample_count = (tr->sound_details[i].num_samples_and_flags_1 >> 2) & TR_SOUND_SAMPLE_NUMBER_MASK;
    }
    
    return 0;
}

int Audio_DeInit(struct world_s *world)
{
    /*if(world->audio_sources)
    {
        delete[] world->audio_sources;
        world->audio_sources = NULL;
        world->audio_sources_count = 0;
    }*/
    extern ALuint al_source;
    
    ///@CRITICAL: You must to delete all sources before buffers deleting!!!
    alSourcePause(al_source);
    alDeleteSources(1, &al_source);
    
    if(world->audio_buffers)
    {
        alDeleteBuffers(world->audio_buffers_count, world->audio_buffers);
        free(world->audio_buffers);
        world->audio_buffers = NULL;
        world->audio_buffers_count = 0;
    }
    
    if(world->audio_effects)
    {
        free(world->audio_effects);
        world->audio_effects = NULL;
        world->audio_effects_count = 0;
    }
    
    if(world->audio_map)
    {
        free(world->audio_map);
        world->audio_map = NULL;
        world->audio_map_count = 0;
    }
    
    return 0;
}

int Audio_LoadALbufferFromWAV_Mem(ALuint buf_number, uint8_t *sample_pointer, uint32_t sample_size)
{
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer;
    Uint32 wav_length;
    int ret = 0;
    SDL_RWops *src = SDL_RWFromMem(sample_pointer, sample_size);
    
    // Decode WAV structure with SDL methods.
    // SDL automatically defines file format (PCM/ADPCM), so we shouldn't bother
    // about if it is TR4 compressed samples or TRLE uncompressed samples.
    if(SDL_LoadWAV_RW(src, 1, &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        Sys_DebugLog(LOG_FILENAME, "Error: can't load sample #%03d from sample block!", buf_number);
        SDL_FreeRW(src);
        return -1;
    }
    SDL_FreeRW(src);
    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats 
    switch(wav_spec.format & SDL_AUDIO_MASK_BITSIZE)
    {
        case 8:
            if(wav_spec.channels == 1)                                          // mono
            {
                alBufferData(buf_number, AL_FORMAT_MONO8, wav_buffer, wav_length, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf_number, AL_FORMAT_STEREO8, wav_buffer, wav_length, wav_spec.freq);
                //Sys_DebugLog(LOG_FILENAME, "Warning: sample %03d is not mono - no 3D audio!", buf_number);
            }
            else                                                                // unsupported
            {
                //Sys_DebugLog(LOG_FILENAME, "Error: sample %03d has more than 2 channels!", buf_number);
                ret = -3;
            }
            break;
            
        case 16:
            if(wav_spec.channels == 1)                                          // mono
            {
                alBufferData(buf_number, AL_FORMAT_MONO16, wav_buffer, wav_length, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf_number, AL_FORMAT_STEREO16, wav_buffer, wav_length, wav_spec.freq);
                //Sys_DebugLog(LOG_FILENAME, "Warning: sample %03d is not mono - no 3D audio!", buf_number);
            }
            else                                                                // unsupported
            {
                //Sys_DebugLog(LOG_FILENAME, "Error: sample %03d has more than 2 channels!", buf_number);
                ret = -3;
            }
    }
    
    SDL_FreeWAV(wav_buffer);
    return ret;
}


int Audio_LoadALbufferFromWAV_File(ALuint buf, const char *fname)
{
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer;
    Uint32 wav_length;
    int ret = 0;
    SDL_RWops *file;
    
    file = SDL_RWFromFile(fname, "rb");
    if(!file)
    {
        Con_Printf("Error: can not open \"%s\"", fname);
        return -1;
    }
    
    if(SDL_LoadWAV_RW(file, 1, &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        Con_Printf("Error: bad file format \"%s\"", fname);
        SDL_FreeRW(file);
        return -2;
    }
    SDL_FreeRW(file);

    switch(wav_spec.format & SDL_AUDIO_MASK_BITSIZE)
    {
        case 8:
            if(wav_spec.channels == 1)                                          // mono
            {
                alBufferData(buf, AL_FORMAT_MONO8, wav_buffer, wav_length, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf, AL_FORMAT_STEREO8, wav_buffer, wav_length, wav_spec.freq);
            }
            else                                                                // unsupported
            {
                //Con_Printf("Error: \"%s\" - unsupported channels count", fname);
                ret = -3;
            }
            break;
            
        case 16:
            if(wav_spec.channels == 1)                                          // mono
            {
                alBufferData(buf, AL_FORMAT_MONO16, wav_buffer, wav_length, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf, AL_FORMAT_STEREO16, wav_buffer, wav_length, wav_spec.freq);
            }
            else                                                                // unsupported
            {
                //Con_Printf("Error: \"%s\" - unsupported channels count", fname);
                ret = -3;
            }
            break;
            
        default:
            //Con_Printf("Error: \"%s\" - unsupported bit count", fname);
            ret = -4;
            break;
    };
    
    SDL_FreeWAV(wav_buffer);
    return ret;
}

/**
 * Updates listener parameters by camera structure. Forcorrect speed calculation
 * that function have to be called every game frame.
 * @param cam - pointer to the camera structure.
 */
void Audio_UpdateListenerByCamera(struct camera_s *cam)
{
    ALfloat v[6];       // vec3 - forvard, vec3 - up
    
    vec3_copy(v+0, cam->view_dir);
    vec3_copy(v+3, cam->up_dir);
    alListenerfv(AL_ORIENTATION, v);
    
    vec3_copy(v, cam->pos);
    alListenerfv(AL_POSITION, v);
    
    vec3_sub(v, cam->pos, cam->prev_pos);
    v[3] = 1.0 / engine_frame_time;
    vec3_mul_scalar(v, v, v[3]);
    alListenerfv(AL_VELOCITY, v);
    vec3_copy(cam->prev_pos, cam->pos);
}

//void Audio_UpdateSource(audio_source_p src)
//{
//    ALfloat v[3];
//    alSourcef(src->al_source, AL_PITCH, src->al_pitch);
//    alSourcef(src->al_source, AL_GAIN, src->al_gain);
//    vec3_mul_scalar(v, src->position, DISTANCE_COEFFICIENT);
//    alSourcefv(src->al_source, AL_POSITION, v);
//    vec3_mul_scalar(v, src->velocity, DISTANCE_COEFFICIENT);
//    alSourcefv(src->al_source, AL_VELOCITY, v);
//    alSourcei(src->al_source, AL_LOOPING, src->al_loop);
//}

//void Audio_FillSourceByEntity(audio_source_p src, struct entity_s *ent)
//{
//    vec3_copy(src->position, ent->transform + 12);
//    if(ent->character)
//    {
//        vec3_copy(src->velocity, ent->character->speed.m_floats);
//    }
//}
