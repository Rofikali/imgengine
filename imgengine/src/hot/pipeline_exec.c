// src/hot/pipeline_exec.c

#include "hot/pipeline_exec.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline.h"
#include "pipeline/fused_kernel.h"
#include <stddef.h>

// 🔥 prefetch distance (tunable)
#define PREFETCH_DISTANCE 2

void img_pipeline_execute_hot(
    img_ctx_t *__restrict ctx,
    const img_pipeline_runtime_t *__restrict pipe,
    img_buffer_t *__restrict buf)
{
    uint32_t count = pipe->count;

    // 🔥 cache pointers locally (avoid repeated deref)
    const img_op_desc_t *__restrict ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++)
    {
        // 🔥 PREFETCH FUTURE OPS
        if (i + PREFETCH_DISTANCE < count)
        {
            __builtin_prefetch(&ops[i + PREFETCH_DISTANCE], 0, 1);
        }

        const img_op_desc_t *__restrict op = &ops[i];

        // 🔥 cache function pointer (avoid repeated table lookup)
        img_op_fn fn = g_jump_table[op->op_code];

        // 🔥 branch prediction: assume valid
        if (__builtin_expect(fn != NULL, 1))
        {
            fn(ctx, buf, op->params);
        }
    }
}

void img_pipeline_execute_fused(
    img_ctx_t *ctx,
    img_fused_kernel_fn fn,
    img_fused_params_t *params,
    img_buffer_t *buf)
{
    ctx->fused_params = params;

    /*
     * 🔥 DIRECT CALL (NO BRANCH)
     */
    fn(ctx, buf);
}