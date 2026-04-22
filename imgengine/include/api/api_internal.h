// ./include/api/api_internal.h
#ifndef IMGENGINE_API_INTERNAL_H
#define IMGENGINE_API_INTERNAL_H

#include "api/v1/img_error.h"
#include "core/context_internal.h"
#include "runtime/worker.h"
#include "runtime/queue_mpmc.h"

typedef struct img_buffer img_buffer_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

extern img_engine_t g_engine;
extern img_worker_t g_workers[64];
extern img_ctx_t g_worker_ctxs[64];
extern img_mpmc_queue_t g_task_queue;

void img_api_make_ctx(const img_engine_t *engine, img_ctx_t *ctx);
img_result_t img_api_process_fast_run(
    img_engine_t *engine,
    img_pipeline_desc_t *pipe,
    img_buffer_t *out_buf);

#endif /* IMGENGINE_API_INTERNAL_H */
