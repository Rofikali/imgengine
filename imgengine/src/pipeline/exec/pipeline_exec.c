// #include "pipeline/exec/pipeline_exec.h"

// void img_pipeline_execute_hot(
//     img_ctx_t *ctx,
//     const img_pipeline_compiled_t *pipeline,
//     img_buffer_t *buf)
// {
//     uint32_t n = pipeline->count;

//     for (uint32_t i = 0; i < n; i++)
//     {
//         pipeline->steps[i].fn(
//             ctx,
//             buf,
//             pipeline->steps[i].params);
//     }
// }

#include "pipeline/exec/pipeline_exec.h"
#include "pipeline/dispatch/jump_table.h"

#include "observability/binlog_fast.h"
#include "observability/profiler.h"

void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_runtime_t *pipe,
    img_buffer_t *buf)
{
    if (!ctx || !pipe || !buf)
        return;

    uint64_t start = img_profiler_now();

    img_runtime_op_t *ops = pipe->ops;
    uint32_t n = pipe->count;

    /*
     * 🔥 HOT LOOP (NO BRANCHES INSIDE)
     */
    for (uint32_t i = 0; i < n; i++)
    {
        uint32_t op = ops[i].op_code;
        void *params = ops[i].params;

        g_jump_table[op](ctx, buf, params);
    }

    uint64_t end = img_profiler_now();

    /*
     * 🔥 ZERO-OVERHEAD LOGGING
     */
    IMG_LOG_LATENCY(end - start, n, 0);
}

// future ultra-optimization
switch (pipeline->count)
{
case 3:
    pipeline->steps[0].fn(ctx, buf, pipeline->steps[0].params);
    pipeline->steps[1].fn(ctx, buf, pipeline->steps[1].params);
    pipeline->steps[2].fn(ctx, buf, pipeline->steps[2].params);
    break;
}