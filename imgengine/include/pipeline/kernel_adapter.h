// include/pipeline/kernel_adapter.h

#ifndef IMGENGINE_KERNEL_ADAPTER_H
#define IMGENGINE_KERNEL_ADAPTER_H

#include "core/context_internal.h"
#include "core/buffer.h"
#include "hot/batch_exec.h"

/*
 * 🔥 SINGLE ABI (GLOBAL TRUTH)
 */
typedef void (*img_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *,
    void *);

typedef void (*img_batch_kernel_fn)(
    img_ctx_t *,
    img_batch_t *,
    void *);

#endif