// ./include/security/poision.h

/* security/poison.h */

#ifndef IMGENGINE_POISON_H
#define IMGENGINE_POISON_H

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/asan_interface.h>
#define IMG_POISON_MEMORY(addr, size) ASAN_POISON_MEMORY_REGION(addr, size)
#define IMG_UNPOISON_MEMORY(addr, size) ASAN_UNPOISON_MEMORY_REGION(addr, size)
#endif
#endif

#ifndef IMG_POISON_MEMORY
#define IMG_POISON_MEMORY(addr, size) ((void)0)
#define IMG_UNPOISON_MEMORY(addr, size) ((void)0)
#endif

#endif