// include/hot/pipelined_compiled.h

#ifndef IMGENGINE_PIPELINE_COMPILED_H
#define IMGENGINE_PIPELINE_COMPILED_H

#include "pipeline/jump_table.h"

#define IMG_MAX_PIPELINE_OPS 16

typedef struct
{
    img_op_fn ops[IMG_MAX_PIPELINE_OPS];
    void *params[IMG_MAX_PIPELINE_OPS];
    uint32_t count;

} img_pipeline_compiled_t;

#endif