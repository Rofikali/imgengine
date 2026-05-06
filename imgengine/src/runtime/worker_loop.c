// ./src/runtime/worker_loop.c

// ================================================================
// FILE 4: src/runtime/worker_loop.c  (REPLACE)
//
// WAS: only polled per-worker SPSC queue.
//      MPMC submission queue was never polled.
//      Tasks pushed via img_submit_task never reached workers.
//
// FIX: poll MPMC after SPSC miss, before steal.
//      This connects the submission path to execution.
//
// Poll order (fastest to slowest):
//   1. w->queue (SPSC, zero contention, owned by this worker)
//   2. g_task_queue (MPMC, shared submission queue)
//   3. steal from other workers via scheduler
// ================================================================

#define _GNU_SOURCE

#include "runtime/worker_internal.h"
#include "runtime/worker.h"
#include "runtime/job_exec.h"
#include "runtime/scheduler.h"
#include "runtime/queue_mpmc.h"
#include "hot/pipeline_exec.h"
#include "observability/binlog.h"
#include <immintrin.h>

extern img_mpmc_queue_t g_task_queue; /* defined in api/api_state.c */
extern void img_binlog_set_cpu(uint32_t cpu);

void *img_worker_loop(void *arg) {
    img_worker_t *w = (img_worker_t *)arg;

    img_binlog_set_cpu(w->id);

    while (__builtin_expect(w->running, 1)) {
        img_task_t *task = NULL;

        /* 1. Own SPSC queue — zero contention */
        task = (img_task_t *)img_queue_pop(w->queue);

        /* 2. Global MPMC submission queue — this is where api submits */
        if (!task)
            task = (img_task_t *)img_mpmc_pop(&g_task_queue);

        /* 3. Work steal from other workers */
        if (!task && w->scheduler)
            task = img_scheduler_steal(w->scheduler, w->id);

        if (!task) {
            _mm_pause();
            continue;
        }

        if (__builtin_expect(!w->ctx, 0)) {
            task->status = IMG_ERR_INTERNAL;
            task->state = IMG_TASK_DONE;
            continue;
        }

        task->state = IMG_TASK_RUNNING;

        switch (task->kind) {
        case IMG_TASK_KIND_PIPELINE:
            if (!task->payload.pipeline.pipeline || !task->payload.pipeline.buffer) {
                task->status = IMG_ERR_SECURITY;
                break;
            }

            img_pipeline_execute_hot(w->ctx, task->payload.pipeline.pipeline,
                                     task->payload.pipeline.buffer);
            task->status = IMG_SUCCESS;
            break;

        case IMG_TASK_KIND_RENDER_STAGE: {
            img_ctx_t *ctx = task->payload.render.ctx;
            if (!ctx)
                ctx = w->ctx;

            task->status = img_runtime_prepare_render_stage(
                task->payload.render.engine, ctx, task->payload.render.canvas,
                task->payload.render.layout, task->payload.render.job, task->payload.render.photo,
                task->payload.render.arena);
            break;
        }

        default:
            task->status = IMG_ERR_INTERNAL;
            break;
        }

        task->state = IMG_TASK_DONE;
    }

    return NULL;
}

// // ./src/runtime/worker_loop.c
// #include "runtime/worker_internal.h"
// #include "runtime/worker.h"
// #include "runtime/scheduler.h"
// #include "hot/pipeline_exec.h"
// #include "observability/binlog.h"

// #include <immintrin.h>

// extern void img_binlog_set_cpu(uint32_t cpu);

// void *img_worker_loop(void *arg)
// {
//     img_worker_t *w = (img_worker_t *)arg;

//     img_binlog_set_cpu(w->id);

//     while (__builtin_expect(w->running, 1))
//     {
//         img_task_t *task = (img_task_t *)img_queue_pop(w->queue);

//         /*
//          * STEAL: only attempt if scheduler is wired.
//          * w->scheduler is set before pthread_create (see init below)
//          * so this check is purely defensive — should never be NULL.
//          */
//         if (!task && w->scheduler)
//             task = img_scheduler_steal(w->scheduler, w->id);

//         if (!task)
//         {
//             _mm_pause();
//             continue;
//         }

//         /*
//          * Guard: ctx must be set. If NULL, skip task — don't crash.
//          * In normal operation ctx is always set before thread starts.
//          */
//         if (__builtin_expect(!w->ctx, 0))
//         {
//             task->status = -1;
//             continue;
//         }

//         img_pipeline_execute_hot(
//             w->ctx,
//             task->pipeline,
//             task->buffer);

//         task->status = 0;
//     }

//     return NULL;
// }
