// src/hot/pipeline_exec.c

// src/hot/pipeline_exec.c

#include "hot/pipeline_exec.h"
#include "pipeline/jump_table.h"

void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_desc_t *pipe,
    img_buffer_t *buf)
{
    const uint32_t count = pipe->count;
    img_op_desc_t *ops = pipe->ops;

    // 🔥 Prefetch first op
    if (count > 0)
        __builtin_prefetch(&ops[0], 0, 3);

    for (uint32_t i = 0; i < count; i++)
    {
        img_op_fn fn = g_jump_table[ops[i].op_code];

        if (fn)
        {
            fn(ctx, buf, ops[i].params);
        }
    }
}

// #include "pipeline/jump_table.h"
// #include "pipeline/pipeline.h"
// #include "observability/profiler.h"
// #include "observability/metrics.h"

// void img_pipeline_execute(
//     img_ctx_t *ctx,
//     img_pipeline_t *p,
//     img_buffer_t *buf)
// {
//     if (!ctx || !p || !buf)
//         return;

//     uint64_t start = img_profiler_now();

//     for (uint32_t i = 0; i < p->op_count; i++)
//     {
//         uint32_t op = p->ops[i];
//         void *params = p->params[i];

//         img_op_fn fn = g_jump_table.ops[op];

//         if (!fn)
//             continue;

//         fn(ctx, buf, params);
//     }

//     uint64_t end = img_profiler_now();

//     img_metrics_inc(&g_metrics.total_requests);
//     img_metrics_observe_latency(end - start);
// }

// #include "hot/pipeline_exec.h"
// #include "pipeline/jump_table.h"
// #include "observability/profiler.h"
// #include "observability/metrics.h"
// // #include "pipeline/pipeline.h"

// void img_pipeline_execute_hot(
//     img_ctx_t *ctx,
//     img_pipeline_graph_t *graph,
//     img_buffer_t *buf)
// {
//     uint64_t start = img_profiler_now();

//     const uint32_t n = graph->node_count;

//     for (uint32_t i = 0; i < n; i++)
//     {
//         img_op_fn op = g_jump_table[graph->nodes[i].op_code];

//         if (__builtin_expect(op != NULL, 1))
//         {
//             op(ctx, buf, graph->nodes[i].params);
//         }
//     }

//     uint64_t elapsed = img_profiler_now() - start;

//     img_metrics_observe_latency(elapsed);
//     img_metrics_inc(&g_metrics.total_requests);
// }