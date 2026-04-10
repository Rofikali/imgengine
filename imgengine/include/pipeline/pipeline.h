
// include/pipeline/pipeline.h

#ifndef IMGENGINE_PIPELINE_H
#define IMGENGINE_PIPELINE_H

#include <stdint.h>

/* Forward Declarations (Opaque Handles) */
typedef struct img_op_desc img_op_desc_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

typedef struct img_pipeline_runtime
{
    img_op_desc_t *ops;
    uint32_t count;
} img_pipeline_runtime_t;

// 🔥 STRICT PIPELINE CONTRACT
// typedef struct
// {
//     img_op_desc_t *ops; // Pointer: Size is always 8 bytes (on x86_64)
//     uint32_t count;
// } img_pipeline_runtime_t;

// // Build function
// int img_pipeline_build(
//     const img_pipeline_desc_t *desc,
//     img_pipeline_runtime_t *runtime);

#endif
