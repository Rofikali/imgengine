// ./src/api/api_init_workers.c

// ================================================================
// FILE 3: src/api/api_init_workers.c  (REPLACE)
//
// WAS:
//   img_worker_init(&g_workers[i], i, NULL, &g_worker_ctxs[i])
//   scheduler not initialized
//   task queue initialized AFTER workers started
//
// FIX:
//   1. Init task queue FIRST (workers poll it on start)
//   2. Build one ctx per worker BEFORE thread creation
//   3. Pass valid scheduler to every worker
//   4. Start threads only after ctx and scheduler are set
// ================================================================

#include "api/api_init_internal.h"
#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "runtime/worker_internal.h"
#include "runtime/affinity.h"

/*
 * Single scheduler for the engine.
 * Lives here — api layer owns engine lifecycle.
 * Exposed via g_scheduler for img_submit_task to use.
 */
img_scheduler_t g_scheduler;

int img_api_init_boot_workers(uint32_t workers) {
    if (workers == 0 || workers > 64)
        return -1;

    /*
     * STEP 1: Initialize MPMC task submission queue.
     * Must exist before any worker thread starts polling it.
     */
    if (img_api_init_prepare_task_queue() != 0)
        return -1;

    /*
     * STEP 2: Initialize scheduler (allocates worker array,
     * creates per-worker SPSC queues, does NOT start threads).
     */
    if (img_scheduler_init(&g_scheduler, workers) != 0) {
        img_mpmc_destroy(&g_task_queue);
        return -1;
    }

    /*
     * STEP 3: Build one ctx per worker and assign to scheduler slots.
     * ctx carries: caps, local_pool, thread_id.
     * These must be set BEFORE pthread_create so the thread never
     * sees a NULL ctx when it picks up its first task.
     */
    for (uint32_t i = 0; i < workers; i++) {
        g_worker_ctxs[i].thread_id = i;
        g_worker_ctxs[i].caps = g_engine.caps;
        g_worker_ctxs[i].local_pool = g_engine.global_pool;
        g_worker_ctxs[i].scratch_arena = NULL;
        g_scheduler.workers[i].render_cache.pool = g_engine.global_pool;
        g_worker_ctxs[i].op_params = &g_scheduler.workers[i].render_cache;
        g_worker_ctxs[i].fused_params = NULL;

        /*
         * Set ctx on the pre-allocated worker slot.
         * Scheduler owns these workers — we set ctx here,
         * then worker_start will call pthread_create.
         */
        g_scheduler.workers[i].ctx = &g_worker_ctxs[i];
    }

    /*
     * STEP 4: Start all worker threads.
     * At this point every worker has:
     *   w->scheduler  = &g_scheduler   (non-NULL, steal works)
     *   w->ctx        = &g_worker_ctxs[i]  (non-NULL, execute works)
     *   w->queue      = per-worker SPSC  (created in scheduler_init)
     *   w->running    = 1
     */
    for (uint32_t i = 0; i < workers; i++) {
        img_worker_t *w = &g_scheduler.workers[i];
        w->running = 1;

        img_pin_thread_to_core(i);

        if (pthread_create(&w->thread, NULL, img_worker_loop, w) != 0) {
            /* stop already-started threads */
            for (uint32_t j = 0; j < i; j++) {
                g_scheduler.workers[j].running = 0;
                pthread_join(g_scheduler.workers[j].thread, NULL);
            }
            img_scheduler_destroy(&g_scheduler);
            img_mpmc_destroy(&g_task_queue);
            return -1;
        }
    }

    g_engine.scheduler = &g_scheduler;
    g_engine.workers = g_scheduler.workers;

    return 0;
}

// // ./src/api/api_init_workers.c
// #include "api/api_init_internal.h"
// int img_api_init_boot_workers(uint32_t workers)
// {
//     uint32_t initialized = 0;

//     for (uint32_t i = 0; i < workers; i++)
//     {
//         if (img_api_init_prepare_worker(i) != 0)
//         {
//             img_api_init_rollback(initialized);
//             return -1;
//         }

//         initialized++;
//     }

//     if (img_api_init_prepare_task_queue() != 0)
//     {
//         img_api_init_rollback(initialized);
//         return -1;
//     }

//     return 0;
// }
