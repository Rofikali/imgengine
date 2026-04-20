// ./src/pipeline/pipeline_executor.c

#define _GNU_SOURCE

#include "core/pipeline_types.h"
#include <stddef.h>
#include <stdlib.h>

/*
 * img_pipeline_t is a public API type but its implementation
 * is pipeline_desc_t. We use an opaque cast.
 *
 * Removed: #include "api/v1/img_pipeline.h"
 * Reason: core/ (0) must not include api/ (9).
 * The public API header is only needed by callers of img_pipeline_*,
 * not by the implementation itself.
 *
 * The function signatures are declared in api/v1/img_pipeline.h
 * which consumers include. This .c file implements them without
 * re-including the header — the linker resolves the symbols.
 */

/* forward: these match what api/v1/img_pipeline.h declares */
typedef struct img_pipeline img_pipeline_t;

img_pipeline_t *img_pipeline_create(void)
{
    img_pipeline_desc_t *pipe = malloc(sizeof(img_pipeline_desc_t));
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
