/* runtime/worker.c */

#include "runtime/worker.h"
#include "pipeline/engine.h"
#include "observability/metrics.h"
#include "observability/profiler.h"
#include "runtime/worker.h"
#include "runtime/affinity.h"
#include "memory/numa.h"

static void *worker_loop(void *arg)
{
    img_worker_t *w = (img_worker_t *)arg;

    while (w->running)
    {
        // 🔥 LOCK-FREE POP (correct API)
        img_task_t *task = (img_task_t *)img_queue_pop(w->queue);

        if (!task)
            continue;

        uint64_t start = img_profiler_now();

        // 🔥 EXECUTE PIPELINE (FIXED SIGNATURE)
        // img_pipeline_execute(
        //     &w->ctx,
        //     task->pipeline,
        //     &task->input_buf);

        img_pipeline_execute(
            &w->ctx,
            task->pipeline,
            &task->buffer);

        uint64_t end = img_profiler_now();

        img_metrics_inc(&g_metrics.total_requests);
        img_metrics_observe_latency(end - start);
    }

    return NULL;
}

int img_worker_init(img_worker_t *w, uint32_t id)
{
    w->id = id;
    w->running = 1;

    // 🔥 Use stable APIs (NUMA handled internally)
    w->slab = img_slab_create(64 * 1024 * 1024, 256 * 1024);
    w->arena = img_arena_create(1024 * 1024);

    if (!w->slab || !w->arena)
        return -1;

    w->queue = img_queue_create(1024);

    // 🔥 Context setup
    w->ctx.thread_id = id;
    w->ctx.local_pool = w->slab;
    w->ctx.scratch_arena = w->arena;
    w->ctx.caps = img_cpu_detect_caps();

    pthread_create(&w->thread, NULL, worker_loop, w);

    // Optional (only if implemented)
    img_set_thread_affinity(w->thread, id);

    return 0;
}

// int img_worker_init(img_worker_t *w, uint32_t id)
// {
//     w->id = id;
//     w->running = 1;

//     int node = img_numa_get_node();

//     w->slab = img_slab_create_on_node(node);
//     w->arena = img_arena_create_on_node(node);

//     w->queue = img_queue_create(1024);

//     // 🔥 Context setup
//     w->ctx.thread_id = id;
//     w->ctx.local_pool = w->slab;
//     w->ctx.scratch_arena = w->arena;
//     w->ctx.caps = img_cpu_detect_caps();

//     pthread_create(&w->thread, NULL, worker_loop, w);
//     img_set_thread_affinity(w->thread, id);

//     return 0;
// }

// // static void *worker_loop(void *arg)
// // {
// //     img_worker_t *w = (img_worker_t *)arg;

// //     while (w->running)
// //     {
// //         img_task_t *task = NULL;

// //         // 🔥 LOCK-FREE POP
// //         if (!img_queue_pop(w->queue, (void **)&task))
// //             continue;

// //         uint64_t start = img_profiler_now();

// //         // 🔥 EXECUTE PIPELINE
// //         task->status = img_pipeline_execute(
// //             &w->ctx,
// //             task->input,
// //             task->input_size,
// //             task->pipeline,
// //             &task->output,
// //             &task->output_size);

// //         uint64_t end = img_profiler_now();

// //         img_metrics_inc(&g_metrics.total_requests);
// //         img_metrics_observe_latency(end - start);
// //     }

// //     return NULL;
// // }

// // int img_worker_init(img_worker_t *w, uint32_t id)
// // {
// //     w->id = id;
// //     w->running = 1;

// //     // 🔥 NUMA-aware allocation
// //     int node = img_numa_get_node(id);

// //     w->slab = img_slab_create_on_node(node);
// //     w->arena = img_arena_create_on_node(node);

// //     w->queue = img_queue_create(1024);

// //     // 🔥 Setup context
// //     w->ctx.thread_id = id;
// //     w->ctx.local_pool = w->slab;
// //     w->ctx.scratch_arena = w->arena;

// //     // 🔥 Pin thread
// //     pthread_create(&w->thread, NULL, worker_loop, w);
// //     img_set_thread_affinity(w->thread, id);

// //     return 0;
// // }
