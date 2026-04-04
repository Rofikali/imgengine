// src/hot/batch_exec.c

#include "hot/batch_exec.h"
#include "pipeline/jump_table.h"

void img_batch_execute(
    img_ctx_t *ctx,
    img_batch_t *batch,
    img_pipeline_desc_t *pipe)
{
    const uint32_t op_count = pipe->count;

    for (uint32_t op = 0; op < op_count; op++)
    {
        uint32_t opcode = pipe->ops[op].op_code;

        img_batch_op_fn fn = g_batch_jump_table[opcode];

        if (!fn)
            continue;

        fn(ctx, batch, pipe->ops[op].params);
    }
}