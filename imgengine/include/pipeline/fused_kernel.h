
// include/pipeline/fused_kernel.h

#ifndef IMGENGINE_FUSED_KERNEL_H
#define IMGENGINE_FUSED_KERNEL_H

#include "core/context_internal.h"
#include "core/buffer.h"

/*
 * 🔥 FUSED PARAM BLOCK (CACHE-FRIENDLY)
 */
typedef struct
{
    void *blocks[8]; // per-op param blocks
    uint32_t count;
} img_fused_params_t;

/*
 * 🔥 FUSED KERNEL ABI (NO PARAM ARG)
 */
typedef void (*img_fused_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *);

#endif