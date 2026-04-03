// runtime/task.h

// #ifndef IMGENGINE_RUNTIME_TASK_H
// #define IMGENGINE_RUNTIME_TASK_H

// #include <stdint.h>
// #include <stddef.h>
// #include "api/v1/img_types.h"

// typedef struct
// {
//     // Input
//     const uint8_t *input;
//     size_t input_size;

//     // Pipeline
//     img_pipeline_desc_t *pipeline;

//     // Output
//     uint8_t *output;
//     size_t output_size;

//     // Result
//     int status;

// } img_task_t;

// #endif

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H

#include "api/v1/img_types.h"
#include "pipeline/pipeline.h"

typedef struct
{
    img_buffer_t buffer;
    img_pipeline_t *pipeline;

    int status;

} img_task_t;

#endif