
// /* pipeline/scheduler.h */

#ifndef IMGENGINE_RUNTIME_SCHEDULER_H
#define IMGENGINE_RUNTIME_SCHEDULER_H

#include <stdint.h>

#include "runtime/task.h"
#include "runtime/queue_spsc.h"
#include "runtime/queue_mpmc.h"

typedef struct img_worker img_worker_t;

/*
 * 🔥 REAL STRUCT (NOT ANONYMOUS!)
 */
typedef struct img_scheduler
{
    uint32_t worker_count;

    img_worker_t *workers;

    // 🔥 GLOBAL FALLBACK (MPMC)
    img_mpmc_queue_t global_queue;

} img_scheduler_t;

/*
 * Lifecycle
 */
int img_scheduler_init(img_scheduler_t *sched, uint32_t workers);
void img_scheduler_destroy(img_scheduler_t *sched);

/*
 * Submit
 */
int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task);

/*
 * Steal
 */
img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id);

#endif

// #ifndef IMGENGINE_RUNTIME_SCHEDULER_H
// #define IMGENGINE_RUNTIME_SCHEDULER_H

// #include <stdint.h>
// #include "runtime/task.h"
// #include "runtime/queue_spsc.h"
// #include "runtime/queue_mpmc.h"

// /*
//  * 🔥 FORWARD DECL
//  */
// struct img_worker;

// /*
//  * 🔥 REAL STRUCT (NOT OPAQUE)
//  */
// typedef struct img_scheduler
// {
//     uint32_t worker_count;

//     struct img_worker *workers;

//     // 🔥 GLOBAL FALLBACK (MPMC)
//     img_mpmc_queue_t global_queue;

// } img_scheduler_t;

// /*
//  * 🔥 API
//  */
// int img_scheduler_init(img_scheduler_t *sched, uint32_t workers);
// void img_scheduler_destroy(img_scheduler_t *sched);

// int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task);

// img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id);

// #endif

// // #ifndef IMGENGINE_RUNTIME_SCHEDULER_H
// // #define IMGENGINE_RUNTIME_SCHEDULER_H

// // #include <stdint.h>
// // #include "runtime/task.h"
// // // #include "runtime/queue_spsc.h"
// // // #include "runtime/queue_mpmc.h"

// // typedef struct img_worker_s img_worker_t;

// // typedef struct
// // {
// //     uint32_t worker_count;

// //     img_worker_t *workers;

// //     // 🔥 GLOBAL FALLBACK
// //     img_mpmc_queue_t global_queue;

// // } img_scheduler_t;

// // // lifecycle
// // int img_scheduler_init(img_scheduler_t *sched, uint32_t workers);

// // void img_scheduler_destroy(img_scheduler_t *sched);

// // // submit
// // int img_scheduler_submit(img_scheduler_t *sched, img_task_t *task);

// // // steal
// // img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id);

// // #endif
