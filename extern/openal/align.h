#ifndef AL_ALIGN_H
#define AL_ALIGN_H

#ifdef HAVE_STDALIGN_H
#include <stdalign.h>
#endif

#ifndef alignas
#ifdef HAVE_C11_ALIGNAS
#define alignas _Alignas
#elif defined(IN_IDE_PARSER)
/* KDevelop has problems with our align macro, so just use nothing for parsing. */
#define alignas(x)
#else
/* NOTE: Our custom ALIGN macro can't take a type name like alignas can. For
 * maximum compatibility, only provide constant integer values to alignas. */
#define alignas(_x) __attribute__((packed, aligned(_x)))
#endif
#endif

#endif /* AL_ALIGN_H */
