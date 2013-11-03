
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
extern "C" {
#include "al/AL/al.h"
#include "al/AL/alc.h"
#include "al/AL/alext.h"
}

#include "audio.h"
#include "console.h"
#include "camera.h"
#include "vmath.h"
#include "entity.h"
#include "character_controller.h"


int Audio_LoadALbufferFromWAV(ALuint buf, const char *fname)
{
    SDL_AudioSpec wav_spec;
    SDL_RWops *file;
    Uint8 *wav_buffer;
    Uint32 wav_length;
    int ret = 0;
    
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
        return -1;
    }
    SDL_FreeRW(file);
    
    switch(wav_spec.format & 0x00FF)
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
                Con_Printf("Error: \"%s\" - unsupported channels count", fname);
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
                Con_Printf("Error: \"%s\" - unsupported channels count", fname);
                ret = -3;
            }
            break;
            
        default:
            Con_Printf("Error: \"%s\" - unsupported bit count", fname);
            ret = -2;
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

void Audio_UpdateSource(audio_source_p src)
{
    ALfloat v[3];
    alSourcef(src->al_source, AL_PITCH, src->al_pitch);
    alSourcef(src->al_source, AL_GAIN, src->al_gain);
    vec3_copy(v, src->position);
    alSourcefv(src->al_source, AL_POSITION, v);
    vec3_copy(v, src->velocity);
    alSourcefv(src->al_source, AL_VELOCITY, v);
    alSourcei(src->al_source, AL_LOOPING, src->al_loop);
}

void Audio_FillSourceByEntity(audio_source_p src, struct entity_s *ent)
{
    vec3_copy(src->position, ent->transform + 12);
    if(ent->character)
    {
        vec3_copy(src->velocity, ent->character->speed.m_floats);
    }
}
