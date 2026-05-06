// ./include/memory/align.h
#ifndef IMGENGINE_MEMORY_ALIGN_H
#define IMGENGINE_MEMORY_ALIGN_H

#include <stddef.h>

static inline size_t img_align_up(size_t x, size_t align) {
    return (x + (align - 1)) & ~(align - 1);
}

static inline size_t img_align64(size_t x) { return img_align_up(x, 64); }

#endif /* IMGENGINE_MEMORY_ALIGN_H */
