// ./src/pipeline/engine.c

// ./src/pipeline/engine.c

#include "pipeline/engine.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"

void img_pipeline_execute(
    img_ctx_t *ctx,
    img_pipeline_desc_t *pipe,
    img_buffer_t *buf)
{
    const uint32_t count = pipe->count;

    img_op_desc_t *ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t op_code = ops[i].op_code;

        img_kernel_fn fn = g_jump_table[op_code];

        if (fn)
        {
            fn(ctx, buf, ops[i].params);
        }
    }
}
