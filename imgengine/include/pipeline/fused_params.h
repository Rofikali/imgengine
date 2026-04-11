// ./include/pipeline/fused_params.h

// include/pipeline/fused_params.h

#ifndef IMGENGINE_FUSED_PARAMS_H
#define IMGENGINE_FUSED_PARAMS_H

#include <stdint.h>

/*
 * FUSED PARAM BLOCK
 *
 * Holds all op parameters for a fused kernel invocation.
 * Single allocation, passed by pointer — zero copies in hot path.
 *
 * 64-byte aligned: fits exactly in one cache line.
 * Padding added to prevent false sharing on adjacent allocations.
 *
 * RULES:
 *   - All fields populated during pipeline_fuse() (cold path)
 *   - Read-only during kernel execution (hot path)
 *   - Never embed pointers — must be memcpy-safe
 */
typedef struct __attribute__((aligned(64))) img_fused_params
{
    /* op presence flags */
    uint8_t has_grayscale;
    uint8_t has_brightness;
    uint8_t has_resize;
    uint8_t _pad0;

    /* brightness */
    uint16_t brightness_value;
    uint16_t _pad1;

    /* resize target dimensions */
    uint32_t new_w;
    uint32_t new_h;

    /* explicit padding to 64 bytes */
    uint8_t _pad2[44];

} img_fused_params_t;

/* compile-time layout assertion */
_Static_assert(sizeof(img_fused_params_t) == 64,
               "img_fused_params_t must be exactly 64 bytes (one cache line)");

#endif /* IMGENGINE_FUSED_PARAMS_H */

// // include/pipeline/fused_params.h

// #ifndef IMGENGINE_FUSED_PARAMS_H
// #define IMGENGINE_FUSED_PARAMS_H

// #include <stdint.h>

// typedef struct
// {
//     uint8_t has_grayscale;
//     uint8_t has_brightness;
//     uint8_t has_resize;

//     uint16_t brightness_value;
//     uint32_t new_w;
//     uint32_t new_h;

// } img_fused_params_t;

// #endif