#include "pipeline/threaded/pipeline_threaded.h"
#include "runtime/scheduler/scheduler.h"

#include <stdlib.h>

typedef struct img_task
{
    img_pipeline_runtime_t *pipeline;
    img_buffer_t *buffer;

    int status;

} img_task_t;

int img_pipeline_submit(
    img_scheduler_t *sched,
    img_pipeline_runtime_t *pipe,
    img_buffer_t *buf)
{
    if (!sched || !pipe || !buf)
        return -1;

    img_task_t *task = malloc(sizeof(*task));
    if (!task)
        return -1;

    task->pipeline = pipe;
    task->buffer = buf;
    task->status = -1;

    return img_scheduler_submit(sched, task);
}