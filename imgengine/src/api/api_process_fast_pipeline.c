// ./src/api/api_process_fast_pipeline.c
#include "api/api_internal.h"
#include "runtime/job_exec.h"

img_result_t img_api_process_fast_run(
    img_engine_t *engine,
    img_pipeline_desc_t *pipe,
    img_buffer_t *out_buf)
{
    return img_runtime_run_compiled_pipeline(engine, pipe, out_buf);
}
