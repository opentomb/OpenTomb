/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by authors.
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

#include "../config.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_CPUID_H
#include <cpuid.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include "../alMain.h"

ALuint CPUCapFlags = 0;


void FillCPUCaps(ALuint capfilter)
{
    ALuint caps = 0;

/* FIXME: We really should get this for all available CPUs in case different
 * CPUs have different caps (is that possible on one machine?). */
#if defined(HAVE_CPUID_H) && (defined(__i386__) || defined(__x86_64__) || \
                              defined(_M_IX86) || defined(_M_X64))
    union {
        unsigned int regs[4];
        char str[sizeof(unsigned int[4])];
    } cpuinf[3];

    if(!__get_cpuid(0, &cpuinf[0].regs[0], &cpuinf[0].regs[1], &cpuinf[0].regs[2], &cpuinf[0].regs[3]))
        ERR("Failed to get CPUID\n");
    else
    {
        unsigned int maxfunc = cpuinf[0].regs[0];
        unsigned int maxextfunc = 0;

        if(__get_cpuid(0x80000000, &cpuinf[0].regs[0], &cpuinf[0].regs[1], &cpuinf[0].regs[2], &cpuinf[0].regs[3]))
            maxextfunc = cpuinf[0].regs[0];
        TRACE("Detected max CPUID function: 0x%x (ext. 0x%x)\n", maxfunc, maxextfunc);

        TRACE("Vendor ID: \"%.4s%.4s%.4s\"\n", cpuinf[0].str+4, cpuinf[0].str+12, cpuinf[0].str+8);
        if(maxextfunc >= 0x80000004 &&
           __get_cpuid(0x80000002, &cpuinf[0].regs[0], &cpuinf[0].regs[1], &cpuinf[0].regs[2], &cpuinf[0].regs[3]) &&
           __get_cpuid(0x80000003, &cpuinf[1].regs[0], &cpuinf[1].regs[1], &cpuinf[1].regs[2], &cpuinf[1].regs[3]) &&
           __get_cpuid(0x80000004, &cpuinf[2].regs[0], &cpuinf[2].regs[1], &cpuinf[2].regs[2], &cpuinf[2].regs[3]))
            TRACE("Name: \"%.16s%.16s%.16s\"\n", cpuinf[0].str, cpuinf[1].str, cpuinf[2].str);

        if(maxfunc >= 1 &&
           __get_cpuid(1, &cpuinf[0].regs[0], &cpuinf[0].regs[1], &cpuinf[0].regs[2], &cpuinf[0].regs[3]))
        {
#ifdef bit_SSE
            if((cpuinf[0].regs[3]&bit_SSE))
            {
                caps |= CPU_CAP_SSE;
                if((cpuinf[0].regs[3]&bit_SSE2))
                    caps |= CPU_CAP_SSE2;
            }
#endif
        }
    }
#elif defined(HAVE_WINDOWS_H)
    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    BOOL (WINAPI*IsProcessorFeaturePresent)(DWORD ProcessorFeature);
    IsProcessorFeaturePresent = (BOOL(WINAPI*)(DWORD))GetProcAddress(k32, "IsProcessorFeaturePresent");
    if(!IsProcessorFeaturePresent)
        ERR("IsProcessorFeaturePresent not available; CPU caps not detected\n");
    else
    {
        if(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
        {
            caps |= CPU_CAP_SSE;
            if(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
                caps |= CPU_CAP_SSE2;
        }
    }
#endif
#ifdef HAVE_NEON
    /* Assume Neon support if compiled with it */
    caps |= CPU_CAP_NEON;
#endif

    TRACE("Got caps:%s%s%s%s\n", ((caps&CPU_CAP_SSE)?((capfilter&CPU_CAP_SSE)?" SSE":" (SSE)"):""),
                                 ((caps&CPU_CAP_SSE)?((capfilter&CPU_CAP_SSE2)?" SSE2":" (SSE2)"):""),
                                 ((caps&CPU_CAP_NEON)?((capfilter&CPU_CAP_NEON)?" Neon":" (Neon)"):""),
                                 ((!caps)?" -none-":""));
    CPUCapFlags = caps & capfilter;
}


void *al_malloc(size_t alignment, size_t size)
{
#if defined(HAVE_ALIGNED_ALLOC)
    size = (size+(alignment-1))&~(alignment-1);
    return aligned_alloc(alignment, size);
#elif defined(HAVE_POSIX_MEMALIGN)
    void *ret;
    if(posix_memalign(&ret, alignment, size) == 0)
        return ret;
    return NULL;
#elif defined(HAVE__ALIGNED_MALLOC)
    return _aligned_malloc(size, alignment);
#else
    char *ret = malloc(size+alignment);
    if(ret != NULL)
    {
        *(ret++) = 0x00;
        while(((ALintptrEXT)ret&(alignment-1)) != 0)
            *(ret++) = 0x55;
    }
    return ret;
#endif
}

void *al_calloc(size_t alignment, size_t size)
{
    void *ret = al_malloc(alignment, size);
    if(ret) memset(ret, 0, size);
    return ret;
}

void al_free(void *ptr)
{
#if defined(HAVE_ALIGNED_ALLOC) || defined(HAVE_POSIX_MEMALIGN)
    free(ptr);
#elif defined(HAVE__ALIGNED_MALLOC)
    _aligned_free(ptr);
#else
    if(ptr != NULL)
    {
        char *finder = ptr;
        do {
            --finder;
        } while(*finder == 0x55);
        free(finder);
    }
#endif
}


#if (defined(HAVE___CONTROL87_2) || defined(HAVE__CONTROLFP)) && (defined(__x86_64__) || defined(_M_X64))
/* Win64 doesn't allow us to set the precision control. */
#undef _MCW_PC
#define _MCW_PC 0
#endif

void SetMixerFPUMode(FPUCtl *ctl)
{
#ifdef HAVE_FENV_H
    fegetenv(STATIC_CAST(fenv_t, ctl));
#if defined(__GNUC__) && defined(HAVE_SSE)
    if((CPUCapFlags&CPU_CAP_SSE))
        __asm__ __volatile__("stmxcsr %0" : "=m" (*&ctl->sse_state));
#endif

#ifdef FE_TOWARDZERO
    fesetround(FE_TOWARDZERO);
#endif
#if defined(__GNUC__) && defined(HAVE_SSE)
    if((CPUCapFlags&CPU_CAP_SSE))
    {
        int sseState = ctl->sse_state;
        sseState |= 0x6000; /* set round-to-zero */
        sseState |= 0x8000; /* set flush-to-zero */
        if((CPUCapFlags&CPU_CAP_SSE2))
            sseState |= 0x0040; /* set denormals-are-zero */
        __asm__ __volatile__("ldmxcsr %0" : : "m" (*&sseState));
    }
#endif

#elif defined(HAVE___CONTROL87_2)

    int mode;
    __control87_2(0, 0, &ctl->state, NULL);
    __control87_2(_RC_CHOP|_PC_24, _MCW_RC|_MCW_PC, &mode, NULL);
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
    {
        __control87_2(0, 0, NULL, &ctl->sse_state);
        __control87_2(_RC_CHOP|_DN_FLUSH, _MCW_RC|_MCW_DN, NULL, &mode);
    }
#endif

#elif defined(HAVE__CONTROLFP)

    ctl->state = _controlfp(0, 0);
    (void)_controlfp(_RC_CHOP|_PC_24, _MCW_RC|_MCW_PC);
#endif
}

void RestoreFPUMode(const FPUCtl *ctl)
{
#ifdef HAVE_FENV_H
    fesetenv(STATIC_CAST(fenv_t, ctl));
#if defined(__GNUC__) && defined(HAVE_SSE)
    if((CPUCapFlags&CPU_CAP_SSE))
        __asm__ __volatile__("ldmxcsr %0" : : "m" (*&ctl->sse_state));
#endif

#elif defined(HAVE___CONTROL87_2)

    int mode;
    __control87_2(ctl->state, _MCW_RC|_MCW_PC, &mode, NULL);
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
        __control87_2(ctl->sse_state, _MCW_RC|_MCW_DN, NULL, &mode);
#endif

#elif defined(HAVE__CONTROLFP)

    _controlfp(ctl->state, _MCW_RC|_MCW_PC);
#endif
}


#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#include <sched.h>

void InitializeCriticalSection(pthread_mutex_t *cs)
{
    pthread_mutexattr_t attrib;
    int ret;

    ret = pthread_mutexattr_init(&attrib);
    assert(ret == 0);

    ret = pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE);
#ifdef HAVE_PTHREAD_NP_H
    if(ret != 0)
        ret = pthread_mutexattr_setkind_np(&attrib, PTHREAD_MUTEX_RECURSIVE);
#endif
    assert(ret == 0);
    ret = pthread_mutex_init(cs, &attrib);
    assert(ret == 0);

    pthread_mutexattr_destroy(&attrib);
}
void DeleteCriticalSection(pthread_mutex_t *cs)
{
    int ret;
    ret = pthread_mutex_destroy(cs);
    assert(ret == 0);
}
void EnterCriticalSection(pthread_mutex_t *cs)
{
    int ret;
    ret = pthread_mutex_lock(cs);
    assert(ret == 0);
}
void LeaveCriticalSection(pthread_mutex_t *cs)
{
    int ret;
    ret = pthread_mutex_unlock(cs);
    assert(ret == 0);
}

/* NOTE: This wrapper isn't quite accurate as it returns an ALuint, as opposed
 * to the expected DWORD. Both are defined as unsigned 32-bit types, however.
 * Additionally, Win32 is supposed to measure the time since Windows started,
 * as opposed to the actual time. */
ALuint ALtimeGetTime(void)
{
    struct timeval tv;
    int ret;

    ret = gettimeofday(&tv, NULL);
    assert(ret == 0);

    return tv.tv_usec/1000 + tv.tv_sec*1000;
}

#include <stdlib.h>

void ALSleep(ALuint t)
{
    ALuint seconds = (ALuint)t / 1000;
    ALuint rest = t - seconds * 1000;

    while(seconds > 0)
    {
        usleep (1000000);
        seconds--;
    }
    usleep((unsigned int) (rest * 1000000));
}

void al_print(const char *type, const char *func, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(LogFile, "AL lib: %s %s: ", type, func);
    vfprintf(LogFile, fmt, ap);
    va_end(ap);

    fflush(LogFile);
}


void SetRTPriority(void)
{
    ALboolean failed = AL_FALSE;

    if(RTPrioLevel > 0)
    {
        struct sched_param param;
        /* Use the minimum real-time priority possible for now (on Linux this
         * should be 1 for SCHED_RR) */
        param.sched_priority = sched_get_priority_min(SCHED_RR);
        failed = !!pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    }
    if(failed)
        ERR("Failed to set priority level for thread\n");
}


static void Lock(volatile ALenum *l)
{
    while(ExchangeInt(l, AL_TRUE) == AL_TRUE)
        sched_yield();
}

static void Unlock(volatile ALenum *l)
{
    ExchangeInt(l, AL_FALSE);
}

void RWLockInit(RWLock *lock)
{
    lock->read_count = 0;
    lock->write_count = 0;
    lock->read_lock = AL_FALSE;
    lock->read_entry_lock = AL_FALSE;
    lock->write_lock = AL_FALSE;
}

void ReadLock(RWLock *lock)
{
    Lock(&lock->read_entry_lock);
    Lock(&lock->read_lock);
    if(IncrementRef(&lock->read_count) == 1)
        Lock(&lock->write_lock);
    Unlock(&lock->read_lock);
    Unlock(&lock->read_entry_lock);
}

void ReadUnlock(RWLock *lock)
{
    if(DecrementRef(&lock->read_count) == 0)
        Unlock(&lock->write_lock);
}

void WriteLock(RWLock *lock)
{
    if(IncrementRef(&lock->write_count) == 1)
        Lock(&lock->read_lock);
    Lock(&lock->write_lock);
}

void WriteUnlock(RWLock *lock)
{
    Unlock(&lock->write_lock);
    if(DecrementRef(&lock->write_count) == 0)
        Unlock(&lock->read_lock);
}


void InitUIntMap(UIntMap *map, ALsizei limit)
{
    map->array = NULL;
    map->size = 0;
    map->maxsize = 0;
    map->limit = limit;
    RWLockInit(&map->lock);
}

void ResetUIntMap(UIntMap *map)
{
    WriteLock(&map->lock);
    free(map->array);
    map->array = NULL;
    map->size = 0;
    map->maxsize = 0;
    WriteUnlock(&map->lock);
}

ALenum InsertUIntMapEntry(UIntMap *map, ALuint key, ALvoid *value)
{
    ALsizei pos = 0;

    WriteLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key < key)
            low++;
        pos = low;
    }

    if(pos == map->size || map->array[pos].key != key)
    {
        if(map->size == map->limit)
        {
            WriteUnlock(&map->lock);
            return AL_OUT_OF_MEMORY;
        }

        if(map->size == map->maxsize)
        {
            ALvoid *temp = NULL;
            ALsizei newsize;

            newsize = (map->maxsize ? (map->maxsize<<1) : 4);
            if(newsize >= map->maxsize)
                temp = realloc(map->array, newsize*sizeof(map->array[0]));
            if(!temp)
            {
                WriteUnlock(&map->lock);
                return AL_OUT_OF_MEMORY;
            }
            map->array = temp;
            map->maxsize = newsize;
        }

        if(pos < map->size)
            memmove(&map->array[pos+1], &map->array[pos],
                    (map->size-pos)*sizeof(map->array[0]));
        map->size++;
    }
    map->array[pos].key = key;
    map->array[pos].value = value;
    WriteUnlock(&map->lock);

    return AL_NO_ERROR;
}

ALvoid *RemoveUIntMapKey(UIntMap *map, ALuint key)
{
    ALvoid *ptr = NULL;
    WriteLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key == key)
        {
            ptr = map->array[low].value;
            if(low < map->size-1)
                memmove(&map->array[low], &map->array[low+1],
                        (map->size-1-low)*sizeof(map->array[0]));
            map->size--;
        }
    }
    WriteUnlock(&map->lock);
    return ptr;
}

ALvoid *LookupUIntMapKey(UIntMap *map, ALuint key)
{
    ALvoid *ptr = NULL;
    ReadLock(&map->lock);
    if(map->size > 0)
    {
        ALsizei low = 0;
        ALsizei high = map->size - 1;
        while(low < high)
        {
            ALsizei mid = low + (high-low)/2;
            if(map->array[mid].key < key)
                low = mid + 1;
            else
                high = mid;
        }
        if(map->array[low].key == key)
            ptr = map->array[low].value;
    }
    ReadUnlock(&map->lock);
    return ptr;
}
