
/* include/core/arch_interface.h */

#ifndef IMGENGINE_ARCH_INTERFACE_H
#define IMGENGINE_ARCH_INTERFACE_H

#include <stdint.h>

/*
 * 🔥 Forward declarations (L8 rule)
 * Avoid pulling full headers into interface layer
 */
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

/*
 * 🔥 Kernel function signature
 * All SIMD/scalar kernels must follow this
 */
typedef void (*img_kernel_fn)(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    void *params);

/*
 * 🔥 Hardware-specific implementations
 * (resolved at runtime via dispatcher)
 */

// x86_64
void img_kernel_resize_avx512(img_ctx_t *, img_buffer_t *, void *);
void img_kernel_resize_avx2(img_ctx_t *, img_buffer_t *, void *);

// ARM
void img_kernel_resize_neon(img_ctx_t *, img_buffer_t *, void *);

#endif