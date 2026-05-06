// ./include/api/api_internal.h
#ifndef IMGENGINE_API_INTERNAL_H
#define IMGENGINE_API_INTERNAL_H

#include "api/v1/img_error.h"
#include "core/context_internal.h"
#include "pipeline/job.h"
#include "runtime/worker.h"
#include "runtime/queue_mpmc.h"

#include <stddef.h>

typedef struct img_buffer img_buffer_t;
typedef struct img_pipeline_desc img_pipeline_desc_t;

typedef struct {
    img_engine_t *engine;
    img_ctx_t ctx;
} img_prepared_decoder_t;

extern img_engine_t g_engine;
extern img_worker_t g_workers[64];
extern img_ctx_t g_worker_ctxs[64];
extern img_mpmc_queue_t g_task_queue;

void img_api_make_ctx(const img_engine_t *engine, img_ctx_t *ctx);
void img_api_release_thread_ctx(void);
img_result_t img_api_resolve_template_job(const img_engine_t *engine,
                                          img_job_template_t template_id, img_job_t *job_out);
img_result_t img_api_prepare_decoder(img_engine_t *engine, img_prepared_decoder_t *state);
img_result_t img_api_decode_prepared(img_prepared_decoder_t *state, const uint8_t *input,
                                     size_t input_size, img_buffer_t *out_buf);
void img_api_decoder_destroy(img_prepared_decoder_t *state);
img_result_t img_api_process_fast_run(img_engine_t *engine, img_pipeline_desc_t *pipe,
                                      img_buffer_t *out_buf);

#endif /* IMGENGINE_API_INTERNAL_H */
