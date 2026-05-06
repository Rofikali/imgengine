// ./src/api/api_task_submit.c
// src/api/api_task_submit.c

#include "api/api_internal.h"
#include "runtime/task.h"

int img_submit_task(img_task_t *task) {
    if (!task)
        return 0;

    return img_mpmc_push(&g_task_queue, task);
}
