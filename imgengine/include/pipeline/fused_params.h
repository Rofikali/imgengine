// pipeline/fused_params.h

#ifndef IMGENGINE_FUSED_PARAMS_H
#define IMGENGINE_FUSED_PARAMS_H

#include <stdint.h>

/*
 * 🔥 SINGLE CACHE-LINE PARAM BLOCK
 */
typedef struct __attribute__((aligned(64)))
{
    uint8_t flags;

    uint16_t brightness;
    uint32_t resize_w;
    uint32_t resize_h;

} img_fused_params_t;

/* FLAGS */
#define FUSED_HAS_GRAY      (1 << 0)
#define FUSED_HAS_BRIGHT    (1 << 1)
#define FUSED_HAS_RESIZE    (1 << 2)

#endif