/* pipeline/scheduler.c */

#include "runtime/worker.h"
#include "runtime/task.h"

static uint32_t rr_counter = 0;

// img_worker_t *img_scheduler_pick_worker(img_engine_t *engine)
// {
//     uint32_t idx = __atomic_fetch_add(&rr_counter, 1, __ATOMIC_RELAXED);
//     return &engine->workers[idx % engine->worker_count];
// }

img_worker_t *img_scheduler_pick_worker(img_engine_t *engine, uint32_t idx)
{
    return &engine->workers[idx % engine->worker_count];
}

// img_task_t *task = malloc(sizeof(img_task_t));

// task->input = input;
// task->input_size = input_size;
// task->pipeline = pipeline;
// task->output = NULL;
// task->output_size = 0;

// img_worker_t *w = img_scheduler_pick_worker(engine);

// // 🔥 BACKPRESSURE CONTROL
// if (!img_queue_push(w->queue, task))
// {
//     img_metrics_inc(&g_metrics.backpressure_drops);
//     return IMG_ERR_IO;
// }

// // 🔥 WAIT (simple version, future = async/futures)
// while (task->status == 0)
// {
//     // spin or condition variable (future improvement)
// }

// *output = task->output;
// *output_size = task->output_size;

// free(task);
// return IMG_SUCCESS;
