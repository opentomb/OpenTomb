#ifndef _AL_AUXEFFECTSLOT_H_
#define _AL_AUXEFFECTSLOT_H_

#include "alMain.h"
#include "alEffect.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALeffectStateFactory ALeffectStateFactory;

typedef struct ALeffectState ALeffectState;
typedef struct ALeffectslot ALeffectslot;

struct ALeffectStateVtable {
    ALvoid (*const Destruct)(ALeffectState *state);
    ALboolean (*const DeviceUpdate)(ALeffectState *state, ALCdevice *device);
    ALvoid (*const Update)(ALeffectState *state, ALCdevice *device, const ALeffectslot *slot);
    ALvoid (*const Process)(ALeffectState *state, ALuint samplesToDo, const ALfloat *__restrict__ samplesIn, ALfloat (*__restrict__ samplesOut)[BUFFERSIZE]);

    void (*const Delete)(ALeffectState *state);
};

struct ALeffectState {
    const struct ALeffectStateVtable *vtbl;
};

#define DEFINE_ALEFFECTSTATE_VTABLE(T)                                        \
static ALvoid T##_ALeffectState_Destruct(ALeffectState *state)                \
{ T##_Destruct(STATIC_UPCAST(T, ALeffectState, state)); }                     \
static ALboolean T##_ALeffectState_DeviceUpdate(ALeffectState *state, ALCdevice *device) \
{ return T##_DeviceUpdate(STATIC_UPCAST(T, ALeffectState, state), device); }             \
static ALvoid T##_ALeffectState_Update(ALeffectState *state, ALCdevice *device, const ALeffectslot *slot) \
{ T##_Update(STATIC_UPCAST(T, ALeffectState, state), device, slot); }                                     \
static ALvoid T##_ALeffectState_Process(ALeffectState *state, ALuint samplesToDo, const ALfloat *__restrict__ samplesIn, ALfloat (*__restrict__ samplesOut)[BUFFERSIZE]) \
{ T##_Process(STATIC_UPCAST(T, ALeffectState, state), samplesToDo, samplesIn, samplesOut); }                                                                     \
static ALvoid T##_ALeffectState_Delete(ALeffectState *state)                  \
{ T##_Delete(STATIC_UPCAST(T, ALeffectState, state)); }                       \
                                                                              \
static const struct ALeffectStateVtable T##_ALeffectState_vtable = {          \
    T##_ALeffectState_Destruct,                                               \
    T##_ALeffectState_DeviceUpdate,                                           \
    T##_ALeffectState_Update,                                                 \
    T##_ALeffectState_Process,                                                \
    T##_ALeffectState_Delete,                                                 \
}


struct ALeffectStateFactoryVtable {
    ALeffectState *(*const create)(ALeffectStateFactory *factory);
};

struct ALeffectStateFactory {
    const struct ALeffectStateFactoryVtable *vtbl;
};

#define DEFINE_ALEFFECTSTATEFACTORY_VTABLE(T)                                 \
static ALeffectState* T##_ALeffectStateFactory_create(ALeffectStateFactory *factory) \
{ return T##_create(STATIC_UPCAST(T, ALeffectStateFactory, factory)); }              \
                                                                              \
static const struct ALeffectStateFactoryVtable T##_ALeffectStateFactory_vtable = { \
    T##_ALeffectStateFactory_create,                                          \
}


struct ALeffectslot
{
    ALenum EffectType;
    ALeffectProps EffectProps;

    volatile ALfloat   Gain;
    volatile ALboolean AuxSendAuto;

    volatile ALenum NeedsUpdate;
    ALeffectState *EffectState;

    ALIGN(16) ALfloat WetBuffer[1][BUFFERSIZE];

    ALfloat ClickRemoval[1];
    ALfloat PendingClicks[1];

    RefCount ref;

    /* Self ID */
    ALuint id;
};


ALenum InitEffectSlot(ALeffectslot *slot);
ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context);


ALeffectStateFactory *ALnullStateFactory_getFactory(void);
ALeffectStateFactory *ALreverbStateFactory_getFactory(void);
ALeffectStateFactory *ALchorusStateFactory_getFactory(void);
ALeffectStateFactory *ALdistortionStateFactory_getFactory(void);
ALeffectStateFactory *ALechoStateFactory_getFactory(void);
ALeffectStateFactory *ALequalizerStateFactory_getFactory(void);
ALeffectStateFactory *ALflangerStateFactory_getFactory(void);
ALeffectStateFactory *ALmodulatorStateFactory_getFactory(void);

ALeffectStateFactory *ALdedicatedStateFactory_getFactory(void);


ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect);

void InitEffectFactoryMap(void);
void DeinitEffectFactoryMap(void);

#ifdef __cplusplus
}
#endif

#endif
