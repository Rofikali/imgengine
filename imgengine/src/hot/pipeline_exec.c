// src/hot/pipeline_exec.c

// #include "hot/pipeline_exec.h"
// #include "pipeline/pipeline_compiled.h"

// void img_pipeline_execute_hot(
//     img_ctx_t *ctx,
//     img_pipeline_compiled_t *pipe,
//     img_buffer_t *buf)
// {
//     uint32_t count = pipe->count;

//     img_op_fn *ops = pipe->ops;
//     void **params = pipe->params;

//     // 🔥 Prefetch
//     __builtin_prefetch(ops, 0, 3);

//     for (uint32_t i = 0; i < count; i++)
//     {
//         ops[i](ctx, buf, params[i]);
//     }
// }

#include "hot/pipeline_exec.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline.h"
// #include "pipeline/pipeline_compiled.h"

void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_runtime_t *pipe,
    // img_pipeline_compiled_t *pipe,
    img_buffer_t *buf)
{
    uint32_t count = pipe->count;
    img_op_desc_t *ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++)
    {
        __builtin_prefetch(&ops[i + 1], 0, 1);

        img_op_fn fn = g_jump_table[ops[i].op_code];

        // 🔥 NO BRANCH MISSPREDICTION (predict taken)
        if (__builtin_expect(fn != NULL, 1))
        {
            fn(ctx, buf, ops[i].params);
        }
    }
}

// #include "hot/pipeline_exec.h"
// #include "pipeline/jump_table.h"

// void img_pipeline_execute_hot(
//     img_ctx_t *ctx,
//     img_pipeline_desc_t *pipe,
//     img_buffer_t *buf)
// {
//     const uint32_t count = pipe->count;
//     img_op_desc_t *ops = pipe->ops;

//     // 🔥 Prefetch first op
//     if (count > 0)
//         __builtin_prefetch(&ops[0], 0, 3);

//     for (uint32_t i = 0; i < count; i++)
//     {
//         img_op_fn fn = g_jump_table[ops[i].op_code];

//         if (fn)
//         {
//             fn(ctx, buf, ops[i].params);
//         }
//     }
// }
