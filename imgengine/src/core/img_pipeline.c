/* src/core/img_pipeline.c */

#include "api/v1/img_pipeline.h"
#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline_compiled.h"
#include <stddef.h>
#include <stdlib.h>

img_pipeline_t *img_pipeline_create(void)
{
    img_pipeline_desc_t *pipe =
        malloc(sizeof(img_pipeline_desc_t));

    if (!pipe)
        return NULL;

    pipe->count = 0;
    return (img_pipeline_t *)pipe;
}

void img_pipeline_destroy(img_pipeline_t *pipe)
{
    free(pipe);
}

int img_pipeline_add_op(
    img_pipeline_t *p,
    uint32_t op_code,
    void *params)
{
    img_pipeline_desc_t *pipe = (img_pipeline_desc_t *)p;

    if (!pipe || pipe->count >= IMG_MAX_PIPELINE_OPS)
        return -1;

    pipe->ops[pipe->count].op_code = op_code;
    pipe->ops[pipe->count].params = params;
    pipe->count++;

    return 0;
}
