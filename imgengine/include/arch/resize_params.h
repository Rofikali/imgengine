// ./include/arch/resize_params.h
// include/arch/resize_params.h

#ifndef IMGENGINE_ARCH_RESIZE_PARAMS_H
#define IMGENGINE_ARCH_RESIZE_PARAMS_H

#include <stdint.h>

/*
 * Forward declaration of img_buffer_t.
 *
 * resize_params_t uses img_buffer_t* for src.
 * We forward-declare the struct here so arch/ does not
 * need to include core/buffer.h (which would add a dep).
 *
 * All arch resize kernels already include core/buffer.h
 * for their own buf parameter — so the full definition
 * is visible at every call site. The forward decl here
 * just satisfies the compiler for the pointer field.
 */
struct img_buffer;

typedef struct
{
    struct img_buffer *src; /* source buffer — full type at call sites */

    uint32_t target_w;
    uint32_t target_h;

    uint32_t scale_x; /* fixed-point 16.16 */
    uint32_t scale_y;

    uint32_t *x_index; /* precomputed column map (arena-allocated) */
    uint32_t *y_index; /* precomputed row map    (arena-allocated) */
    uint16_t *x_weight; /* low 16 bits of the 16.16 fractional part */
    uint16_t *y_weight;

} resize_params_t;

#endif /* IMGENGINE_ARCH_RESIZE_PARAMS_H */

// // include/arch/resize_params.h  (NEW FILE)

// // ================================================================
// // STEP 2 OF 6: Move resize_params_t to arch/
// // Fixes: arch depending on plugins (8 violations)
// // arch/ must not know about plugins/ — wrong direction.
// // ================================================================

// #ifndef IMGENGINE_ARCH_RESIZE_PARAMS_H
// #define IMGENGINE_ARCH_RESIZE_PARAMS_H

// #include <stdint.h>

// /*
//  * resize_params_t — parameter block for all resize kernels.
//  *
//  * Lives in arch/ because it describes the hardware kernel contract.
//  * Previously in plugins/plugin_resize.h — that made arch/ depend
//  * upward on plugins/, which is a layer violation.
//  *
//  * All arch resize kernels (AVX2, scalar, AVX-512) use this struct.
//  * Pipeline and plugins reference it through forward declaration only.
//  */
// typedef struct
// {
//     /* source buffer pointer (read-only in kernel) */
//     void *src; /* img_buffer_t* — forward-declared to avoid circular dep */

//     /* target dimensions */
//     uint32_t target_w;
//     uint32_t target_h;

//     /* fixed-point scale factors (16.16) */
//     uint32_t scale_x;
//     uint32_t scale_y;

//     /* precomputed index lookup tables (arena-allocated, cold path) */
//     uint32_t *x_index;
//     uint32_t *y_index;

// } resize_params_t;

// #endif /* IMGENGINE_ARCH_RESIZE_PARAMS_H */
