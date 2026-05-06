// ./src/pipeline/engine.c

// src/pipeline/engine.c  (UPDATE)
// Replace exec_router include with pipeline/dispatch.h

#include "pipeline/engine.h"
#include "pipeline/dispatch.h" /* replaces runtime/exec_router.h */
#include "pipeline/pipeline_types.h"

void img_pipeline_execute(img_ctx_t *ctx, img_pipeline_desc_t *pipe, img_buffer_t *buf) {
    const uint32_t count = pipe->count;
    img_op_desc_t *ops = pipe->ops;

    for (uint32_t i = 0; i < count; i++)
        g_dispatch_fn(ctx, ops[i].op_code, buf, ops[i].params);
}

// // src/pipeline/engine.c

// // ================================================================
// // FILE 4: src/pipeline/engine.c
// // Replace direct jump table call with exec_router dispatch.
// // This wires exec_router into the actual execution path.
// // ================================================================

// #include "pipeline/engine.h"
// #include "pipeline/jump_table.h"
// #include "pipeline/pipeline_types.h"
// #include "runtime/exec_router.h"

// /*
//  * img_pipeline_execute()
//  *
//  * Sequential execution of a compiled pipeline.
//  * Each op is dispatched via exec_router — local SIMD or remote RPC.
//  *
//  * Previously: direct g_jump_table[op_code] call.
//  * Now: img_exec_dispatch() — same cost for local ops (one extra
//  * switch statement, branch-predicted to IMG_EXEC_LOCAL always),
//  * with the ability to route heavy ops to remote workers.
//  */
// void img_pipeline_execute(
//     img_ctx_t *ctx,
//     img_pipeline_desc_t *pipe,
//     img_buffer_t *buf)
// {
//     const uint32_t count = pipe->count;
//     img_op_desc_t *ops = pipe->ops;

//     for (uint32_t i = 0; i < count; i++)
//     {
//         img_exec_dispatch(ctx, ops[i].op_code, buf, ops[i].params);
//     }
// }
