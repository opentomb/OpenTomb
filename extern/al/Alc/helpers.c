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
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#endif

#ifdef HAVE_CPUID_H
#include <cpuid.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#include "../alMain.h"
#include "../alu.h"
#include "../atomic.h"
#include "../uintmap.h"
#include "../Alc/vector.h"
#include "../Alc/alstring.h"
#include "../Alc/compat.h"
#include "../threads.h"


extern inline ALuint NextPowerOf2(ALuint value);
extern inline ALint fastf2i(ALfloat f);
extern inline ALuint fastf2u(ALfloat f);


ALuint CPUCapFlags = 0;


void FillCPUCaps(ALuint capfilter)
{
    ALuint caps = 0;

/* FIXME: We really should get this for all available CPUs in case different
 * CPUs have different caps (is that possible on one machine?). */
#if defined(HAVE_GCC_GET_CPUID) && (defined(__i386__) || defined(__x86_64__) || \
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
            if((cpuinf[0].regs[3]&(1<<25)))
            {
                caps |= CPU_CAP_SSE;
                if((cpuinf[0].regs[3]&(1<<26)))
                {
                    caps |= CPU_CAP_SSE2;
                    if((cpuinf[0].regs[2]&(1<<19)))
                        caps |= CPU_CAP_SSE4_1;
                }
            }
        }
    }
#elif defined(HAVE_CPUID_INTRINSIC) && (defined(__i386__) || defined(__x86_64__) || \
                                        defined(_M_IX86) || defined(_M_X64))
    union {
        int regs[4];
        char str[sizeof(int[4])];
    } cpuinf[3];

    (__cpuid)(cpuinf[0].regs, 0);
    if(cpuinf[0].regs[0] == 0)
        ERR("Failed to get CPUID\n");
    else
    {
        unsigned int maxfunc = cpuinf[0].regs[0];
        unsigned int maxextfunc;

        (__cpuid)(cpuinf[0].regs, 0x80000000);
        maxextfunc = cpuinf[0].regs[0];

        TRACE("Detected max CPUID function: 0x%x (ext. 0x%x)\n", maxfunc, maxextfunc);

        TRACE("Vendor ID: \"%.4s%.4s%.4s\"\n", cpuinf[0].str+4, cpuinf[0].str+12, cpuinf[0].str+8);
        if(maxextfunc >= 0x80000004)
        {
            (__cpuid)(cpuinf[0].regs, 0x80000002);
            (__cpuid)(cpuinf[1].regs, 0x80000003);
            (__cpuid)(cpuinf[2].regs, 0x80000004);
            TRACE("Name: \"%.16s%.16s%.16s\"\n", cpuinf[0].str, cpuinf[1].str, cpuinf[2].str);
        }

        if(maxfunc >= 1)
        {
            (__cpuid)(cpuinf[0].regs, 1);
            if((cpuinf[0].regs[3]&(1<<25)))
            {
                caps |= CPU_CAP_SSE;
                if((cpuinf[0].regs[3]&(1<<26)))
                {
                    caps |= CPU_CAP_SSE2;
                    if((cpuinf[0].regs[2]&(1<<19)))
                        caps |= CPU_CAP_SSE4_1;
                }
            }
        }
    }
#else
    /* Assume support for whatever's supported if we can't check for it */
#if defined(HAVE_SSE4_1)
#warning "Assuming SSE 4.1 run-time support!"
    capfilter |= CPU_CAP_SSE | CPU_CAP_SSE2 | CPU_CAP_SSE4_1;
#elif defined(HAVE_SSE2)
#warning "Assuming SSE 2 run-time support!"
    capfilter |= CPU_CAP_SSE | CPU_CAP_SSE2;
#elif defined(HAVE_SSE)
#warning "Assuming SSE run-time support!"
    capfilter |= CPU_CAP_SSE;
#endif
#endif
#ifdef HAVE_NEON
    /* Assume Neon support if compiled with it */
    caps |= CPU_CAP_NEON;
#endif

    TRACE("Extensions:%s%s%s%s%s\n",
        ((capfilter&CPU_CAP_SSE)    ? ((caps&CPU_CAP_SSE)    ? " +SSE"    : " -SSE")    : ""),
        ((capfilter&CPU_CAP_SSE2)   ? ((caps&CPU_CAP_SSE2)   ? " +SSE2"   : " -SSE2")   : ""),
        ((capfilter&CPU_CAP_SSE4_1) ? ((caps&CPU_CAP_SSE4_1) ? " +SSE4.1" : " -SSE4.1") : ""),
        ((capfilter&CPU_CAP_NEON)   ? ((caps&CPU_CAP_NEON)   ? " +Neon"   : " -Neon")   : ""),
        ((!capfilter) ? " -none-" : "")
    );
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
    __control87_2(_RC_CHOP, _MCW_RC, &mode, NULL);
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
    {
        __control87_2(0, 0, NULL, &ctl->sse_state);
        __control87_2(_RC_CHOP|_DN_FLUSH, _MCW_RC|_MCW_DN, NULL, &mode);
    }
#endif

#elif defined(HAVE__CONTROLFP)

    ctl->state = _controlfp(0, 0);
    (void)_controlfp(_RC_CHOP, _MCW_RC);
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
    __control87_2(ctl->state, _MCW_RC, &mode, NULL);
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
        __control87_2(ctl->sse_state, _MCW_RC|_MCW_DN, NULL, &mode);
#endif

#elif defined(HAVE__CONTROLFP)

    _controlfp(ctl->state, _MCW_RC);
#endif
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

FILE *OpenDataFile(const char *fname, const char *subdir)
{
    char buffer[PATH_MAX] = "";
    const char *str, *next;
    FILE *f;

    if(fname[0] == '/')
    {
        if((f=al_fopen(fname, "rb")) != NULL)
        {
            TRACE("Opened %s\n", fname);
            return f;
        }
        WARN("Could not open %s\n", fname);
        return NULL;
    }

    if((f=al_fopen(fname, "rb")) != NULL)
    {
        TRACE("Opened %s\n", fname);
        return f;
    }
    WARN("Could not open %s\n", fname);

    if((str=getenv("XDG_DATA_HOME")) != NULL && str[0] != '\0')
        snprintf(buffer, sizeof(buffer), "%s/%s/%s", str, subdir, fname);
    else if((str=getenv("HOME")) != NULL && str[0] != '\0')
        snprintf(buffer, sizeof(buffer), "%s/.local/share/%s/%s", str, subdir, fname);
    if(buffer[0])
    {
        if((f=al_fopen(buffer, "rb")) != NULL)
        {
            TRACE("Opened %s\n", buffer);
            return f;
        }
        WARN("Could not open %s\n", buffer);
    }

    if((str=getenv("XDG_DATA_DIRS")) == NULL || str[0] == '\0')
        str = "/usr/local/share/:/usr/share/";

    next = str;
    while((str=next) != NULL && str[0] != '\0')
    {
        size_t len;
        next = strchr(str, ':');

        if(!next)
            len = strlen(str);
        else
        {
            len = next - str;
            next++;
        }

        if(len > sizeof(buffer)-1)
            len = sizeof(buffer)-1;
        strncpy(buffer, str, len);
        buffer[len] = '\0';
        snprintf(buffer+len, sizeof(buffer)-len, "/%s/%s", subdir, fname);

        if((f=al_fopen(buffer, "rb")) != NULL)
        {
            TRACE("Opened %s\n", buffer);
            return f;
        }
        WARN("Could not open %s\n", buffer);
    }

    return NULL;
}


void SetRTPriority(void)
{
    ALboolean failed = AL_FALSE;

#if defined(HAVE_PTHREAD_SETSCHEDPARAM) && !defined(__OpenBSD__)
    if(RTPrioLevel > 0)
    {
        struct sched_param param;
        /* Use the minimum real-time priority possible for now (on Linux this
         * should be 1 for SCHED_RR) */
        param.sched_priority = sched_get_priority_min(SCHED_RR);
        failed = !!pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    }
#else
    /* Real-time priority not available */
    failed = (RTPrioLevel>0);
#endif
    if(failed)
        ERR("Failed to set priority level for thread\n");
}


ALboolean vector_reserve(char *ptr, size_t base_size, size_t obj_size, ALsizei obj_count, ALboolean exact)
{
    vector_ *vecptr = (vector_*)ptr;
    if(obj_count < 0)
        return AL_FALSE;
    if((*vecptr ? (*vecptr)->Capacity : 0) < obj_count)
    {
        ALsizei old_size = (*vecptr ? (*vecptr)->Size : 0);
        void *temp;

        /* Use the next power-of-2 size if we don't need to allocate the exact
         * amount. This is preferred when regularly increasing the vector since
         * it means fewer reallocations. Though it means it also wastes some
         * memory. */
        if(exact == AL_FALSE)
        {
            obj_count = NextPowerOf2((ALuint)obj_count);
            if(obj_count < 0) return AL_FALSE;
        }

        /* Need to be explicit with the caller type's base size, because it
         * could have extra padding before the start of the array (that is,
         * sizeof(*vector_) may not equal base_size). */
        temp = realloc(*vecptr, base_size + obj_size*obj_count);
        if(temp == NULL) return AL_FALSE;

        *vecptr = temp;
        (*vecptr)->Capacity = obj_count;
        (*vecptr)->Size = old_size;
    }
    return AL_TRUE;
}

ALboolean vector_resize(char *ptr, size_t base_size, size_t obj_size, ALsizei obj_count)
{
    vector_ *vecptr = (vector_*)ptr;
    if(obj_count < 0)
        return AL_FALSE;
    if(*vecptr || obj_count > 0)
    {
        if(!vector_reserve((char*)vecptr, base_size, obj_size, obj_count, AL_TRUE))
            return AL_FALSE;
        (*vecptr)->Size = obj_count;
    }
    return AL_TRUE;
}

ALboolean vector_insert(char *ptr, size_t base_size, size_t obj_size, void *ins_pos, const void *datstart, const void *datend)
{
    vector_ *vecptr = (vector_*)ptr;
    if(datstart != datend)
    {
        ptrdiff_t ins_elem = (*vecptr ? ((char*)ins_pos - ((char*)(*vecptr) + base_size)) :
                                        ((char*)ins_pos - (char*)NULL)) /
                             obj_size;
        ptrdiff_t numins = ((const char*)datend - (const char*)datstart) / obj_size;

        assert(numins > 0);
        if(INT_MAX-VECTOR_SIZE(*vecptr) <= numins ||
           !vector_reserve((char*)vecptr, base_size, obj_size, VECTOR_SIZE(*vecptr)+numins, AL_TRUE))
            return AL_FALSE;

        /* NOTE: ins_pos may have been invalidated if *vecptr moved. Use ins_elem instead. */
        if(ins_elem < (*vecptr)->Size)
        {
            memmove((char*)(*vecptr) + base_size + ((ins_elem+numins)*obj_size),
                    (char*)(*vecptr) + base_size + ((ins_elem       )*obj_size),
                    ((*vecptr)->Size-ins_elem)*obj_size);
        }
        memcpy((char*)(*vecptr) + base_size + (ins_elem*obj_size),
               datstart, numins*obj_size);
        (*vecptr)->Size += (ALsizei)numins;
    }
    return AL_TRUE;
}


extern inline void al_string_deinit(al_string *str);
extern inline ALsizei al_string_length(const_al_string str);
extern inline ALboolean al_string_empty(const_al_string str);
extern inline const al_string_char_type *al_string_get_cstr(const_al_string str);

void al_string_clear(al_string *str)
{
    /* Reserve one more character than the total size of the string. This is to
     * ensure we have space to add a null terminator in the string data so it
     * can be used as a C-style string. */
    VECTOR_RESERVE(*str, 1);
    VECTOR_RESIZE(*str, 0);
    *VECTOR_ITER_END(*str) = 0;
}

static inline int al_string_compare(const al_string_char_type *str1, ALsizei str1len,
                                    const al_string_char_type *str2, ALsizei str2len)
{
    ALsizei complen = mini(str1len, str2len);
    int ret = memcmp(str1, str2, complen);
    if(ret == 0)
    {
        if(str1len > str2len) return  1;
        if(str1len < str2len) return -1;
    }
    return ret;
}
int al_string_cmp(const_al_string str1, const_al_string str2)
{
    return al_string_compare(&VECTOR_FRONT(str1), al_string_length(str1),
                             &VECTOR_FRONT(str2), al_string_length(str2));
}
int al_string_cmp_cstr(const_al_string str1, const al_string_char_type *str2)
{
    return al_string_compare(&VECTOR_FRONT(str1), al_string_length(str1),
                             str2, (ALsizei)strlen(str2));
}

void al_string_copy(al_string *str, const_al_string from)
{
    ALsizei len = VECTOR_SIZE(from);
    VECTOR_RESERVE(*str, len+1);
    VECTOR_RESIZE(*str, 0);
    VECTOR_INSERT(*str, VECTOR_ITER_END(*str), VECTOR_ITER_BEGIN(from), VECTOR_ITER_BEGIN(from)+len);
    *VECTOR_ITER_END(*str) = 0;
}

void al_string_copy_cstr(al_string *str, const al_string_char_type *from)
{
    size_t len = strlen(from);
    VECTOR_RESERVE(*str, len+1);
    VECTOR_RESIZE(*str, 0);
    VECTOR_INSERT(*str, VECTOR_ITER_END(*str), from, from+len);
    *VECTOR_ITER_END(*str) = 0;
}

void al_string_append_char(al_string *str, const al_string_char_type c)
{
    VECTOR_RESERVE(*str, al_string_length(*str)+2);
    VECTOR_PUSH_BACK(*str, c);
    *VECTOR_ITER_END(*str) = 0;
}

void al_string_append_cstr(al_string *str, const al_string_char_type *from)
{
    size_t len = strlen(from);
    if(len != 0)
    {
        VECTOR_RESERVE(*str, al_string_length(*str)+len+1);
        VECTOR_INSERT(*str, VECTOR_ITER_END(*str), from, from+len);
        *VECTOR_ITER_END(*str) = 0;
    }
}

void al_string_append_range(al_string *str, const al_string_char_type *from, const al_string_char_type *to)
{
    if(to != from)
    {
        VECTOR_RESERVE(*str, al_string_length(*str)+(to-from)+1);
        VECTOR_INSERT(*str, VECTOR_ITER_END(*str), from, to);
        *VECTOR_ITER_END(*str) = 0;
    }
}
