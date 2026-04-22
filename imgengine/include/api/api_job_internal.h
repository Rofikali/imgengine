// ./include/api/api_job_internal.h
#ifndef IMGENGINE_API_JOB_INTERNAL_H
#define IMGENGINE_API_JOB_INTERNAL_H

#include "api/api_internal.h"
#include "core/buffer.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include "pipeline/canvas.h"
#include "pipeline/job.h"
#include "pipeline/layout.h"

#include <stddef.h>
#include <stdint.h>

img_result_t img_api_release_input(uint8_t *file_data, size_t file_size);

img_result_t img_api_init_ctx(img_engine_t *engine, img_ctx_t *ctx);

img_result_t img_api_load_input(
    const char *input_path,
    uint8_t **file_data,
    size_t *file_size);

img_result_t img_api_decode_input(
    img_ctx_t *ctx,
    uint8_t **file_data,
    size_t file_size,
    img_buffer_t *photo);

img_result_t img_api_prepare_render_stage(
    img_engine_t *engine,
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    img_layout_t *layout,
    const img_job_t *job,
    const img_buffer_t *photo,
    img_arena_t **arena);

img_result_t img_api_finish_job_stage(
    img_engine_t *engine,
    img_canvas_t *canvas,
    img_arena_t *arena);

img_result_t img_apply_job_postfx(
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job);

img_result_t img_write_buffer_file(
    const char *output_path,
    const uint8_t *data,
    size_t size);

img_result_t img_api_render_job_output(
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job,
    uint8_t **out,
    size_t *out_size);

img_result_t img_api_finish_job_output(
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job,
    const char *output_path);

void img_encoded_free(uint8_t *ptr);

img_result_t img_api_begin_job(
    img_engine_t *engine,
    const char *input_path,
    const img_job_t *job,
    img_ctx_t *ctx,
    img_canvas_t *canvas,
    img_layout_t *layout,
    img_arena_t **arena);

img_result_t img_api_run_job_impl(
    img_engine_t *engine,
    const char *input_path,
    const char *output_path,
    const img_job_t *job);

img_result_t img_api_run_job_raw_impl(
    img_engine_t *engine,
    const char *input_path,
    const img_job_t *job,
    uint8_t **output,
    size_t *output_size);

#endif /* IMGENGINE_API_JOB_INTERNAL_H */
