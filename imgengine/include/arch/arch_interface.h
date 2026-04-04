// /* arch/arch_interface.h */

#ifndef IMGENGINE_ARCH_INTERFACE_H
#define IMGENGINE_ARCH_INTERFACE_H

#include "core/context_internal.h"
#include "api/v1/img_types.h"

// 🔥 UNIFIED KERNEL SIGNATURE
typedef void (*img_kernel_fn)(img_ctx_t *ctx, img_buffer_t *buf, void *params);

// ===== LEGACY (optional keep/remove later) =====
void img_arch_resize_h_avx512(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_avx512(img_ctx_t *, img_buffer_t *, void *);
void img_arch_grayscale_avx512(img_ctx_t *, img_buffer_t *, void *);

// void img_arch_resize_avx512(img_ctx_t *, img_buffer_t *, void *);
// void img_arch_resize_avx2(img_ctx_t *, img_buffer_t *, void *);
// void img_arch_resize_neon(img_ctx_t *, img_buffer_t *, void *);
// void img_arch_resize_scalar(img_ctx_t *, img_buffer_t *, void *);

// ===== GRAYSCALE =====
void img_arch_grayscale_avx2(img_ctx_t *, img_buffer_t *, void *);
void img_arch_grayscale_scalar(img_ctx_t *, img_buffer_t *, void *);

// ===== 🔥 SEPARABLE RESIZE (FINAL PATH) =====

// AVX2
void img_arch_resize_h_avx2(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_avx2(img_ctx_t *, img_buffer_t *, void *);

// NEON
void img_arch_resize_h_neon(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_neon(img_ctx_t *, img_buffer_t *, void *);

// SCALAR
void img_arch_resize_h_scalar(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_scalar(img_ctx_t *, img_buffer_t *, void *);

#endif