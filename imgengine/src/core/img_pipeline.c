/* src/core/img_pipeline.c */

#include "api/v1/img_pipeline.h"

void img_pipeline_init(img_pipeline_desc_t *pipe)
{
    pipe->count = 0;
}

int img_pipeline_add_op(img_pipeline_desc_t *pipe,
                        uint32_t op_code,
                        void *params)
{
    if (!pipe || pipe->count >= IMG_MAX_PIPELINE_OPS)
        return -1;

    pipe->ops[pipe->count].op_code = op_code;
    pipe->ops[pipe->count].params = params;
    pipe->count++;

    return 0;
}