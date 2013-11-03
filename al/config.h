/* API declaration export attribute */
#define AL_API
#define ALC_API

/* Define to the library version */
#define ALSOFT_VERSION "1.15.1"
#define restrict __restrict__

/* Define any available alignment declaration */
#define ALIGN(x)
#ifdef __MINGW32__
#define align(x) //aligned(x)
#endif

//#define HAVE_NANOSLEEP
#define HAVE_TIME_H 1
#define HAVE_USLEEP 1 
#define HAVE_UNISTD_H 1

/* Define if we have the Windows Multimedia backend */
#define HAVE_SDL 1
#define HAVE_WINMM 0

/* Define if we have the C11 aligned_alloc function */
//#cmakedefine HAVE_ALIGNED_ALLOC

/* Define if we have the posix_memalign function */
//#cmakedefine HAVE_POSIX_MEMALIGN

/* Define if we have the _aligned_malloc function */
//#cmakedefine HAVE__ALIGNED_MALLOC

/* Define if we have SSE CPU extensions */
//#cmakedefine HAVE_SSE

/* Define if we have ARM Neon CPU extensions */
//#cmakedefine HAVE_NEON

/* Define if we have the Wave Writer backend */
#define HAVE_WAVE 1

/* Define if we have the stat function */
#define HAVE_STAT 1
#define HAVE__STAT 1

/* Define if we have the lrintf function */
#define HAVE_LRINTF 1

/* Define if we have the strtof function */
#define HAVE_STRTOF 1

/* Define if we have the __int64 type */
#define HAVE___INT64 1

/* Define to the size of a long int type */
#define SIZEOF_LONG (sizeof(long int))

/* Define to the size of a long long int type */
#define SIZEOF_LONG_LONG (sizeof(long long int))

/* Define if we have GCC's destructor attribute 
 CHECK_C_SOURCE_COMPILES("int foo() __attribute__((destructor));
                             int main() {return 0;}" HAVE_GCC_DESTRUCTOR)*/
#define HAVE_GCC_DESTRUCTOR 1

/* Define if we have GCC's format attribute 
 CHECK_C_SOURCE_COMPILES("int foo(const char *str, ...) __attribute__((format(printf, 1, 2)));
                         int main() {return 0;}" HAVE_GCC_FORMAT)*/
//#cmakedefine HAVE_GCC_FORMAT

/* Define if we have stdint.h */
#define HAVE_STDINT_H 1

/* Define if we have windows.h */
#define HAVE_WINDOWS_H 1

/* Define if we have dlfcn.h */
//#cmakedefine HAVE_DLFCN_H

/* Define if we have pthread_np.h */
//#cmakedefine HAVE_PTHREAD_NP_H

/* Define if we have xmmintrin.h */
#define HAVE_XMMINTRIN_H 1

/* Define if we have arm_neon.h */
//#cmakedefine HAVE_ARM_NEON_H

/* Define if we have malloc.h */
#define HAVE_MALLOC_H

/* Define if we have cpuid.h */
#define HAVE_CPUID_H 1

/* Define if we have guiddef.h */
//#cmakedefine HAVE_GUIDDEF_H

/* Define if we have initguid.h */
#define HAVE_INITGUID_H 1

/* Define if we have ieeefp.h */
//#cmakedefine HAVE_IEEEFP_H

/* Define if we have float.h */
#define HAVE_FLOAT_H

/* Define if we have fenv.h */
#define HAVE_FENV_H

/* Define if we have _controlfp() */
#define HAVE__CONTROLFP 1

/* Define if we have __control87_2() */
//#cmakedefine HAVE___CONTROL87_2

/* Define if we have pthread_setschedparam() */
#define HAVE_PTHREAD_SETSCHEDPARAM 1