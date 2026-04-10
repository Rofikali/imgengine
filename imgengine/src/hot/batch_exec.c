
// src/hot/batch_exec.c

#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline.h"

#include "hot/batch_exec.h"
#include "pipeline/jump_table.h"

#define UNROLL 2

void img_batch_execute(
    img_ctx_t *__restrict ctx,
    img_batch_t *__restrict batch,
    const img_pipeline_runtime_t *__restrict pipe)
{
    const uint32_t count = pipe->count;

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t opcode = pipe->ops[i].op_code;

        img_batch_kernel_fn fn = g_batch_jump_table[opcode];

        if (!fn)
            continue;

        fn(ctx, batch, pipe->ops[i].params);
    }
}
