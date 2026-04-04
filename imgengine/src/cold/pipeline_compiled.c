// src/cold/pipeline_compiled.c

#include "pipeline/pipeline_compiled.h"
#include "pipeline/jump_table.h"
#include "api/v1/img_types.h"

int img_pipeline_compile(
    const img_pipeline_desc_t *in,
    img_pipeline_compiled_t *out)
{
    if (!in || !out)
        return -1;

    if (in->count > IMG_MAX_PIPELINE_OPS)
        return -1;

    out->count = in->count;

    for (uint32_t i = 0; i < in->count; i++)
    {
        img_op_fn fn = g_jump_table[in->ops[i].op_code];

        if (!fn)
            return -1;

        out->ops[i] = fn;
        out->params[i] = in->ops[i].params;
    }

    return 0;
}