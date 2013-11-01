
#ifndef AUDIO_H
#define AUDIO_H

extern "C" {
#include "al/AL/al.h"
}

int Audio_LoadALbufferFromWAV(ALuint buf, const char *fname);




#endif // AUDIO_H
