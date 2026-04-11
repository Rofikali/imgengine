// ./src/api/api.c

#include "api/v1/img_api.h"
#include "api/v1/img_buffer_utils.h"

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

#include "io/decoder/decoder_entry.h"
#include "io/encoder/encoder_entry.h"

#include "io/decoder/decoder_entry.h"
#include "io/encoder/encoder_entry.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

/*
 * decode_image_secure()
 *
 * Internal bridge to the hardened JPEG decoder.
 * Returns img_result_t — never raw -1.
 */
img_result_t decode_image_secure(
    const uint8_t *input,
    size_t size,
    img_buffer_t *out_buf)
{
    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = g_engine.caps;
    ctx.local_pool = g_engine.global_pool;

    int rc = img_decode_to_buffer(&ctx, input, size, out_buf);

    /*
     * img_decode_to_buffer returns img_result_t cast to int.
     * Cast back — no information lost.
     */
    return (img_result_t)rc;
}

/*
 * img_api_process_raw()
 *
 * Full pipeline: decode → (no-op pipeline) → encode.
 * Every error path returns a named code with a log line.
 */
img_result_t img_api_process_raw(
    img_engine_t *engine,
    uint8_t *input,
    size_t input_size,
    uint8_t **output,
    size_t *output_size)
{
    if (!engine || !input || !output || !output_size)
        return IMG_ERR_SECURITY;

    /* Security gate — validate dimensions and size before touching data */
    img_result_t sec = img_security_validate_request(4096, 4096, input_size);
    if (sec != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] security validation failed: %d\n", sec);
        return sec;
    }

    /* Decode */
    img_buffer_t buf = {0};

    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = engine->caps;
    ctx.local_pool = engine->global_pool;

    img_result_t dec = (img_result_t)img_decode_to_buffer(
        &ctx, input, input_size, &buf);

    if (dec != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] decode failed: %d (%s)\n",
                dec, img_result_name(dec));
        return dec;
    }

    /* Encode */
    img_result_t enc = (img_result_t)img_encode_from_buffer(
        &ctx, &buf, output, output_size);

    img_slab_free(engine->global_pool, buf.data);

    if (enc != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] encode failed: %d (%s)\n",
                enc, img_result_name(enc));
        return enc;
    }

    return IMG_SUCCESS;
}

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
        512 * 1024 * 1024, // 512MB total pool (supports ~16 concurrent 4K images)
        32 * 1024 * 1024); // 32MB per block  (fits 4K RGB: 3840*2160*3 = 24MB)

    // g_engine.global_pool = img_slab_create(
    //     64 * 1024 * 1024,
    //     64 * 1024);

    if (!g_engine.global_pool)
        return NULL;

    // 🔥 DISPATCH INIT
    img_jump_table_init(g_engine.caps);
    img_hw_register_kernels(g_engine.caps);
    static img_ctx_t g_worker_ctxs[64];
    /* 🔥 ZERO-COST PLUGIN LOAD */
    // img_plugins_init_all();

    for (uint32_t i = 0; i < workers; i++)
    {
        g_worker_ctxs[i].thread_id = i;
        g_worker_ctxs[i].caps = g_engine.caps;
        g_worker_ctxs[i].local_pool = g_engine.global_pool;

        img_worker_init(
            &g_workers[i],
            i,
            NULL,
            &g_worker_ctxs[i]);
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
