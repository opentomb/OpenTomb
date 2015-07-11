/* API declaration export attribute - using like in staticlibrarys */
#define AL_API
#define ALC_API

/* Define to the library version */
#define ALSOFT_VERSION "1.15.1.TRE"

#ifndef M_PI
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#endif

/* Define any available alignment declaration */
#define ALIGN(x)
#ifdef __MINGW32__
#define align(x) aligned(x)
#endif

//#define HAVE_NANOSLEEP
#define HAVE_TIME_H 1
#define HAVE_USLEEP 1 
#define HAVE_UNISTD_H 1

/* Define if we have the SDL audio backend */
#define HAVE_SDL 1

/* Define if we have the C11 aligned_alloc function */
//#define HAVE_ALIGNED_ALLOC

/* Define if we have the posix_memalign function */
//#define HAVE_POSIX_MEMALIGN

/* Define if we have the _aligned_malloc function */
//#define HAVE__ALIGNED_MALLOC

#if 0 && defined(__SSE__)                                                       ///@FIXME: enabling __SSE__ prevents engine to crash!
/* Define if we have SSE CPU extensions */
#define HAVE_SSE

/* Define if we have xmmintrin.h */
#define HAVE_XMMINTRIN_H 1
#endif

/* Define if we have ARM Neon CPU extensions */
//#cmakedefine HAVE_NEON

/* Define if we have the Wave Writer backend */
#define HAVE_WAVE 1

/* Define if we have the lrintf function */
#define HAVE_LRINTF 1

/* Define if we have the strtof function */
#define HAVE_STRTOF 1

/* Define if we have the __int64 type */
#define HAVE___INT64 1

/* Define if we have GCC's destructor attribute 
 CHECK_C_SOURCE_COMPILES("int foo() __attribute__((destructor));
                             int main() {return 0;}" HAVE_GCC_DESTRUCTOR)*/
#define HAVE_GCC_DESTRUCTOR 1

/* Define if we have GCC's format attribute 
 CHECK_C_SOURCE_COMPILES("int foo(const char *str, ...) __attribute__((format(printf, 1, 2)));
                         int main() {return 0;}" HAVE_GCC_FORMAT)*/
//#define HAVE_GCC_FORMAT 1

/* Define if we have stdint.h */
#define HAVE_STDINT_H 1

/* Define if we have pthread_np.h */
//#define HAVE_PTHREAD_NP_H 1

/* Define if we have arm_neon.h */
//#define HAVE_ARM_NEON_H 1

/* Define if we have malloc.h */
#define HAVE_MALLOC_H 1

/* Define if we have cpuid.h */
#define HAVE_CPUID_H 1

/* Define if we have float.h */
#define HAVE_FLOAT_H 1

/* Define if we have fenv.h */
#define HAVE_FENV_H 1

/* Define if we have _controlfp() */
#define HAVE__CONTROLFP 1

/* Define if we have __control87_2() */
//#define HAVE___CONTROL87_2 1

/* Define if we have pthread_setschedparam() */
#define HAVE_PTHREAD_SETSCHEDPARAM 1