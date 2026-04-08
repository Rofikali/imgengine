#ifndef IMGENGINE_BOUNDS_CHECK_H
#define IMGENGINE_BOUNDS_CHECK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IMG_LIKELY(x)   __builtin_expect(!!(x), 1)
#define IMG_UNLIKELY(x) __builtin_expect(!!(x), 0)

/*
 * 🔥 HARD BOUNDS CHECK (NO UB, NO OVERFLOW)
 */
static inline bool img_bounds_check(
    const void *ptr,
    size_t size,
    const void *base,
    size_t limit)
{
    uintptr_t p = (uintptr_t)ptr;
    uintptr_t b = (uintptr_t)base;

    if (IMG_UNLIKELY(p < b))
        return false;

    uintptr_t offset = p - b;

    if (IMG_UNLIKELY(offset > limit))
        return false;

    return IMG_LIKELY(size <= (limit - offset));
}

/*
 * 🔥 SAFE MULTIPLY (overflow detection)
 */
static inline bool img_safe_mul(size_t a, size_t b, size_t *out)
{
    if (a == 0 || b == 0)
    {
        *out = 0;
        return true;
    }

    if (a > SIZE_MAX / b)
        return false;

    *out = a * b;
    return true;
}

#endif