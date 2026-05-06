// ./include/core/pipeline_types.h
#ifndef IMGENGINE_CORE_PIPELINE_TYPES_H
#define IMGENGINE_CORE_PIPELINE_TYPES_H

#include <stdint.h>

#define IMG_MAX_PIPELINE_OPS 32

typedef struct img_op_desc {
    uint32_t op_code;
    void *params;
} __attribute__((aligned(64))) img_op_desc_t;

typedef struct img_pipeline_desc {
    img_op_desc_t ops[IMG_MAX_PIPELINE_OPS];
    uint32_t count;
} img_pipeline_desc_t;

#endif /* IMGENGINE_CORE_PIPELINE_TYPES_H */
