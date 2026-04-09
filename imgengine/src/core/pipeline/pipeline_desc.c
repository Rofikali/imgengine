
#include "core/pipeline/pipeline_types.h"

int img_pipeline_add(
    img_pipeline_desc_t *p,
    uint32_t opcode,
    void *params)
{
    if (!p || p->count >= IMG_MAX_PIPELINE_OPS)
        return -1;

    p->ops[p->count].op_code = opcode;
    p->ops[p->count].params = params;

    p->count++;
    return 0;
}