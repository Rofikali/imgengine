// ./src/api/api_task_submit.c
// src/api/api_task_submit.c

#include "api/api_internal.h"
#include "runtime/scheduler.h"
#include "runtime/task.h"

extern img_scheduler_t g_scheduler;

int img_submit_task(img_task_t *task) {
    if (!task)
        return 0;

    return img_scheduler_submit_external(&g_scheduler, &g_task_queue, task);
}
