// src/hot/batch_exec.c

#include "hot/batch_exec.h"
#include "pipeline/jump_table.h"

#include <stddef.h>
#include "hot/pipeline_threaded.h"
#include "pipeline/pipeline.h"

void img_batch_execute(
    img_ctx_t *__restrict ctx,
    img_batch_t *__restrict batch,
    const img_pipeline_runtime_t *__restrict pipe)
{
    const img_op_desc_t *ops = pipe->ops;

    for (uint32_t i = 0; i < pipe->count; i++)
    {
        uint32_t opcode = ops[i].op_code;

        img_batch_kernel_fn fn = g_batch_jump_table[opcode];

        if (__builtin_expect(fn != NULL, 1))
        {
            fn(ctx, batch, ops[i].params); // ✅ FIXED ABI
        }
    }
}