
#include "../../config.h"

#ifdef HAVE_PULSEAUDIO
#include "pulseaudio.hpp"
#endif
#ifdef HAVE_ALSA
#include "alsa.hpp"
#endif
#ifdef HAVE_COREAUDIO
#include "coreaudio.hpp"
#endif
#ifdef HAVE_OSS
#include "oss.hpp"
#endif
#ifdef HAVE_SOLARIS
#include "solaris.hpp"
#endif
#ifdef HAVE_SNDIO
#include "sndio.hpp"
#endif
#ifdef HAVE_QSA
#include "qsa.hpp"
#endif
#ifdef HAVE_MMDEVAPI
#include "mmdevapi.hpp"
#endif
#ifdef HAVE_DSOUND
#include "dsound.hpp"
#endif
#ifdef HAVE_WINMM
#include "winmm.hpp"
#endif
#ifdef HAVE_SDL
#include "sdl_backend.hpp"
#endif
#ifdef HAVE_PORTAUDIO
#include "portaudio.hpp"
#endif
#ifdef HAVE_OPENSL
#include "opensl.hpp"
#endif

