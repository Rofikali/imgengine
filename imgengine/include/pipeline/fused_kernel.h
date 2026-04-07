// include/pipeline/fused_kernel.h

#ifndef IMGENGINE_FUSED_KERNEL_H
#define IMGENGINE_FUSED_KERNEL_H

#include "core/context_internal.h"
#include "core/buffer.h"

typedef void (*img_fused_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *);

#endif