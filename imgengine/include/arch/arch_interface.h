// ./include/arch/arch_interface.h

#ifndef IMGENGINE_ARCH_INTERFACE_H
#define IMGENGINE_ARCH_INTERFACE_H

#include "core/context_internal.h"
#include "core/buffer.h"

// 🔥 UNIFIED KERNEL SIGNATURE (STRICT ABI)
typedef void (*img_kernel_fn)(img_ctx_t *ctx, img_buffer_t *buf, void *params);

// ================= GRAYSCALE =================
void img_arch_grayscale_avx2(img_ctx_t *, img_buffer_t *, void *);
void img_arch_grayscale_scalar(img_ctx_t *, img_buffer_t *, void *);

// ================= RESIZE (SEPARABLE) =================

// AVX2
void img_arch_resize_h_avx2(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_avx2(img_ctx_t *, img_buffer_t *, void *);

// SCALAR
void img_arch_resize_h_scalar(img_ctx_t *, img_buffer_t *, void *);
void img_arch_resize_v_scalar(img_ctx_t *, img_buffer_t *, void *);

#endif