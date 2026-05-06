// ./include/pipeline/pipeline.h

// include/pipeline/pipeline.h

#ifndef IMGENGINE_PIPELINE_H
#define IMGENGINE_PIPELINE_H

#include <stdint.h>

/* Forward Declarations (Opaque Handles) */
typedef struct img_op_desc img_op_desc_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

typedef struct img_pipeline_runtime {
    img_op_desc_t *ops;
    uint32_t count;
} img_pipeline_runtime_t;

#endif
