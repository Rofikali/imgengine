// ./src/pipeline/canvas_fill_background.c
#include "pipeline/canvas_internal.h"

#include <string.h>

#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(undefined_behavior_sanitizer)
#define IMGENGINE_ASM_FILL_DISABLED 1
#endif
#endif

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_UNDEFINED__)
#define IMGENGINE_ASM_FILL_DISABLED 1
#endif

static inline void img_fill_bytes_kernel(uint8_t *dst, uint8_t value, size_t len) {
    if (!dst || len == 0)
        return;

#if defined(__x86_64__) && !defined(IMGENGINE_ASM_FILL_DISABLED)
    /*
     * Leaf fill kernel:
     * - caller owns buffer sizing and validation
     * - this helper only writes a known-safe byte span
     */
    __asm__ volatile("rep stosb" : "+D"(dst), "+c"(len) : "a"(value) : "memory");
#else
    memset(dst, value, len);
#endif
}

void img_canvas_fill_background(img_canvas_t *canvas, const img_job_t *job) {
    size_t required = (size_t)canvas->buf.width * canvas->buf.height * 3;
    uint8_t *mem = canvas->buf.data;

    if (job->bg_r == job->bg_g && job->bg_g == job->bg_b) {
        img_fill_bytes_kernel(mem, job->bg_r, required);
    } else {
        for (size_t i = 0; i < required; i += 3) {
            mem[i] = job->bg_r;
            mem[i + 1] = job->bg_g;
            mem[i + 2] = job->bg_b;
        }
    }

    canvas->bg_signature =
        ((uint32_t)job->bg_r << 16) | ((uint32_t)job->bg_g << 8) | (uint32_t)job->bg_b;
    canvas->initialized = 1;
    canvas->cache_owned = 0;
}
