// src/pipeline/batch_exec.c

#include "pipeline/batch_exec.h"

extern img_fused_kernel_fn img_get_fused_kernel();

void img_batch_execute(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *pipeline)
{
    (void)pipeline;

    img_fused_kernel_fn fn = img_get_fused_kernel();

    if (fn)
    {
        fn(ctx, batch, NULL); // 🔥 ONE LOOP ONLY
    }
}
