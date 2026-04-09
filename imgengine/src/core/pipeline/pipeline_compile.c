
#include "core/pipeline/pipeline_types.h"
#include "pipeline/dispatch/jump_table.h"
#include "core/pipeline/opcodes.h"

/*
 * 🔥 VALIDATION (no unsafe pipelines)
 */
static int validate_pipeline(const img_pipeline_desc_t *p)
{
    if (!p || p->count == 0)
        return -1;

    for (uint32_t i = 0; i < p->count; i++)
    {
        if (p->ops[i].op_code >= OP_MAX)
            return -1;
    }

    return 0;
}

/*
 * 🔥 COMPILE (resolve + fuse)
 */
int img_pipeline_compile(
    const img_pipeline_desc_t *in,
    img_pipeline_compiled_t *out)
{
    if (validate_pipeline(in) != 0)
        return -1;

    uint32_t out_idx = 0;

    for (uint32_t i = 0; i < in->count; i++)
    {
        uint32_t op = in->ops[i].op_code;

        img_kernel_fn fn = g_jump_table[op];
        if (!fn)
            return -1;

        /*
         * 🔥 FUTURE: OP FUSION POINT
         * Example:
         * resize + grayscale → fused kernel
         */
        if (i + 1 < in->count)
        {
            uint32_t next = in->ops[i + 1].op_code;

            // Example fusion (stub)
            if (op == OP_RESIZE && next == OP_GRAYSCALE)
            {
                // skip fusion for now (placeholder)
            }
        }

        out->steps[out_idx].fn = fn;
        out->steps[out_idx].params = in->ops[i].params;

        out_idx++;
    }

    out->count = out_idx;
    return 0;
}