
#ifndef AUDIO_H
#define AUDIO_H

extern "C" {
#include "al/AL/al.h"
}

#include "vt/vt_level.h"
#include "system.h"

// MAX_CHANNELS defines maximum amount of sound sources (channels)
// that can play at the same time. Contemporary devices can play
// up to 256 channels, but we set it to 32 for compatibility
// reasons.

#define MAX_CHANNELS 32

// Sound map size is a size of effect ID array, which is used to translate
// global effect IDs to level effect IDs. If effect ID in sound map is -1
// (0xFFFF), it means that this effect is absent in current level.

#define TR_SOUND_MAP_SIZE_NONE -1
#define TR_SOUND_MAP_SIZE_TR1 256
#define TR_SOUND_MAP_SIZE_TR2 370
#define TR_SOUND_MAP_SIZE_TR3 370
#define TR_SOUND_MAP_SIZE_TR4 370
#define TR_SOUND_MAP_SIZE_TR5 450

// TR4 sound flags are found at offset 7 of SoundDetail unit and specify
// certain sound modifications.

#define TR_SOUND_FLAG_RAND_PITCH 0x20
#define TR_SOUND_FLAG_RAND_VOLUME 0x40
#define TR_SOUND_FLAG_N 0x10           // UNKNOWN MEANING!

// Looped field is located at offset 6 of SoundDetail structure and
// combined with SampleIndexes value. This field is responsible for
// looping behaviour of each sound.
// L flag sets sound to continous looped state, while W flag waits
// for any sound with similar ID to finish, and only then plays it
// again. R flag rewinds sound, if sound with similar ID is being
// sent to sources.

#define TR_SOUND_LOOP_LOOPED  0x03  // L flag. Normal looped.
#define TR_SOUND_LOOP_REWIND  0x02  // R flag. Rewind on re-send.
#define TR_SOUND_LOOP_WAIT    0x01  // W flag. Wait until play is over.
#define TR_SOUND_LOOP_NONE    0x00  // No looping.

// Sample number mask is a mask value used in bitwise operation with
// "num_samples_and_flags_1" field to extract amount of samples per
// effect.

#define TR_SOUND_SAMPLE_NUMBER_MASK 0x0F

// Default range and pitch values are required for compatibility with
// TR1 and TR2 levels, as there is no such parameters in SoundDetails
// structures.

#define TR_SOUND_DEFAULT_RANGE 1
#define TR_SOUND_DEFAULT_PITCH 1

// Entity types are used to identify different sound emitter types. Since
// sounds in TR games could be emitted either by entities, sound sources
// or global events, we have defined these three types of emitters.

#define TR_SOUND_EMITTER_ENTITY 0
#define TR_SOUND_EMITTER_SOUNDSOURCE 1
#define TR_SOUND_EMITTER_GLOBAL 2

struct camera_s;
struct entity_s;

typedef struct audio_settings_s
{
    ALuint      music_volume;
    ALuint      sounds_volume;
    ALboolean   use_effects;
    ALboolean   listener_is_player;
    
}audio_settings_t, *audio_settings_p;

// Effect structure.
// Contains all global effect parameters.

typedef struct audio_effect_s
{	
    // General sound source parameters (similar to TR sound info).
    
    ALfloat     pitch;          // [PIT in TR] Global pitch shift.
    ALfloat     gain;           // [VOL in TR] Global gain (volume).
    ALfloat     range;          // [RAD in TR] Range (radius).
    ALuint      chance;         // [CH  in TR] Chance to play.
    ALuint      loop;           // 0 = none, 1 = W, 2 = R, 3 = L.
    ALboolean   rand_pitch;     // Similar to flag 0x200000 (P) in native TRs.
    ALboolean   rand_gain;      // Similar to flag 0x400000 (V) in native TRs.
	
	// Additional sound source parameters.
	// These are not natively in TR engines, but can be later assigned by
	// external script.
	
    ALboolean   rand_freq;          // Slightly randomize frequency.
	ALuint		rand_pitch_var;     // Pitch randomizer bounds.
	ALuint		rand_gain_var;      // Gain  randomizer bounds.
	ALuint		rand_freq_var;      // Frequency randomizer bounds.

    // Sample reference parameters.
    
    ALuint      sample_index;       // First (or only) sample (buffer) index.
    ALuint      sample_count;       // Sample amount to randomly select from.
    
}audio_effect_t, *audio_effect_p;


// Main audio source class.

// Sound source is a complex class, each member of which is linked/not linked with
// certain in-game entity or sound source, but also a kind of entity by itself.
// Number of simultaneously existing sound sources is fixed, and can't be more than
// MAX_CHANNELS global constant. Indeed, it can be active or not active

class AudioSource
{
public:
    AudioSource();  // Audio source constructor.
   ~AudioSource();	// Audio source destructor.
   
    void Play();
    void Pause();
    void Stop();
    void Update();

    void SetBuffer(ALint buffer);
    void SetLooping(ALboolean is_looping);
    void SetPitch(ALfloat pitch_value);
    void SetGain(ALfloat gain_value);
    void SetRange(ALfloat range_value);
    
    bool IsActive();
    void LinkEmitter();
    void SetPosition(const ALfloat pos_vector[]);
    void SetVelocity(const ALfloat vel_vector[]);
    
    int32_t     emitter_ID;     // Entity of origin. -1 means no entity (hence - empty source).
    uint32_t    emitter_type;   // 0 - ordinary entity, 1 - sound source, 2 - global sound.
    uint32_t    effect_index;   // Effect index. Used to associate effect with entity for R/W flags.
    uint32_t    sample_index;   // OpenAL sample (buffer) index. May be the same for different sources.
    uint32_t    sample_count;   // How many buffers to use, beginning with sample_index.
    bool        is_water;       // Environmental flag; if source/listener flags aren't equal, sample will damp.
    
private:
    bool        active;         // Source gets autostopped and destroyed on next frame, if it's not set.
    ALuint      source_index;   // Source index. Should be unique for each source.
};

int  Audio_Init(const int num_Sources, class VT_Level *tr);
int  Audio_DeInit();

int  Audio_GetFreeSource();
bool Audio_IsInRange(int emitter_type, int emitter_ID, float gain);

void Audio_UpdateSources();     // Main sound loop.
void Audio_PauseAllSources();	// Used to pause all effects currently playing.
void Audio_ResumeAllSources();	// Used to resume all effects currently paused.

int  Audio_Send(int effect_ID, int entity_ID, int entity_type);	// Send to play effect with given parameters.
void Audio_Kill(int effect_ID, int entity_ID, int entity_type);	// If exist, immediately stop and destroy all effects with given parameters.

int Audio_IsEffectPlaying(int effect_ID, int entity_ID, int entity_type);

// void Audio_SetFX(bool reverb, bool underwater, bool outside);   // Set global effects for all sources.

int Audio_LoadALbufferFromWAV_Mem(ALuint buf, uint8_t *sample_pointer, uint32_t sample_size);
int Audio_LoadALbufferFromWAV_File(ALuint buf, const char *fname);

// DEPRECATED

void Audio_UpdateListenerByCamera(struct camera_s *cam);

#endif // AUDIO_H
