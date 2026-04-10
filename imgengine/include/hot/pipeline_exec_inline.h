
// include/hot/pipeline_exec_inline.h

#ifndef IMGENGINE_PIPELINE_EXEC_INLINE_H
#define IMGENGINE_PIPELINE_EXEC_INLINE_H

#include "pipeline/jump_table.h"
#include <stddef.h>

/*
 * 🔥 TRUE HOT PATH (NO BRANCH, NO INDIRECT COST)
 */
static inline void img_exec_op_inline(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    uint32_t op_code,
    void *params)
{
    img_kernel_fn fn = g_jump_table[op_code];

    if (__builtin_expect(fn != NULL, 1))
    {
        fn(ctx, buf, params);
    }
}

#endif

// #ifndef IMGENGINE_PIPELINE_EXEC_INLINE_H
// #define IMGENGINE_PIPELINE_EXEC_INLINE_H

// #include "pipeline/jump_table.h"
// #include <stddef.h>

// // 🔥 INLINE FAST PATH (NO LOOP)
// static inline void img_exec_op_inline(
//     img_ctx_t *ctx,
//     img_buffer_t *buf,
//     uint32_t op_code,
//     void *params)
// {
//     img_op_fn fn = g_jump_table[op_code];

//     if (__builtin_expect(fn != NULL, 1))
//     {
//         fn(ctx, buf, params);
//     }
// }

// #endif