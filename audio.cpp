
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
extern "C" {
#include "al/AL/al.h"
#include "al/AL/alc.h"
#include "al/AL/alext.h"
#include "al/AL/efx-presets.h"
}

#include "audio.h"
#include "console.h"
#include "camera.h"
#include "engine.h"
#include "vmath.h"
#include "entity.h"
#include "character_controller.h"
#include "system.h"
#include "render.h"

ALfloat                         listener_position[3];
struct audio_fxmanager_s        fxManager;
static uint8_t                  audio_blocked = 1;


AudioSource::AudioSource()
{
    active = false;
    emitter_ID =  -1;
    emitter_type = TR_AUDIO_EMITTER_ENTITY;
    effect_index = 0;
    sample_index = 0;
    sample_count = 0;
    alGenSources(1, &source_index);
    
    if(alIsSource(source_index))
    {
        alSourcef(source_index, AL_MIN_GAIN, 0.0);
        alSourcef(source_index, AL_MAX_GAIN, 1.0);
        
        if(audio_settings.use_effects)
        {
            alSourcef(source_index, AL_ROOM_ROLLOFF_FACTOR, 1.0);
            alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 1.0);
            alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,   AL_TRUE);
            alSourcei(source_index, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
        }
        else
        {
            alSourcef(source_index, AL_AIR_ABSORPTION_FACTOR, 0.0);
        }
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


bool AudioSource::IsActive()
{
    return active;
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
            
            if(audio_settings.use_effects)
            {
                UnsetFX();
            }
        }
        else
        {
            alSourcei(source_index, AL_SOURCE_RELATIVE, AL_FALSE);
            LinkEmitter();

            if(audio_settings.use_effects)
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
        active = false;
    }
}


void AudioSource::Update()
{
    ALint   state;
    ALfloat range, gain;

    alGetSourcei(source_index, AL_SOURCE_STATE, &state);
    
    // Disable and bypass source, if it is stopped.
    if(state == AL_STOPPED)
    {
        active = false;
        return;
    }
    
    // Stop source, if it is disabled, but still playing.
    if((!active) && (state == AL_PLAYING))
    {
        Stop();
        return;
    }
    
    // Bypass source, if it is paused or global.
    if( (state == AL_PAUSED) || (emitter_type == TR_AUDIO_EMITTER_GLOBAL) )
    {
        return;
    }
    
    alGetSourcef(source_index, AL_GAIN, &gain);
    alGetSourcef(source_index, AL_MAX_DISTANCE, &range);
    
    // Check if source is in listener's range, and if so, update position,
    // else stop and disable it.
    if(Audio_IsInRange(emitter_type, emitter_ID, range, gain))
    {
        LinkEmitter();
        
        if( (audio_settings.use_effects) && (is_water != fxManager.water_state) )
        {
            SetUnderwater();
        }
    }
    else
    {
        Stop();
    }
}


void AudioSource::SetBuffer(ALint buffer)
{
    ALint buffer_index = engine_world.audio_buffers[buffer];
    
    if(alIsSource(source_index) && alIsBuffer(buffer_index))
    {
        alSourcei(source_index, AL_BUFFER, buffer_index);
    }
}


void AudioSource::SetLooping(ALboolean is_looping)
{
    alSourcei(source_index, AL_LOOPING, is_looping);
}


void AudioSource::SetGain(ALfloat gain_value)
{
    // Clamp gain value.
    gain_value = (gain_value > 1.0)?(1.0):(gain_value);
    gain_value = (gain_value < 0.0)?(0.0):(gain_value);
    
    alSourcef(source_index, AL_GAIN, gain_value * audio_settings.sound_volume);
}


void AudioSource::SetPitch(ALfloat pitch_value)
{
    // Clamp pitch value, as OpenAL tends to hang with incorrect ones.
    pitch_value = (pitch_value < 0.1)?(0.1):(pitch_value);
    pitch_value = (pitch_value > 2.0)?(2.0):(pitch_value);
    
    alSourcef(source_index, AL_PITCH, pitch_value);
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
    
    if(fxManager.current_room_type != fxManager.last_room_type)  // Switch audio send.
    {
        fxManager.last_room_type = fxManager.current_room_type;
        fxManager.current_slot   = (++fxManager.current_slot > (TR_AUDIO_MAX_SLOTS-1))?(0):(fxManager.current_slot);
        
        effect = fxManager.al_effect[fxManager.current_room_type];
        slot   = fxManager.al_slot[fxManager.current_slot];
        
        if(alIsAuxiliaryEffectSlot(slot) && alIsEffect(effect))
        {
            alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
        }
    }
    else    // Do not switch audio send.
    {
        slot = fxManager.al_slot[fxManager.current_slot];
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
    
    if(fxManager.water_state)
    {
        alSourcei(source_index, AL_DIRECT_FILTER, fxManager.al_filter);
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
    ALfloat  vec[3];
    entity_p ent;
    
    switch(emitter_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            ent = World_GetEntityByID(&engine_world, emitter_ID);
            if(ent)
            {
                vec3_copy(vec, ent->transform + 12);
                SetPosition(vec);

                if(ent->character)
                {
                    vec3_copy(vec, ent->character->speed.m_floats);
                    SetVelocity(vec);
                }
            }
            return;
            
        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            SetPosition(engine_world.audio_emitters[emitter_ID].position);
            return;
    }
}


bool Audio_IsInRange(int entity_type, int entity_ID, float range, float gain)
{
    ALfloat  vec[3], dist;
    entity_p ent;
    
    switch(entity_type)
    {
        case TR_AUDIO_EMITTER_ENTITY:
            ent = World_GetEntityByID(&engine_world, entity_ID);
            if(!ent)
            {
                return false;
            }
            vec3_copy(vec, ent->transform + 12);
            break;
            
        case TR_AUDIO_EMITTER_SOUNDSOURCE:
            if(entity_ID > engine_world.audio_emitters_count - 1)
            {
                return false;
            }
            vec3_copy(vec, engine_world.audio_emitters[entity_ID].position);
            break;
            
        case TR_AUDIO_EMITTER_GLOBAL:
            return true;
    }
        
    dist = vec3_dist_sq(listener_position, vec);
    
    // We add 1/4 of overall distance to fix up some issues with
    // pseudo-looped sounds that are called at certain frames in animations.
    
    dist /= (gain + 1.25);
    
    return dist < range * range;
}


void Audio_UpdateSources()
{
    uint32_t i;
    
    if(audio_blocked || (engine_world.audio_sources_count < 1))
    {
        return;
    }
    
    alGetListenerfv(AL_POSITION, listener_position);
    
    for(i = 0; i < engine_world.audio_emitters_count; i++)
    {
        Audio_Send(engine_world.audio_emitters[i].sound_index, TR_AUDIO_EMITTER_SOUNDSOURCE, i);
    }
    
    for(i = 0; i < engine_world.audio_sources_count; i++)
    {
        engine_world.audio_sources[i].Update();
    }
}


void Audio_PauseAllSources()
{
    for(int i = 0; i < engine_world.audio_sources_count; i++)
    {
        if(engine_world.audio_sources[i].IsActive())
        {
            engine_world.audio_sources[i].Pause();
        }
    }
}

void Audio_StopAllSources()
{
    for(int i = 0; i < engine_world.audio_sources_count; i++)
    {
        engine_world.audio_sources[i].Stop();
    }
}

void Audio_ResumeAllSources()
{
    for(int i = 0; i < engine_world.audio_sources_count; i++)
    {
        if(engine_world.audio_sources[i].IsActive())
        {
            engine_world.audio_sources[i].Play();
        }
    }
}


///@FIXME: add condition (compare max_dist with new source dist)
int Audio_GetFreeSource()
{    
    for(int i = 0; i < engine_world.audio_sources_count; i++)
    {
        if(engine_world.audio_sources[i].IsActive() == false)
        {
            return i;
        }
    }
    
    return -1;
}


int Audio_IsEffectPlaying(int effect_ID, int entity_type, int entity_ID)
{
    int state;
    
    for(int i = 0; i < engine_world.audio_sources_count; i++)
    {
        if( (engine_world.audio_sources[i].emitter_type == entity_type) &&
            (engine_world.audio_sources[i].emitter_ID   == entity_ID  ) &&
            (engine_world.audio_sources[i].effect_index == effect_ID  ) )
        {
            alGetSourcei(engine_world.audio_sources[i].source_index, AL_SOURCE_STATE, &state);
            
            if(state == AL_PLAYING)
            {
                return i;
            }
        }
    }
    
    return -1;
}


int Audio_Send(int effect_ID, int entity_type, int entity_ID)
{
    int32_t         source_number;
    uint16_t        random_value;
    ALfloat         random_float;
    audio_effect_p  effect = NULL;
    AudioSource    *source = NULL;
    
    // Remap global engine effect ID to local effect ID.
    if((effect_ID < 0) || (effect_ID > engine_world.audio_map_count - 1))
    {
        return TR_AUDIO_SEND_NOSAMPLE;
    }
    effect_ID = (int)engine_world.audio_map[effect_ID];
    
    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.
    
    if((effect_ID == -1) || audio_blocked)
    {
        return TR_AUDIO_SEND_NOSAMPLE;
    }
    effect = engine_world.audio_effects + effect_ID;
            
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
    
    if(Audio_IsInRange(entity_type, entity_ID, effect->range, effect->gain ) == false)
    {
        return TR_AUDIO_SEND_IGNORED;
    }
    
    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and stop it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.
    
    source_number = Audio_IsEffectPlaying(effect_ID, entity_type, entity_ID);
    
    if(source_number != -1)
    {
        if(effect->loop == TR_AUDIO_LOOP_REWIND)
        {
            engine_world.audio_sources[source_number].Stop();
        }
        else if(effect->loop) // Any other looping case (Wait / Loop).
        {
            return TR_AUDIO_SEND_IGNORED;
        }
    }
    else
    {
        source_number = Audio_GetFreeSource();  // Get free source. 
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
        
        source = &engine_world.audio_sources[source_number];
        
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
        
        source->emitter_ID   = entity_ID;
        source->emitter_type = entity_type;
        source->effect_index = effect_ID;
        
        if(effect->rand_pitch)
        {
            random_float = rand() % effect->rand_pitch_var;
            random_float = effect->pitch + ((random_float - 25.0) / 200.0);
            source->SetPitch(random_float);
        }
        else
        {
            source->SetPitch(effect->pitch);
        }
        
        if(effect->rand_gain)
        {
            random_float = rand() % effect->rand_gain_var;
            random_float = effect->gain + (random_float - 25.0) / 200.0;            
            source->SetGain(random_float);
        }
        else
        {
            source->SetGain(effect->gain);
        }
        
        source->SetRange(effect->range);
        
        source->Play();
        
        return TR_AUDIO_SEND_PROCESSED;
    }
    else
    {
        return TR_AUDIO_SEND_NOCHANNEL;
    }
}


int Audio_Kill(int effect_ID, int entity_type, int entity_ID)
{
    effect_ID = (int)engine_world.audio_map[effect_ID];
    
    int playing_sound = Audio_IsEffectPlaying(effect_ID, entity_type, entity_ID);
    
    if(playing_sound != -1)
    {
        engine_world.audio_sources[playing_sound].Stop();
        return TR_AUDIO_SEND_PROCESSED;
    }
    
    return TR_AUDIO_SEND_IGNORED;
}   


int Audio_Init(const int num_Sources, class VT_Level *tr)
{
    uint8_t      *pointer = tr->samples_data;
    int8_t        flag;
    uint32_t      ind1, ind2;
    uint32_t      comp_size, uncomp_size;
    uint32_t      i;
    
    // FX should be inited first, as source constructor checks for FX slot to be created.
    
    if(audio_settings.use_effects)
    {
        Audio_InitFX();
    }
    
    // Generate new buffer array.
    engine_world.audio_buffers_count = tr->samples_count;
    engine_world.audio_buffers = (ALuint*)malloc(engine_world.audio_buffers_count * sizeof(ALuint));
    memset(engine_world.audio_buffers, 0, sizeof(ALuint) * engine_world.audio_buffers_count);
    alGenBuffers(engine_world.audio_buffers_count, engine_world.audio_buffers);
         
    // Generate new source array.
    engine_world.audio_sources_count = num_Sources;
    engine_world.audio_sources = new AudioSource[num_Sources];
    
    // Generate new audio effects array.
    engine_world.audio_effects_count = tr->sound_details_count;
    engine_world.audio_effects =  (audio_effect_t*)malloc(tr->sound_details_count * sizeof(audio_effect_t));
    memset(engine_world.audio_effects, 0xFF, sizeof(audio_effect_t) * tr->sound_details_count);

    // Generate new audio emitters array.
    engine_world.audio_emitters_count = tr->sound_sources_count;
    engine_world.audio_emitters = (audio_emitter_t*)malloc(tr->sound_sources_count * sizeof(audio_emitter_t));
    memset(engine_world.audio_emitters, 0, sizeof(audio_emitter_t) * tr->sound_sources_count);
    
    // Copy sound map.
    engine_world.audio_map = tr->soundmap;
    tr->soundmap = NULL;                   /// without it VT destructor free(tr->soundmap)
    
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
                engine_world.audio_map_count = TR_AUDIO_MAP_SIZE_TR1;

                for(i = 0; i < engine_world.audio_buffers_count-1; i++)
                {
                    pointer = tr->samples_data + tr->sample_indices[i];
                    Audio_LoadALbufferFromWAV_Mem(engine_world.audio_buffers[i], pointer, (tr->sample_indices[(i+1)] - tr->sample_indices[i]));
                }
                i = engine_world.audio_buffers_count-1;
                Audio_LoadALbufferFromWAV_Mem(engine_world.audio_buffers[i], pointer, (tr->samples_count - tr->sample_indices[i]));
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
                engine_world.audio_map_count = (tr->game_version == TR_III)?(TR_AUDIO_MAP_SIZE_TR3):(TR_AUDIO_MAP_SIZE_TR2);
                ind1 = 0;
                ind2 = 0;
                flag = 0;
                i = 0;
                while(pointer < tr->samples_data + tr->samples_data_size - 4)
                {
                    pointer = tr->samples_data + ind2;
                    if(0x46464952 == *((int32_t*)pointer))  // RIFF
                    {
                        if(flag == 0x00)
                        {
                            ind1 = ind2;
                            flag = 0x01;
                        }
                        else
                        {
                            uncomp_size = ind2 - ind1;
                            Audio_LoadALbufferFromWAV_Mem(engine_world.audio_buffers[i], tr->samples_data + ind1, uncomp_size);
                            i++;
                            if(i > engine_world.audio_buffers_count - 1)
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                uncomp_size = tr->samples_data_size - ind1;
                pointer = tr->samples_data + ind1;
                if(i < engine_world.audio_buffers_count)
                {
                    Audio_LoadALbufferFromWAV_Mem(engine_world.audio_buffers[i], pointer, uncomp_size); 
                }
                break;
                
            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                engine_world.audio_map_count = (tr->game_version == TR_V)?(TR_AUDIO_MAP_SIZE_TR5):(TR_AUDIO_MAP_SIZE_TR4);

                for(i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromWAV_Mem(engine_world.audio_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                engine_world.audio_map_count = TR_AUDIO_MAP_SIZE_NONE;
                free(tr->samples_data);
                tr->samples_data = NULL;
                tr->samples_data_size = 0;
                return 0;
        }
        
        free(tr->samples_data);
        tr->samples_data = NULL;
        tr->samples_data_size = 0;
    }
    
    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(i = 0; i < engine_world.audio_effects_count; i++)
    {
        if(tr->game_version < TR_III)
        {
            engine_world.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 32767.0; // Max. volume in TR1/TR2 is 32767.
            engine_world.audio_effects[i].chance = tr->sound_details[i].chance;
        }
        else if(tr->game_version > TR_III)
        {
            engine_world.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            engine_world.audio_effects[i].chance = tr->sound_details[i].chance * 255;
        }
        else
        {
            engine_world.audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            engine_world.audio_effects[i].chance = tr->sound_details[i].chance * 127;
        }
                    
        engine_world.audio_effects[i].rand_gain_var  = 50;
        engine_world.audio_effects[i].rand_pitch_var = 50;
        
        engine_world.audio_effects[i].pitch = (float)(tr->sound_details[i].pitch) / 127.0 + 1.0;
        engine_world.audio_effects[i].range = (float)(tr->sound_details[i].sound_range) * 1024.0;
        
        engine_world.audio_effects[i].rand_pitch = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_PITCH);
        engine_world.audio_effects[i].rand_gain  = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_VOLUME);
        
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    case 0x01:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    default:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;
                
            case TR_II:
            case TR_II_DEMO:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_WAIT;
                        break;
                    case 0x01:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    case 0x03:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    default:
                        engine_world.audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;
                
            default:
                engine_world.audio_effects[i].loop = (tr->sound_details[i].num_samples_and_flags_1 & TR_AUDIO_LOOP_LOOPED);
        }
        
        engine_world.audio_effects[i].sample_index = tr->sound_details[i].sample;
        engine_world.audio_effects[i].sample_count = (tr->sound_details[i].num_samples_and_flags_1 >> 2) & TR_AUDIO_SAMPLE_NUMBER_MASK;
    }
    
    // Cycle through sound emitters and 
    // parse them to native OpenTomb sound emitters structure.
    
    for(i = 0; i < engine_world.audio_emitters_count; i++)
    {
        engine_world.audio_emitters[i].emitter_index = i;
        engine_world.audio_emitters[i].sound_index   =  tr->sound_sources[i].sound_id;
        engine_world.audio_emitters[i].position[0]   =  tr->sound_sources[i].x;
        engine_world.audio_emitters[i].position[1]   =  tr->sound_sources[i].z;
        engine_world.audio_emitters[i].position[2]   = -tr->sound_sources[i].y;
        engine_world.audio_emitters[i].flags         =  tr->sound_sources[i].flags;
    }
    audio_blocked = 0;
    
    return 1;
}

void Audio_InitGlobals()
{
    audio_settings.music_volume = 0.7;
    audio_settings.sound_volume = 0.8;
    audio_settings.use_effects  = true;
    audio_settings.listener_is_player = false;
}

void Audio_InitFX()
{
    EFXEAXREVERBPROPERTIES reverb = {0};
    memset(&fxManager, 0, sizeof(audio_fxmanager_s));
    
    // Set up effect slots, effects and filters.
    
    alGenAuxiliaryEffectSlots(TR_AUDIO_MAX_SLOTS, fxManager.al_slot);
    alGenEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    alGenFilters(1, &fxManager.al_filter);
    
    alFilteri(fxManager.al_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAIN, 0.7f);      // Low frequencies gain.
    alFilterf(fxManager.al_filter, AL_LOWPASS_GAINHF, 0.0f);    // High frequencies gain.
    
    // Fill up effects with reverb presets.
    
    reverb = EFX_REVERB_PRESET_CITY;
    Audio_LoadReverbToFX(TR_AUDIO_FX_OUTSIDE, &reverb);

    reverb = EFX_REVERB_PRESET_LIVINGROOM;
    Audio_LoadReverbToFX(TR_AUDIO_FX_SMALLROOM, &reverb);
    
    reverb = EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
    Audio_LoadReverbToFX(TR_AUDIO_FX_MEDIUMROOM, &reverb);
    
    reverb = EFX_REVERB_PRESET_DOME_TOMB;
    Audio_LoadReverbToFX(TR_AUDIO_FX_LARGEROOM, &reverb);

    reverb = EFX_REVERB_PRESET_PIPE_LARGE;
    Audio_LoadReverbToFX(TR_AUDIO_FX_PIPE, &reverb);

    reverb = EFX_REVERB_PRESET_UNDERWATER;
    Audio_LoadReverbToFX(TR_AUDIO_FX_WATER, &reverb);
}

int Audio_LoadReverbToFX(const int effect_index, const EFXEAXREVERBPROPERTIES *reverb)
{
    ALenum err;
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
        Sys_DebugLog(LOG_FILENAME, "OpenAL error: no effect %d", effect);
        return 0;
    }
    
    return 1;
}

int Audio_DeInit()
{
    audio_blocked = 1;
    Audio_StopAllSources();
   
    if(engine_world.audio_sources)
    {
        delete[] engine_world.audio_sources;
        engine_world.audio_sources = NULL;
        engine_world.audio_sources_count = 0;
    }
    
    ///@CRITICAL: You must delete all sources before buffers deleting!!!

    if(engine_world.audio_buffers)
    {
        alDeleteBuffers(engine_world.audio_buffers_count, engine_world.audio_buffers);
        free(engine_world.audio_buffers);
        engine_world.audio_buffers = NULL;
        engine_world.audio_buffers_count = 0;
    }
    
    if(engine_world.audio_effects)
    {
        free(engine_world.audio_effects);
        engine_world.audio_effects = NULL;
        engine_world.audio_effects_count = 0;
    }
    
    if(engine_world.audio_sources)
    {
        free(engine_world.audio_sources);
        engine_world.audio_sources = NULL;
        engine_world.audio_effects_count = 0;
    }
    
    if(engine_world.audio_map)
    {
        free(engine_world.audio_map);
        engine_world.audio_map = NULL;
        engine_world.audio_map_count = 0;
    }
    
    if(audio_settings.use_effects)
    {
        for(int i = 0; i < TR_AUDIO_MAX_SLOTS; i++)
        {
            alAuxiliaryEffectSloti(fxManager.al_slot[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
            alDeleteAuxiliaryEffectSlots(1, &fxManager.al_slot[i]);
        }
        
        alDeleteFilters(1, &fxManager.al_filter);
        alDeleteEffects(TR_AUDIO_FX_LASTINDEX, fxManager.al_effect);
    }
    
    return 1;
}


void Audio_LogALError(int proc_index)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        Sys_DebugLog(LOG_FILENAME, "OpenAL error: %s [INDEX %d]", alGetString(err), proc_index);
    }
}


int Audio_LoadALbufferFromWAV_Mem(ALuint buf_number, uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size)
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
        //SDL_FreeRW(src);
        return -1;
    }
    //SDL_FreeRW(src);
    
    // Uncomp_sample_size explicitly specifies amount of raw sample data
    // to load into buffer. It is only used in TR4/5 with ADPCM samples,
    // because full-sized ADPCM sample contains a bit of silence at the end,
    // which should be removed. That's where uncomp_sample_size comes into
    // business.
    // Note that we also need to compare if uncomp_sample_size is smaller
    // than native wav length, because for some reason many TR5 uncomp sizes
    // are messed up and actually more than actual sample size.
    
    if((uncomp_sample_size == 0) || (wav_length < uncomp_sample_size))
    {
        uncomp_sample_size = wav_length;
    }
         
    // Find out sample format and load it correspondingly.
    // Note that with OpenAL, we can have samples of different formats in same level.
    
    switch(wav_spec.format & SDL_AUDIO_MASK_BITSIZE)
    {
        case 8:
            if(wav_spec.channels == 1)                                          // mono
            {
                alBufferData(buf_number, AL_FORMAT_MONO8, wav_buffer, uncomp_sample_size, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf_number, AL_FORMAT_STEREO8, wav_buffer, uncomp_sample_size, wav_spec.freq);
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
                alBufferData(buf_number, AL_FORMAT_MONO16, wav_buffer, uncomp_sample_size, wav_spec.freq);
            }
            else if(wav_spec.channels == 2)                                     // stereo
            {
                alBufferData(buf_number, AL_FORMAT_STEREO16, wav_buffer, uncomp_sample_size, wav_spec.freq);
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
        //SDL_FreeRW(file);
        return -2;
    }
    //SDL_FreeRW(file);

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
    int room_type = 0;
    
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
    
    if(cam->current_room)
    {
        if(cam->current_room->flags & 0x01)
        {
            fxManager.current_room_type = TR_AUDIO_FX_WATER;
        }
        else
        {
            fxManager.current_room_type = cam->current_room->reverb_info;
        }
        
        if(fxManager.water_state != (cam->current_room->flags & 0x01))
        {
            fxManager.water_state = cam->current_room->flags & 0x01;
            
            if(fxManager.water_state)
            {
                Audio_Send(60);
            }
            else
            {
                Audio_Kill(60);
            }
        }
    }
}


