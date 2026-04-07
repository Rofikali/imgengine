// /* pipeline/scheduler.c */

#include "runtime/scheduler.h"
#include "runtime/worker.h"

#include <stdlib.h>

// ============================================
// INIT
// ============================================

int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)
{
    if (!sched || workers == 0)
        return -1;

    sched->worker_count = workers;

    sched->workers = calloc(workers, sizeof(img_worker_t));
    if (!sched->workers)
        return -1;

    img_mpmc_init(&sched->global_queue, 1024);

    for (uint32_t i = 0; i < workers; i++)
    {
        img_worker_init(&sched->workers[i], i);
        sched->workers[i].scheduler = sched;
    }

    return 0;
}

// ============================================
// DESTROY
// ============================================

void img_scheduler_destroy(img_scheduler_t *sched)
{
    if (!sched)
        return;

    for (uint32_t i = 0; i < sched->worker_count; i++)
    {
        img_worker_stop(&sched->workers[i]);
        img_worker_join(&sched->workers[i]);
    }

    img_mpmc_destroy(&sched->global_queue);
    free(sched->workers);
}

// ============================================
// SUBMIT
// ============================================

int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task)
{
    if (!sched || !task)
        return -1;

    uint32_t target = (uintptr_t)task % sched->worker_count;

    img_worker_t *w = &sched->workers[target];

    // 🔥 FAST PATH (SPSC)
    if (img_queue_push(w->queue, task))
        return 0;

    // 🔥 FALLBACK (MPMC)
    return img_mpmc_push(&sched->global_queue, task);
}

// ============================================
// STEAL
// ============================================

img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id)
{
    uint32_t n = sched->worker_count;

    for (uint32_t i = 0; i < n; i++)
    {
        uint32_t victim = (self_id + i + 1) % n;

        img_worker_t *w = &sched->workers[victim];

        img_task_t *task =
            (img_task_t *)img_queue_pop(w->queue);

        if (task)
            return task;
    }

    return (img_task_t *)img_mpmc_pop(&sched->global_queue);
}

// #include "runtime/scheduler.h"
// #include "runtime/worker.h"

// #include <stdlib.h>

// // ============================================
// // INIT
// // ============================================

// int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)
// {
//     if (!sched || workers == 0)
//         return -1;

//     sched->worker_count = workers;

//     sched->workers = calloc(workers, sizeof(img_worker_t));
//     if (!sched->workers)
//         return -1;

//     // 🔥 GLOBAL MPMC
//     img_mpmc_init(&sched->global_queue, 1024);

//     for (uint32_t i = 0; i < workers; i++)
//     {
//         img_worker_init(&sched->workers[i], i);
//         sched->workers[i].scheduler = sched;
//     }

//     return 0;
// }

// // ============================================
// // DESTROY
// // ============================================

// void img_scheduler_destroy(img_scheduler_t *sched)
// {
//     if (!sched)
//         return;

//     for (uint32_t i = 0; i < sched->worker_count; i++)
//     {
//         img_worker_stop(&sched->workers[i]);
//         img_worker_join(&sched->workers[i]);
//     }

//     img_mpmc_destroy(&sched->global_queue);

//     free(sched->workers);
// }

// // ============================================
// // SUBMIT
// // ============================================

// int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task)
// {
//     if (!sched || !task)
//         return -1;

//     uint32_t target = (uintptr_t)task % sched->worker_count;

//     img_worker_t *w = &sched->workers[target];

//     // 🔥 FAST PATH (SPSC)
//     if (img_queue_push(w->queue, task))
//         return 0;

//     // 🔥 FALLBACK (MPMC)
//     return img_mpmc_push(&sched->global_queue, task);
// }

// // ============================================
// // STEAL
// // ============================================

// img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id)
// {
//     uint32_t n = sched->worker_count;

//     // 🔥 STEAL FROM OTHER WORKERS
//     for (uint32_t i = 0; i < n; i++)
//     {
//         uint32_t victim = (self_id + i + 1) % n;

//         img_worker_t *w = &sched->workers[victim];

//         img_task_t *task =
//             (img_task_t *)img_queue_pop(w->queue);

//         if (task)
//             return task;
//     }

//     // 🔥 GLOBAL FALLBACK
//     return (img_task_t *)img_mpmc_pop(&sched->global_queue);
// }

// // #include "runtime/scheduler.h"
// // #include "runtime/worker.h"
// // #include "runtime/affinity.h"
// // // #include <numa.h>   // numa_available, numa_alloc_onnode, numa_free
// // // #include <numaif.h> // sometimes needed for numa functions
// // // #include <sched.h>  // sched_getcpu

// // #include <stdlib.h>

// // // ============================================
// // // INIT
// // // ============================================

// // int img_scheduler_init(img_scheduler_t *sched, uint32_t workers)
// // {
// //     if (!sched || workers == 0)
// //         return -1;

// //     sched->worker_count = workers;

// //     sched->workers = calloc(workers, sizeof(img_worker_t));
// //     if (!sched->workers)
// //         return -1;

// //     // 🔥 init global MPMC
// //     img_mpmc_init(&sched->global_queue, 1024);

// //     for (uint32_t i = 0; i < workers; i++)
// //     {
// //         img_worker_init(&sched->workers[i], i);
// //         sched->workers[i].scheduler = sched;
// //     }

// //     return 0;
// // }

// // // ============================================
// // // DESTROY
// // // ============================================

// // void img_scheduler_destroy(img_scheduler_t *sched)
// // {
// //     if (!sched)
// //         return;

// //     for (uint32_t i = 0; i < sched->worker_count; i++)
// //     {
// //         img_worker_stop(&sched->workers[i]);
// //         img_worker_join(&sched->workers[i]);
// //     }

// //     img_mpmc_destroy(&sched->global_queue);
// //     free(sched->workers);
// // }

// // // ============================================
// // // SUBMIT (SMART ROUTING)
// // // ============================================

// // int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task)
// // {
// //     if (!sched || !task)
// //         return -1;

// //     // 🔥 simple hashing → better: NUMA-aware
// //     uint32_t target = (uintptr_t)task % sched->worker_count;

// //     img_worker_t *w = &sched->workers[target];

// //     // FAST PATH (SPSC)
// //     if (img_queue_push(w->queue, task))
// //         return 0;

// //     // FALLBACK (MPMC)
// //     return img_mpmc_push(&sched->global_queue, task);
// // }

// // // ============================================
// // // STEAL LOGIC (CORE MAGIC)
// // // ============================================

// // img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id)
// // {
// //     uint32_t n = sched->worker_count;

// //     // 🔥 try stealing from others
// //     for (uint32_t i = 0; i < n; i++)
// //     {
// //         uint32_t victim = (self_id + i + 1) % n;

// //         img_worker_t *w = &sched->workers[victim];

// //         img_task_t *task =
// //             (img_task_t *)img_queue_pop(w->queue);

// //         if (task)
// //             return task;
// //     }

// //     // 🔥 fallback → global queue
// //     return (img_task_t *)img_mpmc_pop(&sched->global_queue);
// // }
