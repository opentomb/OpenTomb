
#include "../../config.h"
#include "../../config.h"
#include "../../alMain.h"
#include "../../alu.h"


//void Sys_DebugLog(const char *file, const char *fmt, ...);

#if HAVE_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_platform.h>
 /**
  * Great thanks for base SDL backend code!!! Taken from that page:
  * http://openal.996291.n3.nabble.com/PATCH-SDL-1-2-backend-for-OpenAL-Soft-td5390.html
  * Code was modificated for OpenAL version 1.15.1
  * @FIXME: add capture functions implementation
  */
            
static const ALCchar sdl_device[] = "Simple Directmedia Layer";
static SDL_AudioSpec sdl_audio_spec;

static void SDLCALL sdl_callback(void *userdata, Uint8 *stream, int len)
{
    ALCdevice *device = (ALCdevice*)userdata;    
    ALint frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    aluMixData(device, stream, len/frameSize);
}

static ALCenum sdl_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    device->DeviceName = strdup(deviceName);
    device->ExtraData = &sdl_audio_spec;
    sdl_audio_spec.callback = sdl_callback;
    
    return ALC_NO_ERROR;
}

static void sdl_close_playback(ALCdevice *device)
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static ALCboolean sdl_reset_playback(ALCdevice *device)
{
    if(SDL_WasInit(SDL_INIT_AUDIO))
    {
        SDL_PauseAudio(1);
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    
    device->ExtraData = &sdl_audio_spec;
    
    sdl_audio_spec.freq = device->Frequency;
    sdl_audio_spec.channels = ChannelsFromDevFmt(device->FmtChans);
    sdl_audio_spec.samples = device->UpdateSize * sdl_audio_spec.channels;
    sdl_audio_spec.callback = sdl_callback;
    sdl_audio_spec.userdata = device;
    sdl_audio_spec.padding;                                                     ///@FIXME: what I need to do with that?
    
    switch(device->FmtType)
    {
        case DevFmtByte:
            sdl_audio_spec.format = AUDIO_S8;
            break;
            
        case DevFmtUByte:
            sdl_audio_spec.format = AUDIO_U8;;
            break;
            
        case DevFmtShort:
            sdl_audio_spec.format = AUDIO_S16LSB;
            break;
            
        case DevFmtUShort:
            sdl_audio_spec.format = AUDIO_U16LSB;
            break;

        case DevFmtInt:
            sdl_audio_spec.format = AUDIO_S32LSB;
            break;
            
        case DevFmtUInt:                                                        ///@FIXME: that format not used?
            sdl_audio_spec.format = AUDIO_S32LSB;
            break;

        case DevFmtFloat:
            sdl_audio_spec.format = AUDIO_F32LSB;
            break;
            
        default:
            //Sys_DebugLog("d_log.txt", "AL: Uncnown audio format in \"%s\", str = %d", __FILE__, __LINE__);
            ;
    }
    
    
    device->NumUpdates = 2;
    
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
    {
        //Sys_DebugLog("d_log.txt", "AL: Failed to init SDL_InitSubSystem(SDL_INIT_AUDIO) in \"%s\", str = %d", __FILE__, __LINE__);
        return ALC_FALSE;
    }

    if(SDL_OpenAudio(&sdl_audio_spec, NULL) == -1)
    {
        //Sys_DebugLog("d_log.txt", "AL: Failed to open audio in \"%s\", str = %d. ERR = %s", __FILE__, __LINE__, SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return ALC_FALSE;
    }

    SetDefaultWFXChannelOrder(device);
    SDL_PauseAudio(0);
    return ALC_TRUE;
}

ALCboolean sdl_start_playback(ALCdevice *device)
{
    (void) device;
    SDL_PauseAudio(0);
    return ALC_TRUE;
}

static void sdl_stop_playback(ALCdevice *device)
{
    (void) device;
    SDL_PauseAudio(1);
}

static ALCenum sdl_open_capture(ALCdevice *device, const ALCchar *deviceName)
{
    (void) device;
    (void) deviceName;
    return ALC_FALSE;
}

static void sdl_close_capture(ALCdevice *device)
{
    (void) device;
}

static void sdl_start_capture(ALCdevice *pDevice)
{
    (void) pDevice;
}

static void sdl_stop_capture(ALCdevice *pDevice)
{
    (void) pDevice;
}

static ALCenum sdl_capture_samples(ALCdevice *pDevice, ALCvoid *pBuffer, ALCuint lSamples)
{
    (void) pDevice;
    (void) pBuffer;
    (void) lSamples;

    return ALC_NO_ERROR;
}

static ALCuint sdl_available_samples(ALCdevice *pDevice)
{
    (void) pDevice;
    //sdl_warn_nocapture();
    return 0;
}

BackendFuncs sdl_funcs = {
    sdl_open_playback,                  // ALCenum (*OpenPlayback)(ALCdevice*, const ALCchar*);
    sdl_close_playback,                 // void (*ClosePlayback)(ALCdevice*);
    sdl_reset_playback,                 // ALCboolean (*ResetPlayback)(ALCdevice*);
    sdl_start_playback,                 // ALCboolean (*StartPlayback)(ALCdevice*);
    sdl_stop_playback,                  // void (*StopPlayback)(ALCdevice*);
    
    sdl_open_capture,                   // ALCenum (*OpenCapture)(ALCdevice*, const ALCchar*);
    sdl_close_capture,                  // void (*CloseCapture)(ALCdevice*);
    sdl_start_capture,                  // void (*StartCapture)(ALCdevice*);
    sdl_stop_capture,                   // void (*StopCapture)(ALCdevice*);
    sdl_capture_samples,                // ALCenum (*CaptureSamples)(ALCdevice*, void*, ALCuint);
    sdl_available_samples,              // ALCuint (*AvailableSamples)(ALCdevice*);
            
    ALCdevice_LockDefault,
    ALCdevice_UnlockDefault,
    ALCdevice_GetLatencyDefault
};

ALCboolean alc_sdl_init(BackendFuncs *func_list)
{
    *func_list = sdl_funcs;
    return ALC_TRUE;
}

void alc_sdl_deinit(void)
{
    // no-op.
}

 void alc_sdl_probe(enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(sdl_device);
            break;
            
        case CAPTURE_DEVICE_PROBE:
            AppendCaptureDeviceList(sdl_device);
            break;
    }
}

#endif    ///HAVE_SDL
