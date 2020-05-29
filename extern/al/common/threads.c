/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
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

#include <sys/time.h>
#include <unistd.h>

#ifndef _POSIX_SOURCE
#define __USE_POSIX199309  (1)   // make posix GCC workable
#define __USE_XOPEN2K
#define __USE_XOPEN2K8
#endif
#include <pthread.h>
#ifdef _TIMESPEC_DEFINED         // make MinGW workable
#include <pthread_time.h>
#define HAVE_PTHREAD_SETNAME_NP
#endif
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../uintmap.h"
#include "../threads.h"

extern inline althrd_t althrd_current(void);
extern inline int althrd_equal(althrd_t thr0, althrd_t thr1);
extern inline void althrd_exit(int res);
extern inline void althrd_yield(void);

extern inline int almtx_lock(almtx_t *mtx);
extern inline int almtx_unlock(almtx_t *mtx);
extern inline int almtx_trylock(almtx_t *mtx);

extern inline void *altss_get(altss_t tss_id);
extern inline int altss_set(altss_t tss_id, void *val);


#ifndef UNUSED
#if defined(__cplusplus)
#define UNUSED(x)
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif
#endif


#define THREAD_STACK_SIZE (1*1024*1024) /* 1MB */


extern inline int althrd_sleep(const struct timespec *ts, struct timespec *rem);
extern inline void alcall_once(alonce_flag *once, void (*callback)(void));


void althrd_setname(althrd_t thr, const char *name)
{
#if defined(HAVE_PTHREAD_SETNAME_NP)
#if defined(__GNUC__)
    pthread_setname_np(thr, name);
#elif defined(__APPLE__)
    if(althrd_equal(thr, althrd_current())
        pthread_setname_np(name);
#endif
#elif defined(HAVE_PTHREAD_SET_NAME_NP)
    pthread_set_name_np(thr, name);
#else
    (void)thr;
    (void)name;
#endif
}


typedef struct thread_cntr {
    althrd_start_t func;
    void *arg;
} thread_cntr;

static void *althrd_starter(void *arg)
{
    thread_cntr cntr;
    memcpy(&cntr, arg, sizeof(cntr));
    free(arg);

    return (void*)(intptr_t)((*cntr.func)(cntr.arg));
}


int althrd_create(althrd_t *thr, althrd_start_t func, void *arg)
{
    thread_cntr *cntr;
    pthread_attr_t attr;

    cntr = malloc(sizeof(*cntr));
    if(!cntr) return althrd_nomem;

    if(pthread_attr_init(&attr) != 0)
    {
        free(cntr);
        return althrd_error;
    }
    if(pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE) != 0)
    {
        pthread_attr_destroy(&attr);
        free(cntr);
        return althrd_error;
    }

    cntr->func = func;
    cntr->arg = arg;
    if(pthread_create(thr, &attr, althrd_starter, cntr) != 0)
    {
        pthread_attr_destroy(&attr);
        free(cntr);
        return althrd_error;
    }
    pthread_attr_destroy(&attr);

    return althrd_success;
}

int althrd_detach(althrd_t thr)
{
    if(pthread_detach(thr) != 0)
        return althrd_error;
    return althrd_success;
}

int althrd_join(althrd_t thr, int *res)
{
    void *code;

    if(pthread_join(thr, &code) != 0)
        return althrd_error;
    if(res != NULL)
        *res = (int)(intptr_t)code;
    return althrd_success;
}


int almtx_init(almtx_t *mtx, int type)
{
    int ret;

    if(!mtx) return althrd_error;
    if((type&~(almtx_recursive|almtx_timed)) != 0)
        return althrd_error;

    type &= ~almtx_timed;
    if(type == almtx_plain)
        ret = pthread_mutex_init(mtx, NULL);
    else
    {
        pthread_mutexattr_t attr;

        ret = pthread_mutexattr_init(&attr);
        if(ret) return althrd_error;

        if(type == almtx_recursive)
        {
            ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#ifdef HAVE_PTHREAD_MUTEXATTR_SETKIND_NP
            if(ret != 0)
                ret = pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
        }
        else
            ret = 1;
        if(ret == 0)
            ret = pthread_mutex_init(mtx, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    return ret ? althrd_error : althrd_success;
}

void almtx_destroy(almtx_t *mtx)
{
    pthread_mutex_destroy(mtx);
}

int almtx_timedlock(almtx_t *mtx, const struct timespec *ts)
{
    int ret;

    ret = pthread_mutex_timedlock(mtx, ts);
    switch(ret)
    {
        case 0: return althrd_success;
        case ETIMEDOUT: return althrd_timedout;
        case EBUSY: return althrd_busy;
    }
    return althrd_error;
}

int alcnd_init(alcnd_t *cond)
{
    if(pthread_cond_init(cond, NULL) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_signal(alcnd_t *cond)
{
    if(pthread_cond_signal(cond) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_broadcast(alcnd_t *cond)
{
    if(pthread_cond_broadcast(cond) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_wait(alcnd_t *cond, almtx_t *mtx)
{
    if(pthread_cond_wait(cond, mtx) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_timedwait(alcnd_t *cond, almtx_t *mtx, const struct timespec *time_point)
{
    if(pthread_cond_timedwait(cond, mtx, time_point) == 0)
        return althrd_success;
    return althrd_error;
}

void alcnd_destroy(alcnd_t *cond)
{
    pthread_cond_destroy(cond);
}


int altss_create(altss_t *tss_id, altss_dtor_t callback)
{
    if(pthread_key_create(tss_id, callback) != 0)
        return althrd_error;
    return althrd_success;
}

void altss_delete(altss_t tss_id)
{
    pthread_key_delete(tss_id);
}


int altimespec_get(struct timespec *ts, int base)
{
    if(base == AL_TIME_UTC)
    {
        int ret = clock_gettime(CLOCK_REALTIME, ts);
        if(ret == 0) return base;
    }

    return 0;
}


void al_nssleep(time_t sec, long nsec)
{
    struct timespec ts, rem;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;

    while(althrd_sleep(&ts, &rem) == -1)
        ts = rem;
}
