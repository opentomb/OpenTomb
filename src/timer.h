#ifndef TIMER_H
#define TIMER_H
    
    #define BILLION (1E9)

    #ifdef    __cplusplus
    extern "C" {
    #endif
	
    int _clock_gettime(int dummy, struct timespec *ct);
    int _gettimeofday(struct timeval *p, void *tz);
	
    #ifdef    __cplusplus
    }
    #endif
    
#endif