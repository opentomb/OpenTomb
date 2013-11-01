/**
 * OpenAL cross platform audio library
 * Copyright (C) 2013 by Mike Gorchak
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "../../config.h"

#include <math.h>
#include <stdlib.h>

#include "alMain.h"
#include "alFilter.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"


typedef struct ALchorusStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALchorusStateFactory;

static ALchorusStateFactory ChorusFactory;


typedef struct ALchorusState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat *SampleBufferLeft;
    ALfloat *SampleBufferRight;
    ALuint BufferLength;
    ALint offset;
    ALfloat lfo_coeff;
    ALint lfo_disp;

    /* Gains for left and right sides */
    ALfloat Gain[2][MaxChannels];

    /* effect parameters */
    ALint waveform;
    ALint delay;
    ALfloat depth;
    ALfloat feedback;
} ALchorusState;

static ALvoid ALchorusState_Destruct(ALchorusState *state)
{
    free(state->SampleBufferLeft);
    state->SampleBufferLeft = NULL;

    free(state->SampleBufferRight);
    state->SampleBufferRight = NULL;
}

static ALboolean ALchorusState_DeviceUpdate(ALchorusState *state, ALCdevice *Device)
{
    ALuint maxlen;
    ALuint it;

    maxlen = fastf2u(AL_CHORUS_MAX_DELAY * 3.0f * Device->Frequency) + 1;
    maxlen = NextPowerOf2(maxlen);

    if(maxlen != state->BufferLength)
    {
        void *temp;

        temp = realloc(state->SampleBufferLeft, maxlen * sizeof(ALfloat));
        if(!temp) return AL_FALSE;
        state->SampleBufferLeft = temp;

        temp = realloc(state->SampleBufferRight, maxlen * sizeof(ALfloat));
        if(!temp) return AL_FALSE;
        state->SampleBufferRight = temp;

        state->BufferLength = maxlen;
    }

    for(it = 0;it < state->BufferLength;it++)
    {
        state->SampleBufferLeft[it] = 0.0f;
        state->SampleBufferRight[it] = 0.0f;
    }

    return AL_TRUE;
}

static ALvoid ALchorusState_Update(ALchorusState *state, ALCdevice *Device, const ALeffectslot *Slot)
{
    ALfloat frequency = (ALfloat)Device->Frequency;
    ALfloat rate;
    ALint phase;
    ALuint it;

    for (it = 0; it < MaxChannels; it++)
    {
        state->Gain[0][it] = 0.0f;
        state->Gain[1][it] = 0.0f;
    }

    state->waveform = Slot->EffectProps.Chorus.Waveform;
    state->depth = Slot->EffectProps.Chorus.Depth;
    state->feedback = Slot->EffectProps.Chorus.Feedback;
    state->delay = fastf2i(Slot->EffectProps.Chorus.Delay * frequency);

    /* Gains for left and right sides */
    ComputeAngleGains(Device, atan2f(-1.0f, 0.0f), 0.0f, Slot->Gain, state->Gain[0]);
    ComputeAngleGains(Device, atan2f(+1.0f, 0.0f), 0.0f, Slot->Gain, state->Gain[1]);

    phase = Slot->EffectProps.Chorus.Phase;
    rate = Slot->EffectProps.Chorus.Rate;

    /* Calculate LFO coefficient */
    switch (state->waveform)
    {
        case AL_CHORUS_WAVEFORM_TRIANGLE:
             if(rate == 0.0f)
                 state->lfo_coeff = 0.0f;
             else
                 state->lfo_coeff = 1.0f / (frequency / rate);
             break;
        case AL_CHORUS_WAVEFORM_SINUSOID:
             if(rate == 0.0f)
                 state->lfo_coeff = 0.0f;
             else
                 state->lfo_coeff = M_PI*2.0f / (frequency / rate);
             break;
    }

    /* Calculate lfo phase displacement */
    if(phase == 0 || rate == 0.0f)
        state->lfo_disp = 0;
    else
        state->lfo_disp = fastf2i(frequency / rate / (360.0f/phase));
}

static __inline void Triangle(ALint *delay_left, ALint *delay_right, ALint offset, const ALchorusState *state)
{
    ALfloat lfo_value;

    lfo_value = 2.0f - fabsf(2.0f - fmodf(state->lfo_coeff*offset*4.0f, 4.0f));
    lfo_value *= state->depth * state->delay;
    *delay_left = fastf2i(lfo_value) + state->delay;

    lfo_value = 2.0f - fabsf(2.0f - fmodf(state->lfo_coeff *
                                          (offset+state->lfo_disp)*4.0f,
                                          4.0f));
    lfo_value *= state->depth * state->delay;
    *delay_right = fastf2i(lfo_value) + state->delay;
}

static __inline void Sinusoid(ALint *delay_left, ALint *delay_right, ALint offset, const ALchorusState *state)
{
    ALfloat lfo_value;

    lfo_value = 1.0f + sinf(fmodf(state->lfo_coeff*offset, 2.0f*M_PI));
    lfo_value *= state->depth * state->delay;
    *delay_left = fastf2i(lfo_value) + state->delay;

    lfo_value = 1.0f + sinf(fmodf(state->lfo_coeff*(offset+state->lfo_disp),
                                  2.0f*M_PI));
    lfo_value *= state->depth * state->delay;
    *delay_right = fastf2i(lfo_value) + state->delay;
}

#define DECL_TEMPLATE(func)                                                    \
static void Process##func(ALchorusState *state, ALuint SamplesToDo,            \
                          const ALfloat *__restrict__ SamplesIn,                   \
                          ALfloat (*__restrict__ SamplesOut)[BUFFERSIZE])          \
{                                                                              \
    const ALint mask = state->BufferLength-1;                                  \
    ALint offset = state->offset;                                              \
    ALuint it, kt;                                                             \
    ALuint base;                                                               \
                                                                               \
    for(base = 0;base < SamplesToDo;)                                          \
    {                                                                          \
        ALfloat temps[64][2];                                                  \
        ALuint td = minu(SamplesToDo-base, 64);                                \
                                                                               \
        for(it = 0;it < td;it++,offset++)                                      \
        {                                                                      \
            ALint delay_left, delay_right;                                     \
            (func)(&delay_left, &delay_right, offset, state);                  \
                                                                               \
            temps[it][0] = state->SampleBufferLeft[(offset-delay_left)&mask];  \
            state->SampleBufferLeft[offset&mask] = (temps[it][0] +             \
                                                    SamplesIn[it+base]) *      \
                                                   state->feedback;            \
                                                                               \
            temps[it][1] = state->SampleBufferRight[(offset-delay_right)&mask];\
            state->SampleBufferRight[offset&mask] = (temps[it][1] +            \
                                                     SamplesIn[it+base]) *     \
                                                    state->feedback;           \
        }                                                                      \
                                                                               \
        for(kt = 0;kt < MaxChannels;kt++)                                      \
        {                                                                      \
            ALfloat gain = state->Gain[0][kt];                                 \
            if(gain > 0.00001f)                                                \
            {                                                                  \
                for(it = 0;it < td;it++)                                       \
                    SamplesOut[kt][it+base] += temps[it][0] * gain;            \
            }                                                                  \
                                                                               \
            gain = state->Gain[1][kt];                                         \
            if(gain > 0.00001f)                                                \
            {                                                                  \
                for(it = 0;it < td;it++)                                       \
                    SamplesOut[kt][it+base] += temps[it][1] * gain;            \
            }                                                                  \
        }                                                                      \
                                                                               \
        base += td;                                                            \
    }                                                                          \
                                                                               \
    state->offset = offset;                                                    \
}

DECL_TEMPLATE(Triangle)
DECL_TEMPLATE(Sinusoid)

#undef DECL_TEMPLATE

static ALvoid ALchorusState_Process(ALchorusState *state, ALuint SamplesToDo, const ALfloat *__restrict__ SamplesIn, ALfloat (*__restrict__ SamplesOut)[BUFFERSIZE])
{
    if(state->waveform == AL_CHORUS_WAVEFORM_TRIANGLE)
        ProcessTriangle(state, SamplesToDo, SamplesIn, SamplesOut);
    else if(state->waveform == AL_CHORUS_WAVEFORM_SINUSOID)
        ProcessSinusoid(state, SamplesToDo, SamplesIn, SamplesOut);
}

static void ALchorusState_Delete(ALchorusState *state)
{
    free(state);
}

DEFINE_ALEFFECTSTATE_VTABLE(ALchorusState);


static ALeffectState *ALchorusStateFactory_create(ALchorusStateFactory *factory)
{
    ALchorusState *state;
    (void)factory;

    state = malloc(sizeof(*state));
    if(!state) return NULL;
    SET_VTABLE2(ALchorusState, ALeffectState, state);

    state->BufferLength = 0;
    state->SampleBufferLeft = NULL;
    state->SampleBufferRight = NULL;
    state->offset = 0;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALchorusStateFactory);


static void init_chorus_factory(void)
{
    SET_VTABLE2(ALchorusStateFactory, ALeffectStateFactory, &ChorusFactory);
}

ALeffectStateFactory *ALchorusStateFactory_getFactory(void)
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, init_chorus_factory);
    return STATIC_CAST(ALeffectStateFactory, &ChorusFactory);
}


void ALchorus_SetParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_WAVEFORM:
            if(!(val >= AL_CHORUS_MIN_WAVEFORM && val <= AL_CHORUS_MAX_WAVEFORM))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Waveform = val;
            break;

        case AL_CHORUS_PHASE:
            if(!(val >= AL_CHORUS_MIN_PHASE && val <= AL_CHORUS_MAX_PHASE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Phase = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_SetParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALchorus_SetParami(effect, context, param, vals[0]);
}
void ALchorus_SetParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_RATE:
            if(!(val >= AL_CHORUS_MIN_RATE && val <= AL_CHORUS_MAX_RATE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Rate = val;
            break;

        case AL_CHORUS_DEPTH:
            if(!(val >= AL_CHORUS_MIN_DEPTH && val <= AL_CHORUS_MAX_DEPTH))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Depth = val;
            break;

        case AL_CHORUS_FEEDBACK:
            if(!(val >= AL_CHORUS_MIN_FEEDBACK && val <= AL_CHORUS_MAX_FEEDBACK))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Feedback = val;
            break;

        case AL_CHORUS_DELAY:
            if(!(val >= AL_CHORUS_MIN_DELAY && val <= AL_CHORUS_MAX_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Delay = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_SetParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALchorus_SetParamf(effect, context, param, vals[0]);
}

void ALchorus_GetParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_WAVEFORM:
            *val = props->Chorus.Waveform;
            break;

        case AL_CHORUS_PHASE:
            *val = props->Chorus.Phase;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_GetParamiv(ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALchorus_GetParami(effect, context, param, vals);
}
void ALchorus_GetParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_RATE:
            *val = props->Chorus.Rate;
            break;

        case AL_CHORUS_DEPTH:
            *val = props->Chorus.Depth;
            break;

        case AL_CHORUS_FEEDBACK:
            *val = props->Chorus.Feedback;
            break;

        case AL_CHORUS_DELAY:
            *val = props->Chorus.Delay;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_GetParamfv(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    ALchorus_GetParamf(effect, context, param, vals);
}

DEFINE_ALEFFECT_VTABLE(ALchorus);
