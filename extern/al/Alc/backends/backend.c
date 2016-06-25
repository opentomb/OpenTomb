
#include "../../config.h"
#include "../../alMain.h"
#include "../../alu.h"


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
      
#define DEFAULT_AL_FX_SLOTS_COUNT               (16)
#define DEFAULT_AL_UPDATE_SIZE                  (1024*64)                       // dafault = 1024
#define DEFAULT_SDL_AUDIO_BUFFER_SIZE           (65536)                         // default = 32768


static const ALCchar sdl_device[] = "Simple Directmedia Layer";
static SDL_AudioSpec sdl_audio_desired, sdl_audio_obtained;
static SDL_AudioDeviceID sdl_dev_id = -1;


static void SDLCALL sdl_callback(void *userdata, Uint8 *stream, int len)
{
    ALCdevice *device = (ALCdevice*)userdata;    
    ALint frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    aluMixData(device, stream, len/frameSize);
}

static ALCenum sdl_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    if(deviceName)
    {
        al_string_copy_cstr(&device->DeviceName, deviceName);
    }
    else
    {
        al_string_copy_cstr(&device->DeviceName, sdl_device);
    }
    device->ExtraData = &sdl_audio_desired;
    sdl_audio_desired.callback = sdl_callback;

    return ALC_NO_ERROR;
}

static void sdl_close_playback(ALCdevice *device)
{
    SDL_PauseAudio(1);
    if(sdl_dev_id > 0)
    {
        SDL_PauseAudioDevice(sdl_dev_id, 1);
        SDL_LockAudioDevice(sdl_dev_id);
        SDL_CloseAudioDevice(sdl_dev_id);
    }
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static ALCboolean sdl_reset_playback(ALCdevice *device)
{   
    if(SDL_WasInit(SDL_INIT_AUDIO))
    {
        SDL_PauseAudio(1);
        if(sdl_dev_id > 0)
        {
            SDL_PauseAudioDevice(sdl_dev_id, 1);
            SDL_LockAudioDevice(sdl_dev_id);
            SDL_CloseAudioDevice(sdl_dev_id);
        }
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    
    device->ExtraData = &sdl_audio_obtained;
    device->UpdateSize = DEFAULT_AL_UPDATE_SIZE;                                // little UpdateSize - often updates - no play delays
    device->AuxiliaryEffectSlotMax = DEFAULT_AL_FX_SLOTS_COUNT;
    
    sdl_audio_desired.freq = device->Frequency;                                 // by default is 44100 Hz - ok
    sdl_audio_desired.callback = sdl_callback;
    sdl_audio_desired.userdata = device;
    sdl_audio_desired.padding = 0;                                              ///@FIXME: magick + what I need to do with that?
    sdl_audio_desired.format = AUDIO_F32SYS;                                    // try float 32 format
    sdl_audio_desired.samples = 0;                                              // zero it (only for obtained)
    sdl_audio_desired.size = DEFAULT_SDL_AUDIO_BUFFER_SIZE;
    sdl_audio_desired.silence = 0;                                              // zero it
    sdl_audio_desired.channels = 0;                                             // zero it (only for obtained)
    
    sdl_audio_obtained = sdl_audio_desired;
    
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        //fprintf(stderr, "AL: Failed to init SDL_InitSubSystem(SDL_INIT_AUDIO): %s", SDL_GetError());
        return ALC_FALSE;
    }
    
    switch(device->FmtChans)
    {
        case DevFmtMono:
            sdl_audio_desired.channels = 1;
            break;
            
        case DevFmtStereo:
            sdl_audio_desired.channels = 2;
            break;
            
        case DevFmtQuad:
            sdl_audio_desired.channels = 4;
            break;
            
         case DevFmtX51:
            sdl_audio_desired.channels = 6;
            break;
            
         case DevFmtX61:
            sdl_audio_desired.channels = 7;
            break;
            
         case DevFmtX71:
            sdl_audio_desired.channels = 8;
            break;
            
        default:
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            //fprintf(stderr, "AL: Wrong channels number");
            return ALC_FALSE;                  // wrong channels number
    };
 
    /*
     * try to open SDL audio device with device-default amount of channels
     */
     
    sdl_audio_desired.samples = NextPowerOf2(device->UpdateSize * sdl_audio_desired.channels);
    sdl_dev_id = SDL_OpenAudioDevice(NULL, 0, &sdl_audio_desired, &sdl_audio_obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(sdl_dev_id < 2)                                                          // SDL: valid ID >= 2
    {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        //fprintf(stderr, "AL: Invalid SDL audio device ID");
        return ALC_FALSE;
    }
    
    device->NumUpdates = 2;                                                     ///@FIXME: magick

    // set AL format
    switch(sdl_audio_obtained.format)
    {
        case AUDIO_S8:
            device->FmtType = DevFmtByte;
            break;
        
        case AUDIO_U8:
            device->FmtType = DevFmtUByte;
            break;
            
        case AUDIO_S16LSB:
        case AUDIO_S16MSB:
            device->FmtType = DevFmtShort;
            break;
            
        case AUDIO_U16LSB:
        case AUDIO_U16MSB:
            device->FmtType = DevFmtUShort;
            break;
            
        case AUDIO_S32LSB:
        case AUDIO_S32MSB:
            device->FmtType = DevFmtInt;
            break;
            
        // case AUDIO_U32LSB:                                                   // not exists in SDL
        // case AUDIO_U32MSB:
        //    device->FmtType = DevFmtUInt;
        //    break;
            
        case AUDIO_F32LSB:
        case AUDIO_F32MSB:
            device->FmtType = DevFmtFloat;
            break; 
            
        default:
            SDL_CloseAudioDevice(sdl_dev_id);
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            //fprintf(stderr, "AL: Not supported audio format");
            return ALC_FALSE;
    };
    
    SetDefaultWFXChannelOrder(device);
    SDL_UnlockAudioDevice(sdl_dev_id);
    SDL_PauseAudioDevice(sdl_dev_id, 0);
    SDL_PauseAudio(0);
    
    return ALC_TRUE;
}

ALCboolean sdl_start_playback(ALCdevice *device)
{
    (void) device;
    SDL_PauseAudioDevice(sdl_dev_id, 0);//SDL_PauseAudio(0);
    return ALC_TRUE;
}

static void sdl_stop_playback(ALCdevice *device)
{
    (void) device;
    SDL_PauseAudioDevice(sdl_dev_id, 1);//SDL_PauseAudio(1);
}

/*static ALCenum sdl_open_capture(ALCdevice *device, const ALCchar *deviceName)
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
}*/

BackendFuncs sdl_funcs = {
    sdl_open_playback,                  // ALCenum (*OpenPlayback)(ALCdevice*, const ALCchar*);
    sdl_close_playback,                 // void (*ClosePlayback)(ALCdevice*);
    sdl_reset_playback,                 // ALCboolean (*ResetPlayback)(ALCdevice*);
    sdl_start_playback,                 // ALCboolean (*StartPlayback)(ALCdevice*);
    sdl_stop_playback,                  // void (*StopPlayback)(ALCdevice*);
    
    NULL,//sdl_open_capture,                   // ALCenum (*OpenCapture)(ALCdevice*, const ALCchar*);
    NULL,//sdl_close_capture,                  // void (*CloseCapture)(ALCdevice*);
    NULL,//sdl_start_capture,                  // void (*StartCapture)(ALCdevice*);
    NULL,//sdl_stop_capture,                   // void (*StopCapture)(ALCdevice*);
    NULL,//sdl_capture_samples,                // ALCenum (*CaptureSamples)(ALCdevice*, void*, ALCuint);
    NULL,//sdl_available_samples,              // ALCuint (*AvailableSamples)(ALCdevice*);
    
    ALCdevice_GetLatencyDefault
};

ALCboolean alc_sdl_init(BackendFuncs *func_list)
{
    *func_list = sdl_funcs;
    return ALC_TRUE;
}

void alc_sdl_deinit(void)
{
    SDL_PauseAudio(0);
    if(sdl_dev_id > 0)
    {
        SDL_PauseAudioDevice(sdl_dev_id, 1);
        SDL_LockAudioDevice(sdl_dev_id);
        SDL_CloseAudioDevice(sdl_dev_id);
    }
    SDL_CloseAudio();
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
