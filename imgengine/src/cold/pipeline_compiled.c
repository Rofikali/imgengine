// src/cold/pipeline_compiled.c

#include "pipeline/pipeline_compiled.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"
#include "hot/pipeline_exec_inline.h"
#include "core/buffer.h"
#include "arch/arch_interface.h"

#include "pipeline/pipeline_compiled.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_types.h"

int img_pipeline_compile(
    const img_pipeline_desc_t *in,
    img_pipeline_compiled_t *out)
{
    if (!in || !out)
        return -1;

    uint32_t count = in->count;

    if (count == 0 || count > IMG_MAX_PIPELINE_OPS)
        return -1;

    out->count = count;

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t opcode = in->ops[i].op_code;

        img_kernel_fn fn = g_jump_table[opcode];

        if (!fn)
            return -1;

        /*
         * 🔥 STORE DIRECT KERNEL FN (NO ADAPT IN HOT PATH)
         */
        out->ops[i] = fn;
        out->params[i] = in->ops[i].params;
    }

    return 0;
}

// int img_pipeline_compile(
//     const img_pipeline_desc_t *in,
//     img_pipeline_compiled_t *out)
// {
//     if (!in || !out)
//         return -1;

//     uint32_t count = in->count;

//     if (count == 0 || count > IMG_MAX_PIPELINE_OPS)
//         return -1;

//     out->count = count;

//     for (uint32_t i = 0; i < count; i++)
//     {
//         uint32_t opcode = in->ops[i].op_code;

//         img_kernel_fn fn = g_jump_table[opcode];

//         if (!fn)
//             return -1;

//         // 🔥 resolve ONCE
//         out->ops[i] = fn;
//         out->params[i] = in->ops[i].params;
//     }

//     return 0;
// }
