// ./src/cold/pipeline_compiled.c

#include "pipeline/pipeline_types.h"

#include "pipeline/pipeline_compiled.h"
#include "pipeline/jump_table.h"

int img_pipeline_compile(
    const img_pipeline_desc_t *in,
    img_pipeline_compiled_t *out)
{
    if (!in || !out)
        return -1;

    if (in->count == 0 || in->count > IMG_MAX_PIPELINE_OPS)
        return -1;

    out->count = in->count;

    for (uint32_t i = 0; i < in->count; i++)
    {
        uint32_t opcode = in->ops[i].op_code;

        img_kernel_fn fn = g_jump_table[opcode];

        if (!fn)
            return -1;

        out->ops[i] = fn;
        out->params[i] = in->ops[i].params;
    }

    return 0;
}
