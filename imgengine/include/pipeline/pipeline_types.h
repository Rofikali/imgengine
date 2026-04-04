// src/pipeline/pipeline_types.h 

#ifndef IMGENGINE_PIPELINE_TYPES_H
#define IMGENGINE_PIPELINE_TYPES_H

#include <stdint.h>

typedef struct img_op_desc
{
    uint32_t op_code;
    void *params;

} img_op_desc_t;

typedef struct img_pipeline_desc
{
    img_op_desc_t *ops;
    uint32_t count;

} img_pipeline_desc_t;

#endif