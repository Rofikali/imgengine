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
