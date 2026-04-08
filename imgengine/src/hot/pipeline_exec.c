// src/hot/pipeline_exec.c
#include "hot/pipeline_exec.h"
#include "hot/pipeline_threaded.h"

#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline.h"
#include "pipeline/fused_kernel.h"

#include "core/opcodes.h"
#include <stddef.h>

// 🔥 prefetch distance
#define PREFETCH_DISTANCE 2

void img_pipeline_execute_hot(
    img_ctx_t *__restrict ctx,
    const img_pipeline_runtime_t *__restrict pipe,
    img_buffer_t *__restrict buf)
{
    const uint32_t count = pipe->count;
    const img_op_desc_t *__restrict ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++)
    {
        if (i + PREFETCH_DISTANCE < count)
        {
            __builtin_prefetch(&ops[i + PREFETCH_DISTANCE], 0, 1);
        }

        const img_op_desc_t *__restrict op = &ops[i];

        /*
         * 🔥 ALWAYS KERNEL ABI (3 args)
         */
        img_kernel_fn fn = g_jump_table[op->op_code];

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
    /*
     * 🔥 PARAM HOISTING (ONE WRITE)
     */
    ctx->fused_params = params;

    /*
     * 🔥 DIRECT CALL (ZERO OVERHEAD)
     */
    fn(ctx, buf);
}

void img_pipeline_execute_threaded(
    img_ctx_t *ctx,
    const img_threaded_program_t *prog,
    img_buffer_t *buf)
{
    uint32_t ip = 0;

#define DISPATCH() goto *dispatch_table[prog->opcodes[ip++]]

    /*
     * 🔥 LOCAL DISPATCH TABLE (LEGAL)
     */
    static void *dispatch_table[IMG_MAX_OPS] = {0};

    /*
     * 🔥 INIT ONCE (branchless after warmup)
     */
    if (__builtin_expect(dispatch_table[0] == NULL, 0))
    {
        for (uint32_t i = 0; i < IMG_MAX_OPS; i++)
            dispatch_table[i] = &&op_default;

        dispatch_table[OP_RESIZE] = &&op_resize;
        // add more ops here
    }

    DISPATCH();

    /*
     * ================= OPCODES =================
     */

op_resize:
{
    void *params = prog->params[ip - 1];

    img_kernel_fn fn = g_jump_table[OP_RESIZE];
    fn(ctx, buf, params);

    if (ip < prog->count)
        DISPATCH();
    return;
}

op_default:
{
    if (ip < prog->count)
        DISPATCH();
    return;
}
}