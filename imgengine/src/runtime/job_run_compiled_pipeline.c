// ./src/runtime/job_run_compiled_pipeline.c
#include "runtime/job_exec.h"

#include "core/context_internal.h"
#include "cold/validation.h"
#include "hot/pipeline_exec.h"
#include "pipeline/pipeline_compiled.h"

img_result_t img_runtime_run_compiled_pipeline(img_engine_t *engine, img_ctx_t *ctx,
                                               img_pipeline_desc_t *pipe, img_buffer_t *out_buf) {
    if (!engine || !ctx || !pipe || !out_buf)
        return IMG_ERR_SECURITY;

    if (!img_validate_pipeline_safety(pipe))
        return IMG_ERR_SECURITY;

    img_pipeline_compiled_t compiled;
    if (img_pipeline_compile(pipe, &compiled) != 0)
        return IMG_ERR_FORMAT;

    img_pipeline_execute_compiled_hot(ctx, &compiled, out_buf);
    return IMG_SUCCESS;
}
