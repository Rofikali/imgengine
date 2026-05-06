// ./src/pipeline/pipeline_fuse_execute_batch.c
#define _GNU_SOURCE

#include "pipeline/pipeline_fuse_internal.h"

#include <stddef.h>

void img_pipeline_execute_fused_batch(img_ctx_t *ctx, img_pipeline_fused_t *pipe,
                                      img_batch_t *batch) {
    if (__builtin_expect(!pipe || !batch || batch->count == 0, 0))
        return;

    ctx->fused_params = &g_fused_params;

    if (__builtin_expect(pipe->fn_batch != NULL, 1)) {
        pipe->fn_batch(ctx, batch, &g_fused_params);
        return;
    }

    for (uint32_t i = 0; i < batch->count; i++) {
        if (__builtin_expect(batch->buffers[i] != NULL, 1))
            pipe->fn(ctx, batch->buffers[i]);
    }
}
