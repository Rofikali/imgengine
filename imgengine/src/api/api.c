// src/api/api.c

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"

#include "core/engine/engine.h"
#include "core/context/ctx_internal.h"
#include "core/pipeline/pipeline_types.h"

#include "pipeline/exec/pipeline_exec.h"
#include "pipeline/dispatch/jump_table.h"

#include "memory/slab/slab.h"

#include "runtime/worker/worker.h"
#include "runtime/scheduler/scheduler.h"

#include "security/sandbox/sandbox.h"
#include "security/validation/input_validator.h"

#include "observability/metrics/metrics.h"
#include "observability/binlog/binlog_fast.h"

#include <stdlib.h>
#include <string.h>

// ============================================================
// 🔥 INTERNAL ENGINE STRUCT (OPAQUE)
// ============================================================

struct img_engine
{
    uint32_t worker_count;

    img_worker_t *workers;
    img_scheduler_t scheduler;

    img_slab_pool_t *global_pool;

    cpu_caps_t caps;
};

// ============================================================
// 🔥 INIT
// ============================================================

img_engine_t *img_api_init(uint32_t workers)
{
    if (workers == 0 || workers > 64)
        return NULL;

    // 🔥 sandbox FIRST (critical)
    if (!img_security_enter_sandbox())
        return NULL;

    img_engine_t *engine = calloc(1, sizeof(*engine));
    if (!engine)
        return NULL;

    // ================= CPU =================
    engine->caps = img_cpu_detect_caps();

    // ================= MEMORY =================
    engine->global_pool = img_slab_create(
        64 * 1024 * 1024,
        64 * 1024);

    if (!engine->global_pool)
        goto fail;

    // ================= DISPATCH =================
    img_jump_table_init(engine->caps);

    // ================= SCHEDULER =================
    if (img_scheduler_init(&engine->scheduler, workers) != 0)
        goto fail;

    engine->worker_count = workers;
    engine->workers = engine->scheduler.workers;

    // ================= OBSERVABILITY =================
    img_metrics_init();

    return engine;

fail:
    if (engine->global_pool)
        img_slab_destroy(engine->global_pool);

    free(engine);
    return NULL;
}

void img_api_shutdown(img_engine_t *engine)
{
    if (!engine)
        return;

    img_scheduler_destroy(&engine->scheduler);

    if (engine->global_pool)
        img_slab_destroy(engine->global_pool);

    free(engine);
}

int img_api_process(
    img_engine_t *engine,
    img_buffer_t *input,
    img_buffer_t **output)
{
    if (!engine || !input || !output)
        return IMG_ERR_SECURITY;

    img_ctx_t ctx = {0};
    ctx.caps = engine->caps;
    ctx.thread_id = 0;

    // 🔥 direct execution (zero-copy)
    img_pipeline_runtime_t runtime = {0}; // placeholder (compiled pipeline)

    img_pipeline_execute_hot(
        &ctx,
        &runtime,
        input);

    *output = input;

    return IMG_SUCCESS;
}

extern img_result_t decode_image_secure(
    const uint8_t *input,
    size_t size,
    img_buffer_t *out);

int img_api_process_raw(
    img_engine_t *engine,
    uint8_t *input,
    size_t input_size,
    uint8_t **output,
    size_t *output_size)
{
    if (!engine || !input || !output || !output_size)
        return IMG_ERR_SECURITY;

    // ================= SECURITY =================
    img_result_t sec =
        img_security_validate_request(4096, 4096, input_size);

    if (sec != IMG_SUCCESS)
        return sec;

    // ================= DECODE =================
    img_buffer_t buf = {0};

    img_result_t res =
        decode_image_secure(input, input_size, &buf);

    if (res != IMG_SUCCESS)
        return res;

    // ================= EXECUTION =================
    img_ctx_t ctx = {0};
    ctx.caps = engine->caps;

    img_pipeline_runtime_t runtime = {0};

    uint64_t start = img_profiler_now();

    img_pipeline_execute_hot(&ctx, &runtime, &buf);

    uint64_t end = img_profiler_now();

    // ================= METRICS =================
    img_metrics_inc(0);
    img_metrics_latency(0, end - start);

    IMG_LOG_LATENCY(end - start, 1, 0);

    // ================= OUTPUT =================
    *output = buf.data;
    *output_size = buf.height * buf.stride;

    return IMG_SUCCESS;
}