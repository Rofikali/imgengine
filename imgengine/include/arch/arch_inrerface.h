#ifndef IMGENGINE_ARCH_INTERFACE_H
#define IMGENGINE_ARCH_INTERFACE_H

#include "core/context/ctx_internal.h"
#include "core/buffer/buffer.h"

/*
 * 🔥 SINGLE OP SIGNATURE (STRICT ABI)
 */
typedef void (*img_kernel_fn)(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    void *params);

#endif