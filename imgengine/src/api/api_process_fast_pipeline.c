// ./src/api/api_process_fast_pipeline.c
#include "api/api_internal.h"
#include "runtime/job_exec.h"

img_result_t img_api_process_fast_run(img_engine_t *engine, img_pipeline_desc_t *pipe,
                                      img_buffer_t *out_buf) {
    img_ctx_t ctx = {0};

    img_api_make_ctx(engine, &ctx);
    return img_runtime_run_compiled_pipeline(engine, &ctx, pipe, out_buf);
}
