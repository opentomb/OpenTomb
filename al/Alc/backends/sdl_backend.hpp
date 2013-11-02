
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_platform.h>

#include "../../config.h"
#include "alMain.h"

 /**
  * Great thanks for SDL backend code!!! Taken from that page:
  * http://openal.996291.n3.nabble.com/PATCH-SDL-1-2-backend-for-OpenAL-Soft-td5390.html
  * But it does not works with current OpenAL version 1.15.1
  * @FIXME: fix that backend!!!
  */
            
static const ALCchar sdl_device[] = "Simple Directmedia Layer";

static void SDLCALL sdl_callback(void *userdata, Uint8 *stream, int len)
{
    ALCdevice *device = (ALCdevice*)userdata;    
    const ALint frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    aluMixData(device, stream, len/frameSize);
}

static ALCenum sdl_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    device->DeviceName = strdup(deviceName);
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
    SDL_AudioSpec desired;
    memset(&desired, '\0', sizeof (desired));

    if (SDL_WasInit(SDL_INIT_AUDIO))
    {
        SDL_PauseAudio(1);
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    
    switch(BytesFromDevFmt(device->FmtType))
    {
        case 1:
            desired.format = AUDIO_U8;
            break;
        case 4:
            switch(ChannelsFromDevFmt(device->FmtType))
            {
                case 1: device->FmtType = AL_FORMAT_MONO16; break;
                case 2: device->FmtType = AL_FORMAT_STEREO16; break;
                case 4: device->FmtType = AL_FORMAT_QUAD16; break;
                case 6: device->FmtType = AL_FORMAT_51CHN16; break;
                case 7: device->FmtType = AL_FORMAT_61CHN16; break;
                case 8: device->FmtType = AL_FORMAT_71CHN16; break;
            }
            // fall-through 
        case 2:
            desired.format = AUDIO_S16;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", DevFmtChannelsString(device->FmtType));
            return ALC_FALSE;
    }

    desired.freq = device->Frequency;
    desired.channels = ChannelsFromDevFmt(device->FmtType);
    desired.samples = device->UpdateSize * desired.channels;
    desired.callback = sdl_callback;
    desired.userdata = device;

    device->NumUpdates = 2;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
    {
        AL_PRINT("SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n", SDL_GetError());
        return ALC_FALSE;
    }

    if (SDL_OpenAudio(&desired, NULL) == -1)
    {
        AL_PRINT("SDL_OpenAudio failed: %s\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return ALC_FALSE;
    }

    SDL_PauseAudio(0);
    return ALC_TRUE;
}

ALCboolean sdl_start_playback(ALCdevice *device)
{
    SDL_PauseAudio(0);
    return ALC_TRUE;
}

static void sdl_stop_playback(ALCdevice *device)
{
    (void) device;
    SDL_PauseAudio(1);
}

static void sdl_warn_nocapture(void)
{
    static int warned = 0;
    if(!warned)
    {
        //AL_PRINT("No OpenAL capture support in the SDL backend at this time.\n");
        warned = 1;
    }
}

static ALCenum sdl_open_capture(ALCdevice *device, const ALCchar *deviceName)
{
    (void) device;
    (void) deviceName;
    sdl_warn_nocapture();
    return ALC_FALSE;
}

static void sdl_close_capture(ALCdevice *device)
{
    (void) device;
    sdl_warn_nocapture();
}

static void sdl_start_capture(ALCdevice *pDevice)
{
    (void) pDevice;
    sdl_warn_nocapture();
}

static void sdl_stop_capture(ALCdevice *pDevice)
{
    (void) pDevice;
    sdl_warn_nocapture();
}

static ALCenum sdl_capture_samples(ALCdevice *pDevice, ALCvoid *pBuffer, ALCuint lSamples)
{
    (void) pDevice;
    (void) pBuffer;
    (void) lSamples;
    sdl_warn_nocapture();
    return ALC_NO_ERROR;
}

static ALCuint sdl_available_samples(ALCdevice *pDevice)
{
    (void) pDevice;
    sdl_warn_nocapture();
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
 