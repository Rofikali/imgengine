// src/hot/batch_engine.c 

#include "hot/batch_engine.h"
#include "pipeline/jump_table.h"

void img_batch_process_hot(
    img_ctx_t *ctx,
    img_batch_t *batch,
    img_pipeline_graph_t *pipe)
{
    for (uint32_t i = 0; i < batch->count; i++)
    {
        __builtin_prefetch(batch->buffers[i], 0, 3);
    }

    for (uint32_t j = 0; j < pipe->node_count; j++)
    {
        img_batch_op_fn fn =
            g_batch_jump_table[pipe->nodes[j].op_code];

        if (fn)
            fn(ctx, batch, pipe->nodes[j].params);
    }
}