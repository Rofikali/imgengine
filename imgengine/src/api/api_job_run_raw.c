// ./src/api/api_job_run_raw.c
#include "api/api_job_internal.h"

img_result_t img_api_run_job_raw_impl(img_engine_t *engine, const char *input_path,
                                      const img_job_t *job, uint8_t **output, size_t *output_size) {
    if (!engine || !input_path || !job || !output || !output_size)
        return IMG_ERR_SECURITY;

    if (job->abi_version != IMG_JOB_ABI_VERSION)
        return IMG_ERR_INTERNAL;

    img_ctx_t ctx = {0};
    img_canvas_t canvas = {0};
    img_layout_t layout = {0};
    img_arena_t *arena = NULL;

    img_result_t r = img_api_begin_job(engine, input_path, job, &ctx, &canvas, &layout, &arena);
    if (r != IMG_SUCCESS) {
        img_api_finish_job_stage(engine, &canvas, arena);
        return r;
    }

    r = img_api_render_job_output(&ctx, &canvas, &layout, job, output, output_size);

    img_api_finish_job_stage(engine, &canvas, arena);
    return r;
}
