// ./src/api/api_job_run_rgb24.c
#include "api/api_job_internal.h"
#include "security/input_validator.h"

#include <stdint.h>

static img_result_t img_api_validate_rgb24_input(const uint8_t *input, size_t input_size,
                                                 uint32_t width, uint32_t height,
                                                 uint32_t *stride_io) {
    if (!input || !stride_io || width == 0 || height == 0)
        return IMG_ERR_SECURITY;

    uint64_t min_stride = (uint64_t)width * 3ULL;
    uint64_t stride = (*stride_io == 0) ? min_stride : (uint64_t)*stride_io;
    if (stride < min_stride)
        return IMG_ERR_SECURITY;

    uint64_t required = stride * (uint64_t)height;
    if (required == 0 || required > SIZE_MAX || input_size != (size_t)required)
        return IMG_ERR_SECURITY;

    img_result_t sec = img_security_validate_request(width, height, input_size);
    if (sec != IMG_SUCCESS)
        return sec;

    *stride_io = (uint32_t)stride;
    return IMG_SUCCESS;
}

img_result_t img_api_run_job_rgb24_impl(img_engine_t *engine, const uint8_t *input,
                                        size_t input_size, uint32_t width, uint32_t height,
                                        uint32_t stride, const char *output_path,
                                        const img_job_t *job) {
    if (!engine || !output_path || !job)
        return IMG_ERR_SECURITY;
    if (job->abi_version != IMG_JOB_ABI_VERSION)
        return IMG_ERR_INTERNAL;

    img_result_t sec = img_api_validate_rgb24_input(input, input_size, width, height, &stride);
    if (sec != IMG_SUCCESS)
        return sec;

    img_ctx_t ctx = {0};
    img_canvas_t canvas = {0};
    img_layout_t layout = {0};
    img_arena_t *arena = NULL;
    img_buffer_t photo = {
        .data = (uint8_t *)input,
        .owner_pool = NULL,
        .width = width,
        .height = height,
        .channels = 3,
        .stride = stride,
    };

    img_result_t r = img_api_init_ctx(engine, &ctx);
    if (r != IMG_SUCCESS)
        return r;

    r = img_api_begin_job_with_photo(engine, job, &photo, &ctx, &canvas, &layout, &arena);
    if (r != IMG_SUCCESS) {
        img_api_finish_job_stage(engine, &canvas, arena);
        return r;
    }

    r = img_api_finish_job_output(&ctx, &canvas, &layout, job, output_path);
    img_api_finish_job_stage(engine, &canvas, arena);
    return r;
}

img_result_t img_api_run_job_rgb24_raw_impl(img_engine_t *engine, const uint8_t *input,
                                            size_t input_size, uint32_t width, uint32_t height,
                                            uint32_t stride, const img_job_t *job, uint8_t **output,
                                            size_t *output_size) {
    if (!engine || !job || !output || !output_size)
        return IMG_ERR_SECURITY;
    if (job->abi_version != IMG_JOB_ABI_VERSION)
        return IMG_ERR_INTERNAL;

    img_result_t sec = img_api_validate_rgb24_input(input, input_size, width, height, &stride);
    if (sec != IMG_SUCCESS)
        return sec;

    img_ctx_t ctx = {0};
    img_canvas_t canvas = {0};
    img_layout_t layout = {0};
    img_arena_t *arena = NULL;
    img_buffer_t photo = {
        .data = (uint8_t *)input,
        .owner_pool = NULL,
        .width = width,
        .height = height,
        .channels = 3,
        .stride = stride,
    };

    img_result_t r = img_api_init_ctx(engine, &ctx);
    if (r != IMG_SUCCESS)
        return r;

    r = img_api_begin_job_with_photo(engine, job, &photo, &ctx, &canvas, &layout, &arena);
    if (r != IMG_SUCCESS) {
        img_api_finish_job_stage(engine, &canvas, arena);
        return r;
    }

    r = img_api_render_job_output(&ctx, &canvas, &layout, job, output, output_size);
    img_api_finish_job_stage(engine, &canvas, arena);
    return r;
}
