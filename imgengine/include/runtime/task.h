// /* include/runtime/task.h */

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H

#include <stdint.h>

/*
 * Forward declarations
 */
typedef struct img_buffer img_buffer_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

/*
 * 🔥 Task state
 */
typedef enum
{
    IMG_TASK_READY = 0,
    IMG_TASK_RUNNING,
    IMG_TASK_DONE
} img_task_state_t;

/*
 * 🔥 Task (cache aligned)
 */
typedef struct __attribute__((aligned(64)))
{
    img_buffer_t *buffer;
    img_pipeline_desc_t *pipeline;

    uint32_t op_index;

    img_task_state_t state;

    int status;

} img_task_t;

#endif
