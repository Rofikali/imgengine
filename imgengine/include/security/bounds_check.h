
// secrity/bounds_check.h

#ifndef IMGENGINE_BOUNDS_CHECK_H
#define IMGENGINE_BOUNDS_CHECK_H

#define IMG_LIKELY(x) __builtin_expect(!!(x), 1)
#define IMG_UNLIKELY(x) __builtin_expect(!!(x), 0)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // <--- ADD THIS for size_t

// 🔥 SAFE (NO OVERFLOW)
static inline bool img_bounds_check(
    const void *ptr,
    size_t size,
    const void *base,
    size_t limit)
{
    uintptr_t p = (uintptr_t)ptr;
    uintptr_t b = (uintptr_t)base;

    if (p < b)
        return false;

    uintptr_t offset = p - b;

    // 🔥 prevent overflow: offset + size <= limit
    if (offset > limit)
        return false;

    return size <= (limit - offset);
}

#endif