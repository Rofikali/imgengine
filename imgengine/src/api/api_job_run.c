
// // src/api/api_job_run.c  (UPDATE — add task submission)

// #define _GNU_SOURCE

// #include "api/api_job_internal.h"
// #include "api/api_internal.h"
// #include "runtime/scheduler.h"
// #include "runtime/task.h"
// #include "core/result.h"
// #include <stdatomic.h>
// #include <sched.h>

// img_result_t img_api_run_job(
//     img_engine_t *engine,
//     const char *input_path,
//     const char *output_path,
//     const img_job_t *job)
// {
//     if (!engine || !input_path || !output_path || !job)
//         return IMG_ERR_SECURITY;

//     /*
//      * PHASE 1: Load and decode input (cold path — I/O)
//      * This runs on the calling thread. Decode is I/O-bound
//      * and not suitable for the worker queue.
//      */
//     img_result_t r = img_api_job_load_input(engine, input_path, job);
//     if (r != IMG_SUCCESS)
//         return r;

//     /*
//      * PHASE 2: Submit pipeline task to worker queue
//      *
//      * The worker pulls this task and calls img_pipeline_execute_hot()
//      * which dispatches through the jump table → AVX2 kernels.
//      *
//      * For single-image jobs: one task, one worker picks it up.
//      * For batch: multiple tasks submitted, workers process in parallel.
//      *
//      * img_submit_task() pushes to g_task_queue (MPMC).
//      * Worker loops pop from their per-worker SPSC queue or steal.
//      */
//     img_task_t task = {
//         .pipeline = engine->compiled_pipeline,
//         .buffer = engine->input_buf,
//         .status = -1,
//     };

//     if (engine->worker_count > 0 && engine->scheduler)
//     {
//         /*
//          * Async path: submit to worker, spin-wait for completion.
//          * The spin-wait is intentional for low-latency single jobs.
//          * For throughput workloads, use img_api_submit_async() instead.
//          */
//         r = img_submit_task(engine, &task);
//         if (r != IMG_SUCCESS)
//             return r;

//         /* Spin-wait for worker to complete the task */
//         uint32_t spin = 0;
//         while (__builtin_expect(task.status == -1, 1))
//         {
//             if (spin++ < 1000)
//                 __asm__ volatile("pause" ::: "memory");
//             else
//                 sched_yield();
//         }

//         if (task.status != 0)
//             return IMG_ERR_INTERNAL;
//     }
//     else
//     {
//         /*
//          * Synchronous fallback: no workers initialized.
//          * Runs pipeline on calling thread.
//          * Used in single-threaded mode (--threads 1 without workers).
//          */
//         img_pipeline_execute_hot(
//             engine->ctx,
//             engine->compiled_pipeline,
//             engine->input_buf);
//     }

//     /*
//      * PHASE 3: Encode and write output (cold path — I/O)
//      * Runs on calling thread after pipeline completes.
//      */
//     return img_api_job_finish_output(engine, output_path, job);
// }

// ./src/api/api_job_run.c
#include "api/api_job_internal.h"

#include <stdio.h>

img_result_t img_api_run_job_impl(
    img_engine_t *engine,
    const char *input_path,
    const char *output_path,
    const img_job_t *job)
{
    if (!engine || !input_path || !output_path || !job)
        return IMG_ERR_SECURITY;

    if (job->abi_version != IMG_JOB_ABI_VERSION)
        return IMG_ERR_INTERNAL;

    img_ctx_t ctx = {0};
    img_canvas_t canvas = {0};
    img_layout_t layout = {0};
    img_arena_t *arena = NULL;

    img_result_t r = img_api_begin_job(
        engine,
        input_path,
        job,
        &ctx,
        &canvas,
        &layout,
        &arena);
    if (r != IMG_SUCCESS)
    {
        img_api_finish_job_stage(engine, &canvas, arena);
        return r;
    }

    r = img_api_finish_job_output(
        &ctx,
        &canvas,
        &layout,
        job,
        output_path);

    if (r != IMG_SUCCESS)
        fprintf(stderr, "[JOB] output stage failed: %s\n", img_result_name(r));

    img_api_finish_job_stage(engine, &canvas, arena);
    return r;
}
