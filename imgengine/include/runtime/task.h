/* include/runtime/task.h */

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H
#include "pipeline/pipeline_types.h"

#include <stdint.h>

/*
 * 🔥 Forward declarations (NO heavy includes)
 */
typedef struct img_buffer img_buffer_t;
// typedef struct img_pipeline_desc img_pipeline_desc_t;

/*
 * 🔥 Ownership model (L8 CRITICAL)
 */
typedef enum
{
    IMG_TASK_OWNED = 0, // worker owns buffer
    IMG_TASK_BORROWED   // external memory (no free)
} img_task_ownership_t;

/*
 * 🔥 Task = execution unit (ZERO-COPY)
 */
typedef struct
{
    img_buffer_t *buffer;          // 🔥 pointer, not copy
    img_pipeline_desc_t *pipeline; // pipeline DAG

    img_task_ownership_t ownership;

    int status;

} img_task_t;

#endif


