#ifndef IMGENGINE_PIPELINE_TYPES_H
#define IMGENGINE_PIPELINE_TYPES_H

#include <stdint.h>
#include "arch/arch_interface.h"

#define IMG_MAX_PIPELINE_OPS 64

/*
 * 🔥 USER PIPELINE (BUILD PHASE)
 */
typedef struct
{
    uint32_t op_code;
    void *params;

} img_pipeline_op_t;

typedef struct
{
    img_pipeline_op_t ops[IMG_MAX_PIPELINE_OPS];
    uint32_t count;

} img_pipeline_desc_t;

/*
 * 🔥 COMPILED PIPELINE (HOT PATH)
 */
typedef struct
{
    img_kernel_fn fn;
    void *params;

} img_pipeline_step_t;

typedef struct
{
    img_pipeline_step_t steps[IMG_MAX_PIPELINE_OPS];
    uint32_t count;

} img_pipeline_compiled_t;

#endif