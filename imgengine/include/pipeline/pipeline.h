// include/pipeline/pipeline.h

#ifndef IMGENGINE_PIPELINE_H
#define IMGENGINE_PIPELINE_H

#include "api/v1/img_types.h"

// 🔥 STRICT PIPELINE CONTRACT (NO INTERNAL LEAK)

typedef struct
{
    img_op_desc_t *ops;
    uint32_t count;
} img_pipeline_runtime_t;

// Convert user descriptor → runtime (cold path)
int img_pipeline_build(
    const img_pipeline_desc_t *desc,
    img_pipeline_runtime_t *runtime);

#endif