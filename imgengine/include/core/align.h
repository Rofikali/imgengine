// ./include/core/align.h
#ifndef IMGENGINE_CORE_ALIGN_H
#define IMGENGINE_CORE_ALIGN_H

#include <stddef.h>

static inline size_t img_align_pow2(size_t x) {
    size_t p = 1;
    while (p < x)
        p <<= 1;
    return p;
}

#endif /* IMGENGINE_CORE_ALIGN_H */
