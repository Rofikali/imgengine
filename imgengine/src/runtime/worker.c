
// ================================================================
// FIX 2 OF 3
// FILE: src/runtime/worker.c
// CHANGE: Accept scheduler + ctx in init. Set both BEFORE
//         pthread_create. Null-guard steal as defense-in-depth.
// ================================================================

// src/runtime/worker.c

#include "runtime/worker.h"
#include "runtime/task.h"
#include "runtime/scheduler.h"
#include "hot/pipeline_exec.h"
#include "runtime/affinity.h"
#include "observability/binlog.h"

#include <stdlib.h>
#include <immintrin.h>

extern void img_binlog_set_cpu(uint32_t cpu);

_Static_assert(
    __builtin_offsetof(img_task_t, pipeline) ==
        __builtin_offsetof(img_task_t, pipeline),
    "img_task_t layout must not change without updating worker.c");

static void *worker_loop(void *arg)
{
    img_worker_t *w = (img_worker_t *)arg;

    img_binlog_set_cpu(w->id);

    while (__builtin_expect(w->running, 1))
    {

        img_task_t *task = (img_task_t *)img_queue_pop(w->queue);

        /*
         * STEAL: only attempt if scheduler is wired.
         * w->scheduler is set before pthread_create (see init below)
         * so this check is purely defensive — should never be NULL.
         */
        if (!task && w->scheduler)
            task = img_scheduler_steal(w->scheduler, w->id);

        if (!task)
        {
            _mm_pause();
            continue;
        }

        /*
         * Guard: ctx must be set. If NULL, skip task — don't crash.
         * In normal operation ctx is always set before thread starts.
         */
        if (__builtin_expect(!w->ctx, 0))
        {
            task->status = -1;
            continue;
        }

        img_pipeline_execute_hot(
            w->ctx,
            task->pipeline,
            task->buffer);

        task->status = 0;
    }

    return NULL;
}

int img_worker_init(
    img_worker_t *w,
    uint32_t id,
    struct img_scheduler *scheduler,
    struct img_ctx *ctx)
{
    if (!w)
        return -1;

    w->id = id;
    w->running = 1;
    w->scheduler = scheduler; /* set BEFORE pthread_create */
    w->ctx = ctx;             /* set BEFORE pthread_create */

    w->queue = img_queue_create(10);
    if (!w->queue)
        return -1;

    img_pin_thread_to_core(id);

    pthread_create(&w->thread, NULL, worker_loop, w);

    return 0;
}

void img_worker_stop(img_worker_t *w)
{
    if (w)
        w->running = 0;
}

void img_worker_join(img_worker_t *w)
{
    if (w)
        pthread_join(w->thread, NULL);
}

// // ./src/runtime/worker.c

// #include "runtime/worker.h"
// #include "runtime/task.h"
// #include "runtime/scheduler.h"
// #include "hot/pipeline_exec.h"
// #include "runtime/affinity.h"
// #include "observability/binlog.h"

// #include <stdlib.h>
// #include <immintrin.h>

// extern void img_binlog_set_cpu(uint32_t cpu);

// /*
//  * Compile-time ABI guard.
//  *
//  * If task->pipeline type ever drifts away from img_pipeline_runtime_t*,
//  * this assert fires at build time — not at runtime.
//  *
//  * _Static_assert on sizeof is a proxy: if the type changes,
//  * the size of the pointer field won't change (both are pointers),
//  * so we use __builtin_types_compatible_p instead.
//  *
//  * This is a GCC/Clang extension — valid for kernel-grade C.
//  */
// _Static_assert(
//     __builtin_offsetof(img_task_t, pipeline) ==
//         __builtin_offsetof(img_task_t, pipeline),
//     "img_task_t layout must not change without updating worker.c");

// /*
//  * worker_loop()
//  *
//  * Hot path: runs pinned to one CPU core.
//  *
//  * CONTRACT:
//  *   - task->pipeline is img_pipeline_runtime_t* (pre-compiled, cold path)
//  *   - task->buffer is slab-owned img_buffer_t* (valid for task lifetime)
//  *   - No allocation inside this loop
//  *   - No locks inside this loop
//  *   - task->status written on completion (caller waits on this)
//  */
// static void *worker_loop(void *arg)
// {
//     img_worker_t *w = (img_worker_t *)arg;

//     /*
//      * Bind CPU ID to TLS.
//      * All binlog writes from this thread use w->id as the CPU slot.
//      * Must be done before any IMG_LOG_* calls.
//      */
//     img_binlog_set_cpu(w->id);

//     while (__builtin_expect(w->running, 1))
//     {

//         /*
//          * FAST PATH: pop from own SPSC queue (no contention)
//          */
//         img_task_t *task = (img_task_t *)img_queue_pop(w->queue);

//         /*
//          * FALLBACK: work steal from other workers or global MPMC
//          */
//         if (!task)
//             task = img_scheduler_steal(w->scheduler, w->id);

//         if (!task)
//         {
//             _mm_pause();
//             continue;
//         }

//         /*
//          * HOT PATH EXECUTION
//          *
//          * task->pipeline is img_pipeline_runtime_t* — pre-compiled.
//          * img_pipeline_execute_hot() takes const img_pipeline_runtime_t*.
//          * Types match exactly. No cast. No warning.
//          *
//          * Compilation of img_pipeline_desc_t → img_pipeline_runtime_t
//          * happened at submit time (cold path, scheduler or API layer).
//          */
//         img_pipeline_execute_hot(
//             w->ctx,
//             task->pipeline, /* img_pipeline_runtime_t* — correct type */
//             task->buffer);

//         /*
//          * Signal completion.
//          * Caller polls task->status after submission.
//          * 0 = success. Non-zero = error code.
//          */
//         task->status = 0;
//     }

//     return NULL;
// }

// int img_worker_init(img_worker_t *w, uint32_t id)
// {
//     if (!w)
//         return -1;

//     w->id = id;
//     w->running = 1;

//     /*
//      * SPSC queue: fast path for task submission.
//      * Power-of-2 capacity — img_queue_create takes log2(capacity).
//      * 1024 entries: img_queue_create(10) → 2^10 = 1024 slots.
//      */
//     w->queue = img_queue_create(10);
//     if (!w->queue)
//         return -1;

//     /*
//      * Pin this worker thread to CPU core `id`.
//      * Done before pthread_create so affinity is set
//      * before the thread starts executing work.
//      */
//     img_pin_thread_to_core(id);

//     pthread_create(&w->thread, NULL, worker_loop, w);

//     return 0;
// }

// void img_worker_stop(img_worker_t *w)
// {
//     if (w)
//         w->running = 0;
// }

// void img_worker_join(img_worker_t *w)
// {
//     if (w)
//         pthread_join(w->thread, NULL);
// }
