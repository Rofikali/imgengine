// src/hot/pipeline_exec.c

#include "hot/pipeline_exec.h"
#include "pipeline/jump_table.h"
#include "observability/profiler.h"
#include "observability/metrics.h"
// #include "pipeline/pipeline.h"

void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_graph_t *graph,
    img_buffer_t *buf)
{
    uint64_t start = img_profiler_now();

    const uint32_t n = graph->node_count;

    for (uint32_t i = 0; i < n; i++)
    {
        img_op_fn op = g_jump_table[graph->nodes[i].op_code];

        if (__builtin_expect(op != NULL, 1))
        {
            op(ctx, buf, graph->nodes[i].params);
        }
    }

    uint64_t elapsed = img_profiler_now() - start;

    img_metrics_observe_latency(elapsed);
    img_metrics_inc(&g_metrics.total_requests);
}