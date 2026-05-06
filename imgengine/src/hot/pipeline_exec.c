// ./src/hot/pipeline_exec.c

// src/hot/pipeline_exec.c

#include "hot/pipeline_exec.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline.h"
#include "pipeline/fused_kernel.h"
#include "pipeline/fused_params.h"
#include <stddef.h>

/*
 * Prefetch distance: tuned for 64-byte cache lines.
 * 2 ops ahead = ~128 bytes prefetched.
 * Adjust if pipeline ops are larger.
 */
#define PREFETCH_DISTANCE 2

/*
 * img_pipeline_execute_hot()
 *
 * Hot path: sequential single-image execution.
 * Iterates compiled op list → jump table dispatch → kernel call.
 *
 * CONTRACT:
 *   - pipe->ops must be cache-aligned (enforced by pipeline_types.h)
 *   - g_jump_table populated before first call (img_jump_table_init)
 *   - No allocation, no locks, no branches in steady state
 */
void img_pipeline_execute_hot(img_ctx_t *__restrict ctx,
                              const img_pipeline_runtime_t *__restrict pipe,
                              img_buffer_t *__restrict buf) {
    const uint32_t count = pipe->count;
    const img_op_desc_t *__restrict ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++) {

        /* prefetch next op descriptor into L1 */
        if (i + PREFETCH_DISTANCE < count)
            __builtin_prefetch(&ops[i + PREFETCH_DISTANCE], 0, 1);

        const img_op_desc_t *__restrict op = &ops[i];

        /*
         * KERNEL ABI: always 3 args (ctx, buf, params)
         * g_jump_table[opcode] → img_kernel_fn
         */
        img_kernel_fn fn = g_jump_table[op->op_code];

        if (__builtin_expect(fn != NULL, 1))
            fn(ctx, buf, op->params);
    }
}

void img_pipeline_execute_compiled_hot(img_ctx_t *__restrict ctx,
                                       const img_pipeline_compiled_t *__restrict pipe,
                                       img_buffer_t *__restrict buf) {
    const uint32_t count = pipe->count;

    for (uint32_t i = 0; i < count; i++) {
        if (i + PREFETCH_DISTANCE < count)
            __builtin_prefetch(&pipe->ops[i + PREFETCH_DISTANCE], 0, 1);

        img_kernel_fn fn = pipe->ops[i];

        if (__builtin_expect(fn != NULL, 1))
            fn(ctx, buf, pipe->params[i]);
    }
}

/*
 * img_pipeline_execute_fused()
 *
 * Hot path: single-image fused execution.
 *
 * Uses img_single_kernel_fn — NOT img_fused_kernel_fn.
 * Single-image ABI: (img_ctx_t*, img_buffer_t*) — no batch, no params arg.
 * Params are passed via ctx->fused_params (set once before call).
 *
 * Why params via ctx and not as arg?
 *   - Eliminates one pointer arg from the hot call
 *   - Params are cache-resident in ctx (64-byte aligned)
 *   - Consistent with kernel-style execution context pattern
 *
 * Caller responsibility:
 *   - fn must not be NULL (check before calling this)
 *   - ctx->fused_params must point to valid img_fused_params_t
 */
void img_pipeline_execute_fused(img_ctx_t *ctx, img_single_kernel_fn fn, img_fused_params_t *params,
                                img_buffer_t *buf) {
    /*
     * Hoist params into ctx once.
     * All subsequent reads by fn are from ctx->fused_params.
     * No per-pixel param lookup.
     */
    ctx->fused_params = params;

    /*
     * Direct call — zero overhead.
     * fn is pre-resolved (cold path), never a runtime lookup here.
     */
    fn(ctx, buf);
}
