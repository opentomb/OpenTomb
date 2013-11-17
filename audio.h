
#ifndef AUDIO_H
#define AUDIO_H

extern "C" {
#include "al/AL/al.h"
}

#include "vt/vt_level.h"
#include "system.h"

// AL_UNITS constant is used to translate native TR coordinates into
// OpenAL coordinates. By default, it's the same as geometry grid
// resolution (1024).

#define TR_AUDIO_AL_UNITS 1024.0

// MAX_CHANNELS defines maximum amount of sound sources (channels)
// that can play at the same time. Contemporary devices can play
// up to 256 channels, but we set it to 32 for compatibility
// reasons.

#define TR_AUDIO_MAX_CHANNELS 32

// Audio map size is a size of effect ID array, which is used to translate
// global effect IDs to level effect IDs. If effect ID in audio map is -1
// (0xFFFF), it means that this effect is absent in current level.
// Normally, audio map size is a constant for each TR game version and
// won't change from level to level.

#define TR_AUDIO_MAP_SIZE_NONE (-1)
#define TR_AUDIO_MAP_SIZE_TR1 256
#define TR_AUDIO_MAP_SIZE_TR2 370
#define TR_AUDIO_MAP_SIZE_TR3 370
#define TR_AUDIO_MAP_SIZE_TR4 370
#define TR_AUDIO_MAP_SIZE_TR5 450

// Sound flags are found at offset 7 of SoundDetail unit and specify
// certain sound modifications.

#define TR_AUDIO_FLAG_RAND_PITCH  0x20 // Slight random pitch shift.
#define TR_AUDIO_FLAG_RAND_VOLUME 0x40 // Slight random gain shift.
#define TR_AUDIO_FLAG_UNKNOWN     0x10 // UNKNOWN MEANING!

// Looped field is located at offset 6 of SoundDetail structure and
// combined with SampleIndexes value. This field is responsible for
// looping behaviour of each sound.
// L flag sets sound to continous looped state, while W flag waits
// for any sound with similar ID to finish, and only then plays it
// again. R flag rewinds sound, if sound with similar ID is being
// sent to sources.

#define TR_AUDIO_LOOP_LOOPED  0x03  // L flag. Normal looped.
#define TR_AUDIO_LOOP_REWIND  0x02  // R flag. Rewind on re-send.
#define TR_AUDIO_LOOP_WAIT    0x01  // W flag. Wait until play is over.
#define TR_AUDIO_LOOP_NONE    0x00  // No looping.

// Sample number mask is a mask value used in bitwise operation with
// "num_samples_and_flags_1" field to extract amount of samples per
// effect.

#define TR_AUDIO_SAMPLE_NUMBER_MASK 0x0F

// Default range and pitch values are required for compatibility with
// TR1 and TR2 levels, as there is no such parameters in SoundDetails
// structures.

#define TR_AUDIO_DEFAULT_RANGE 8
#define TR_AUDIO_DEFAULT_PITCH 0

// Entity types are used to identify different sound emitter types. Since
// sounds in TR games could be emitted either by entities, sound sources
// or global events, we have defined these three types of emitters.

#define TR_AUDIO_EMITTER_ENTITY 0        // Entity (movable)
#define TR_AUDIO_EMITTER_SOUNDSOURCE 1   // Sound source (static)
#define TR_AUDIO_EMITTER_GLOBAL 2        // Global sound (menus, secret, etc.)

// Possible types of errors returned by Audio_Send / Audio_Kill functions.

#define TR_AUDIO_SEND_NOSAMPLE   (-2)
#define TR_AUDIO_SEND_NOCHANNEL  (-1)
#define TR_AUDIO_SEND_IGNORED    0
#define TR_AUDIO_SEND_PROCESSED  1

struct camera_s;
struct entity_s;

// Audio settings structure.

typedef struct audio_settings_s
{
    ALfloat     music_volume;       // RESERVED FOR FUTURE USE
    ALfloat     sound_volume;
    ALboolean   use_effects;        // RESERVED FOR FUTURE USE
    ALboolean   listener_is_player; // RESERVED FOR FUTURE USE
    
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
    ALuint      rand_pitch_var;     // Pitch randomizer bounds.
    ALuint      rand_gain_var;      // Gain  randomizer bounds.
    ALuint      rand_freq_var;      // Frequency randomizer bounds.

    // Sample reference parameters.
    
    ALuint      sample_index;       // First (or only) sample (buffer) index.
    ALuint      sample_count;       // Sample amount to randomly select from.
}audio_effect_t, *audio_effect_p;

// Audio emitter (aka SoundSource) structure.

typedef struct audio_emitter_s
{
    ALuint      emitter_index;  // Unique emitter index.
    ALuint      sound_index;    // Sound index.
    btScalar    position[3];    // Vector coordinate.    
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
}audio_emitter_t, *audio_emitter_p;


// Main audio source class.

// Sound source is a complex class, each member of which is linked with
// certain in-game entity or sound source, but also a kind of entity by itself.
// Number of simultaneously existing sound sources is fixed, and can't be more than
// MAX_CHANNELS global constant.

class AudioSource
{
public:
    AudioSource();  // Audio source constructor.
   ~AudioSource();	// Audio source destructor.
   
    void Play();    // Make source active and play it.
    void Pause();   // Pause source (leaving it active).
    void Stop();    // Stop and destroy source.
    void Update();  // Update source parameters.

    void SetBuffer(ALint buffer);           // Assign buffer to source.
    void SetLooping(ALboolean is_looping);  // Set looping flag.
    void SetPitch(ALfloat pitch_value);     // Set pitch shift.
    void SetGain(ALfloat gain_value);       // Set gain (volume).
    void SetRange(ALfloat range_value);     // Set max. audible distance.
    
    bool IsActive();            // Check if source is active.
    
    int32_t     emitter_ID;     // Entity of origin. -1 means no entity (hence - empty source).
    uint32_t    emitter_type;   // 0 - ordinary entity, 1 - sound source, 2 - global sound.
    uint32_t    effect_index;   // Effect index. Used to associate effect with entity for R/W flags.
    uint32_t    sample_index;   // OpenAL sample (buffer) index. May be the same for different sources.
    uint32_t    sample_count;   // How many buffers to use, beginning with sample_index.
    bool        is_water;       // Environmental flag; if source/listener flags aren't equal, sample will damp.

    friend int Audio_IsEffectPlaying(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);
    
private:
    bool        active;         // Source gets autostopped and destroyed on next frame, if it's not set.
    ALuint      source_index;   // Source index. Should be unique for each source.
    
    void LinkEmitter();                             // Link source to parent emitter.
    void SetPosition(const ALfloat pos_vector[]);   // Set source position.
    void SetVelocity(const ALfloat vel_vector[]);   // Set source velocity (speed).
};

void Audio_InitGlobals();

int  Audio_Init(const int num_Sources, class VT_Level *tr);
int  Audio_DeInit();

int  Audio_GetFreeSource();
bool Audio_IsInRange(int emitter_type, int emitter_ID, float range, float gain);
int  Audio_IsEffectPlaying(int effect_ID, int entity_ID, int entity_type);

int  Audio_Send(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);	// Send to play effect with given parameters.
int  Audio_Kill(int effect_ID, int entity_type = TR_AUDIO_EMITTER_GLOBAL, int entity_ID = 0);	// If exist, immediately stop and destroy all effects with given parameters.

void Audio_PauseAllSources();	// Used to pause all effects currently playing.
void Audio_ResumeAllSources();	// Used to resume all effects currently paused.
void Audio_UpdateSources();     // Main sound loop.
void Audio_UpdateListenerByCamera(struct camera_s *cam);

int  Audio_LoadALbufferFromWAV_Mem(ALuint buf, uint8_t *sample_pointer, uint32_t sample_size, uint32_t uncomp_sample_size = 0);
int  Audio_LoadALbufferFromWAV_File(ALuint buf, const char *fname);

#endif // AUDIO_H
