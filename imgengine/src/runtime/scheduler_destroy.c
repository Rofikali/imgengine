// ./src/runtime/scheduler_destroy.c
#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "pipeline/render_cache.h"

#include <stdlib.h>
#include <string.h>

static void img_scheduler_reject_task(img_task_t *task) {
    if (!task)
        return;
    const img_task_state_t state = atomic_load_explicit(&task->state, memory_order_acquire);
    if (state == IMG_TASK_QUEUED || state == IMG_TASK_READY) {
        task->status = IMG_ERR_INTERNAL;
        atomic_store_explicit(&task->state, IMG_TASK_REJECTED, memory_order_release);
    }
}

static void img_scheduler_reject_mpmc(img_mpmc_queue_t *queue) {
    if (!queue || !queue->cells)
        return;
    for (;;) {
        img_task_t *task = img_mpmc_pop(queue);
        if (!task)
            return;
        img_scheduler_reject_task(task);
    }
}

void img_scheduler_close(img_scheduler_t *sched) {
    if (!sched || !sched->lifecycle_initialized)
        return;
    pthread_mutex_lock(&sched->admission_lock);
    atomic_store_explicit(&sched->accepting, false, memory_order_release);
    pthread_mutex_unlock(&sched->admission_lock);
}

void img_scheduler_destroy(img_scheduler_t *sched) {
    if (!sched || !sched->lifecycle_initialized)
        return;

    img_scheduler_close(sched);
    for (uint32_t i = 0; i < sched->worker_count; i++) {
        if (sched->workers[i].thread_started)
            img_worker_join(&sched->workers[i]);
    }

    for (uint32_t i = 0; i < sched->worker_count; i++) {
        img_queue_t *queue = sched->workers[i].queue;
        for (img_task_t *task = queue ? img_queue_pop(queue) : NULL; task;
             task = img_queue_pop(queue))
            img_scheduler_reject_task(task);
    }
    img_scheduler_reject_mpmc(&sched->global_queue);
    img_scheduler_reject_mpmc(sched->external_queue);

    for (uint32_t i = 0; i < sched->worker_count; i++) {
        img_render_cache_discard(&sched->workers[i].render_cache);
        if (sched->workers[i].queue) {
            img_queue_destroy(sched->workers[i].queue);
            sched->workers[i].queue = NULL;
        }
    }

    img_mpmc_destroy(&sched->global_queue);
    free(sched->workers);
    pthread_mutex_destroy(&sched->admission_lock);
    memset(sched, 0, sizeof(*sched));
}
