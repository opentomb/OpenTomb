#ifndef AUDIO_AL_H
#define AUDIO_AL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <AL/al.h>
#include <AL/alc.h>

#ifdef HAVE_ALEXT_H
#include <AL/alext.h>
#endif

#ifdef HAVE_EFX_H
#include <AL/efx.h>
#endif

#ifdef HAVE_EFX_PRESETS_H
#include <AL/efx-presets.h>
#endif

#ifdef __cplusplus
}
#endif

#endif // AUDIO_AL_H

