
#ifndef AUDIO_H
#define AUDIO_H

extern "C" {
#include "al/AL/al.h"
}

struct camera_s;
struct entity_s;

typedef struct audio_source_s
{
    ALfloat     position[3];
    ALfloat     velocity[3];
    ALfloat     al_pitch;
    ALfloat     al_gain;
    
    ALuint      al_buf;
    ALuint      al_source;
    ALint       al_loop;
}audio_source_t, *audio_source_p;


int Audio_LoadALbufferFromWAV(ALuint buf, const char *fname);

void Audio_UpdateListenerByCamera(struct camera_s *cam);
void Audio_UpdateSource(audio_source_p src);
void Audio_FillSourceByEntity(audio_source_p src, struct entity_s *ent);

#endif // AUDIO_H
