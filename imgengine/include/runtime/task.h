// runtime/task.h

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H

#include "api/v1/img_types.h"
#include "pipeline/pipeline.h"

typedef struct
{
    img_buffer_t buffer;
    img_pipeline_desc_t *pipeline;

    int status;

} img_task_t;

#endif