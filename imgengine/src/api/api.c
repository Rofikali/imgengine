// src/api/api.c

#include "api/v1/img_api.h"
// #include "api/v1/img_plugin_api.h"

#include "core/context_internal.h"
#include "runtime/worker.h"
#include "runtime/task.h"
#include "runtime/queue_mpmc.h"

#include "memory/slab.h"
#include "memory/arena.h"

#include "arch/cpu_caps.h"

#include "pipeline/jump_table.h"
#include "pipeline/pipeline_compiled.h"

#include "hot/pipeline_exec.h"

#include "security/input_validator.h"
#include "security/sandbox.h"

#include "cold/validation.h"

#include <string.h>
#include <stdlib.h>

// 🔥 MISSING DECLARATION FIX
void img_hw_register_kernels(cpu_caps_t caps);

// ============================================================
// 🔥 GLOBAL ENGINE
// ============================================================

static img_mpmc_queue_t g_task_queue;
static img_engine_t g_engine;
static img_worker_t g_workers[64];

// ============================================================
// 🔥 FORWARD
// ============================================================

extern img_result_t decode_image_secure(
    const uint8_t *input,
    size_t size,
    img_buffer_t *out_buf);

// ============================================================
// 🔥 FAST PATH
// ============================================================

img_result_t img_api_process_fast(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_pipeline_desc_t *pipe,
    img_buffer_t *out_buf)
{
    if (!engine || !input || !pipe || !out_buf)
        return IMG_ERR_SECURITY;

    img_result_t sec = img_security_validate_request(
        4096, 4096,
        input_size);

    if (sec != IMG_SUCCESS)
        return sec;

    img_result_t res = decode_image_secure(
        input,
        input_size,
        out_buf);

    if (res != IMG_SUCCESS)
        return res;

    if (!img_validate_pipeline_safety(pipe))
        return IMG_ERR_SECURITY;

    img_pipeline_compiled_t compiled;

    if (img_pipeline_compile(pipe, &compiled) != 0)
        return IMG_ERR_FORMAT;

    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = engine->caps;

    img_pipeline_execute_hot(
        &ctx,
        (img_pipeline_runtime_t *)&compiled,
        out_buf);

    return IMG_SUCCESS;
}

// ============================================================
// 🔥 INIT
// ============================================================

img_engine_t *img_api_init(uint32_t workers)
{
    if (!img_security_enter_sandbox())
        return NULL;

    if (workers == 0 || workers > 64)
        return NULL;

    memset(&g_engine, 0, sizeof(g_engine));

    g_engine.worker_count = workers;
    g_engine.workers = g_workers;

    // 🔥 CPU DETECT
    g_engine.caps = img_cpu_detect_caps();

    // 🔥 GLOBAL SLAB
    g_engine.global_pool = img_slab_create(
        64 * 1024 * 1024,
        64 * 1024);

    if (!g_engine.global_pool)
        return NULL;

    // 🔥 DISPATCH INIT
    img_jump_table_init(g_engine.caps);
    img_hw_register_kernels(g_engine.caps);
    /* 🔥 ZERO-COST PLUGIN LOAD */
    // img_plugins_init_all();

    // 🔥 WORKERS INIT (SPSC OWNED)
    for (uint32_t i = 0; i < workers; i++)
    {
        img_worker_init(&g_workers[i], i);
    }

    // 🔥 OPTIONAL GLOBAL QUEUE (for external submissions only)
    img_mpmc_init(&g_task_queue, 1024);

    return &g_engine;
}

// ============================================================
// 🔥 TASK SUBMIT
// ============================================================

int img_submit_task(img_task_t *task)
{
    if (!task)
        return 0;

    return img_mpmc_push(&g_task_queue, task);
}

// ============================================================
// 🔥 SHUTDOWN
// ============================================================

void img_api_shutdown(img_engine_t *engine)
{
    if (!engine)
        return;

    for (uint32_t i = 0; i < engine->worker_count; i++)
    {
        img_worker_stop(&engine->workers[i]);
        img_worker_join(&engine->workers[i]);
    }

    if (engine->global_pool)
        img_slab_destroy(engine->global_pool);
}

// ============================================================
// 🔥 FREE
// ============================================================

void img_encoded_free(uint8_t *ptr)
{
    if (ptr)
        free(ptr);
}

// #include "api/v1/img_api.h"

// #include "core/context_internal.h"
// #include "runtime/worker.h"
// #include "runtime/task.h"
// #include "runtime/queue_mpmc.h"

// #include "memory/slab.h"
// #include "memory/arena.h"

// #include "arch/cpu_caps.h"

// #include "pipeline/jump_table.h"
// #include "pipeline/pipeline_compiled.h"

// #include "hot/pipeline_exec.h"

// #include "security/input_validator.h"
// #include "security/sandbox.h"

// #include "cold/validation.h"

// #include <string.h>
// #include <stdlib.h>

// // ============================================================
// // 🔥 GLOBAL ENGINE (STATIC, NO HEAP FRAGMENTATION)
// // ============================================================

// static img_mpmc_queue_t g_task_queue;
// static img_engine_t g_engine;
// static img_worker_t g_workers[64];

// // ============================================================
// // 🔥 FORWARD (decoder must use bounds_check internally)
// // ============================================================

// extern img_result_t decode_image_secure(
//     const uint8_t *input,
//     size_t size,
//     img_buffer_t *out_buf);

// // ============================================================
// // 🔥 MAIN FAST PATH (SECURE + DETERMINISTIC)
// // ============================================================

// img_result_t img_api_process_fast(
//     img_engine_t *engine,
//     const uint8_t *input,
//     size_t input_size,
//     img_pipeline_desc_t *pipe,
//     img_buffer_t *out_buf)
// {
//     if (!engine || !input || !pipe || !out_buf)
//         return IMG_ERR_SECURITY;

//     // ========================================================
//     // 🔥 1. REQUEST VALIDATION (FAIL FAST)
//     // ========================================================

//     img_result_t sec = img_security_validate_request(
//         4096, 4096, // 🔥 Replace with parsed header later
//         input_size);

//     if (sec != IMG_SUCCESS)
//         return sec;

//     // ========================================================
//     // 🔥 2. DECODE (BOUNDS-CHECKED INSIDE DECODER)
//     // ========================================================

//     img_result_t res = decode_image_secure(
//         input,
//         input_size,
//         out_buf);

//     if (res != IMG_SUCCESS)
//         return res;

//     // ========================================================
//     // 🔥 3. PIPELINE VALIDATION (COLD PATH)
//     // ========================================================

//     if (!img_validate_pipeline_safety(pipe))
//         return IMG_ERR_SECURITY;

//     // ========================================================
//     // 🔥 4. PIPELINE COMPILE (COLD → HOT TRANSITION)
//     // ========================================================

//     img_pipeline_compiled_t compiled;

//     if (img_pipeline_compile(pipe, &compiled) != 0)
//         return IMG_ERR_FORMAT;

//     // ========================================================
//     // 🔥 5. EXECUTION (HOT PATH — ZERO CHECKS)
//     // ========================================================

//     img_ctx_t ctx = {0}; // 🔥 stack ctx (no alloc)
//     ctx.thread_id = 0;
//     ctx.caps = engine->caps;

//     img_pipeline_execute_hot(
//         &ctx,
//         (img_pipeline_runtime_t *)&compiled,
//         out_buf);

//     return IMG_SUCCESS;
// }

// // ============================================================
// // 🔥 ENGINE INIT (WITH SANDBOX)
// // ============================================================

// img_engine_t *img_api_init(uint32_t workers)
// {
//     // ========================================================
//     // 🔥 0. ENTER SANDBOX FIRST (CRITICAL)
//     // ========================================================

//     if (!img_security_enter_sandbox())
//         return NULL;

//     // ========================================================
//     // 🔥 QUEUE INIT
//     // ========================================================

//     img_mpmc_init(&g_task_queue, 1024);

//     if (workers == 0 || workers > 64)
//         return NULL;

//     memset(&g_engine, 0, sizeof(g_engine));

//     g_engine.worker_count = workers;
//     g_engine.workers = g_workers;

//     // ========================================================
//     // 🔥 CPU DETECT
//     // ========================================================

//     g_engine.caps = img_cpu_detect_caps();

//     // ========================================================
//     // 🔥 GLOBAL SLAB (ONLY SAFE PLACE FOR ALLOCATION)
//     // ========================================================

//     g_engine.global_pool = img_slab_create(
//         64 * 1024 * 1024,
//         64 * 1024);

//     if (!g_engine.global_pool)
//         return NULL;

//     // ========================================================
//     // 🔥 DISPATCH INIT
//     // ========================================================

//     img_jump_table_init(g_engine.caps);
//     img_hw_register_kernels(g_engine.caps);

//     // ========================================================
//     // 🔥 WORKERS INIT
//     // ========================================================

//     for (uint32_t i = 0; i < workers; i++)
//     {
//         img_worker_init(&g_workers[i], i);
//         g_workers[i].queue = &g_task_queue;
//     }

//     return &g_engine;
// }

// // ============================================================
// // 🔥 TASK SUBMISSION (MPMC SAFE)
// // ============================================================

// int img_submit_task(img_task_t *task)
// {
//     if (!task)
//         return 0;

//     return img_mpmc_push(&g_task_queue, task);
// }

// // ============================================================
// // 🔥 CLEANUP
// // ============================================================

// void img_api_shutdown(img_engine_t *engine)
// {
//     if (!engine)
//         return;

//     for (uint32_t i = 0; i < engine->worker_count; i++)
//     {
//         img_worker_stop(&engine->workers[i]);
//         img_worker_join(&engine->workers[i]);
//     }

//     if (engine->global_pool)
//         img_slab_destroy(engine->global_pool);
// }

// // ============================================================
// // 🔥 OUTPUT FREE (BOUNDARY SAFE)
// // ============================================================

// void img_encoded_free(uint8_t *ptr)
// {
//     if (ptr)
//         free(ptr); // or tjFree if TurboJPEG
// }
