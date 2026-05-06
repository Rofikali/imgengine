// ./src/runtime/scheduler_submit.c
#include "runtime/scheduler.h"
#include "runtime/worker.h"

#include "core/context_internal.h"
#include <immintrin.h>

int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task) {
    if (!sched || !task)
        return -1;

    uint32_t target = (uintptr_t)task % sched->worker_count;
    img_worker_t *w = &sched->workers[target];

    if (img_queue_push(w->queue, task))
        return 0;

    return img_mpmc_push(&sched->global_queue, task);
}

int img_runtime_submit_task(img_engine_t *engine, img_task_t *task) {
    if (!engine || !task || !engine->scheduler || engine->worker_count == 0)
        return -1;

    return img_scheduler_submit(engine->scheduler, task);
}

img_result_t img_runtime_wait_task(img_task_t *task) {
    uint32_t spin = 0;

    if (!task)
        return IMG_ERR_SECURITY;

    while (__builtin_expect(task->state != IMG_TASK_DONE, 1)) {
        if (spin++ < 100000)
            _mm_pause();
        else
            sched_yield();
    }

    return (img_result_t)task->status;
}
