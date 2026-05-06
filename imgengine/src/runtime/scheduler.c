// ./src/runtime/scheduler.c
#include "runtime/scheduler.h"
#include "runtime/worker.h"

img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id) {
    if (!sched)
        return NULL;

    uint32_t n = sched->worker_count;

    for (uint32_t i = 0; i < n; i++) {
        uint32_t victim = (self_id + i + 1) % n;
        img_task_t *task = (img_task_t *)img_queue_pop(sched->workers[victim].queue);
        if (task)
            return task;
    }

    return (img_task_t *)img_mpmc_pop(&sched->global_queue);
}
