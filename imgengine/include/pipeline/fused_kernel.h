// include/pipeline/fused_kernel.h

#ifndef IMGENGINE_FUSED_KERNEL_H
#define IMGENGINE_FUSED_KERNEL_H

#include "core/context_internal.h"
#include "core/buffer.h"
#include "pipeline/fused_params.h"

/*
 * 🔥 FUSED KERNEL ABI (STRICT)
 */
typedef void (*img_fused_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *,
    img_fused_params_t *);

#endif