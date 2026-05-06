// ./include/runtime/scheduler.h

// /* pipeline/scheduler.h */

#ifndef IMGENGINE_RUNTIME_SCHEDULER_H
#define IMGENGINE_RUNTIME_SCHEDULER_H

#include <sched.h>
#include <stdint.h>

#include "core/result.h"
#include "runtime/task.h"
#include "runtime/queue_spsc.h"
#include "runtime/queue_mpmc.h"

typedef struct img_worker img_worker_t;

/*
 * 🔥 REAL STRUCT (NOT ANONYMOUS!)
 */
typedef struct img_scheduler {
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
int img_runtime_submit_task(img_engine_t *engine, img_task_t *task);
img_result_t img_runtime_wait_task(img_task_t *task);

/*
 * Steal
 */
img_task_t *img_scheduler_steal(img_scheduler_t *sched, uint32_t self_id);

#endif
