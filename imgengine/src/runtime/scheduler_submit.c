// ./src/runtime/scheduler_submit.c
#include "runtime/scheduler.h"
#include "runtime/worker.h"

#include "core/context_internal.h"
#include <immintrin.h>

static int img_scheduler_submit_to_queue(img_scheduler_t *sched, img_mpmc_queue_t *overflow,
                                         img_task_t *task) {
    if (!sched || !overflow || !task || !sched->lifecycle_initialized)
        return -1;
    pthread_mutex_lock(&sched->admission_lock);
    if (!atomic_load_explicit(&sched->accepting, memory_order_acquire)) {
        pthread_mutex_unlock(&sched->admission_lock);
        return -1;
    }
    img_task_state_t expected = IMG_TASK_READY;
    if (!atomic_compare_exchange_strong_explicit(&task->state, &expected, IMG_TASK_QUEUED,
                                                 memory_order_release, memory_order_acquire)) {
        pthread_mutex_unlock(&sched->admission_lock);
        return -1;
    }
    uint32_t target = (uintptr_t)task % sched->worker_count;
    img_worker_t *worker = &sched->workers[target];
    if (img_queue_push(worker->queue, task) || img_mpmc_push(overflow, task) == 0) {
        pthread_mutex_unlock(&sched->admission_lock);
        return 0;
    }
    atomic_store_explicit(&task->state, IMG_TASK_READY, memory_order_release);
    pthread_mutex_unlock(&sched->admission_lock);
    return -1;
}

int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task) {
    return img_scheduler_submit_to_queue(sched, sched ? &sched->global_queue : NULL, task);
}

int img_scheduler_submit_external(img_scheduler_t *sched, img_mpmc_queue_t *queue,
                                  img_task_t *task) {
    return img_scheduler_submit_to_queue(sched, queue, task);
}

bool img_scheduler_is_accepting(const img_scheduler_t *sched) {
    return sched && sched->lifecycle_initialized &&
           atomic_load_explicit(&sched->accepting, memory_order_acquire);
}

int img_scheduler_attach_external_queue(img_scheduler_t *sched, img_mpmc_queue_t *queue) {
    if (!sched || !queue || !sched->lifecycle_initialized)
        return -1;
    pthread_mutex_lock(&sched->admission_lock);
    if (!atomic_load_explicit(&sched->accepting, memory_order_acquire) || sched->external_queue) {
        pthread_mutex_unlock(&sched->admission_lock);
        return -1;
    }
    sched->external_queue = queue;
    pthread_mutex_unlock(&sched->admission_lock);
    return 0;
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

    for (;;) {
        const img_task_state_t state = atomic_load_explicit(&task->state, memory_order_acquire);
        if (state == IMG_TASK_DONE || state == IMG_TASK_REJECTED)
            return (img_result_t)task->status;
        if (spin++ < 100000)
            _mm_pause();
        else
            sched_yield();
    }
}
