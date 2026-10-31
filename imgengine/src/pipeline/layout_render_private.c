#include "pipeline/layout_render_private.h"
#include "pipeline/layout_crop_geometry.h"

#include "imgengine/api/v2/layout.h"
#include "api/api_internal.h"
#include "api/v1/img_api.h"
#include "io/io_vtable.h"
#include "io/encoder/encoder_entry.h"
#include "memory/slab.h"
#include "pipeline/canvas_internal.h"
#include "pipeline/layout_resize_internal.h"
#include "runtime/job_exec.h"
#include "runtime/scheduler.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

extern void img_blit_avx2(img_buffer_t *dst, const img_buffer_t *src, uint32_t dst_x,
                          uint32_t dst_y);

struct img_layout_render_v1_state {
    img_engine_t *engine;
    img_buffer_t source;
    img_layout_render_v1_request_t request;
    img_layout_render_rect_t *placements;
    img_layout_render_v1_result_t result;
};

static int img_layout_render_add_u32(uint32_t left, uint32_t right, uint32_t *out) {
    if (left > UINT32_MAX - right)
        return 0;
    *out = left + right;
    return 1;
}

static int img_layout_render_mul_size(size_t left, size_t right, size_t *out) {
    if (left != 0 && right > SIZE_MAX / left)
        return 0;
    *out = left * right;
    return 1;
}

static int img_layout_render_rectangles_overlap(const img_layout_render_rect_t *left,
                                                const img_layout_render_rect_t *right) {
    uint32_t left_right = left->x + left->width;
    uint32_t left_bottom = left->y + left->height;
    uint32_t right_right = right->x + right->width;
    uint32_t right_bottom = right->y + right->height;

    return left->x < right_right && right->x < left_right && left->y < right_bottom &&
           right->y < left_bottom;
}

static img_result_t
img_layout_render_validate_request(const img_layout_render_v1_request_t *request) {
    if (!request || request->version != IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION ||
        request->canvas_width == 0 || request->canvas_height == 0 ||
        request->placement_count == 0 || request->expected_count != request->placement_count ||
        !request->placements)
        return IMG_ERR_SECURITY;

    /* Defense in depth: the private renderer must not rely on its HTTP caller
     * to bound canvas memory or the quadratic rectangle-overlap validation. */
    const uint32_t canvas_pixel_limit = request->max_canvas_pixels
                                            ? request->max_canvas_pixels
                                            : IMGENGINE_LAYOUT_V2_MAX_CANVAS_PIXELS;
    if (request->crop_mode > IMG_LAYOUT_RENDER_CROP_USER_RECTANGLE ||
        canvas_pixel_limit > IMG_LAYOUT_RENDER_V1_MAX_CANVAS_PIXELS ||
        request->placement_count > IMGENGINE_LAYOUT_V2_MAX_PLACEMENTS ||
        request->canvas_width > IMGENGINE_LAYOUT_V2_MAX_DIMENSION ||
        request->canvas_height > IMGENGINE_LAYOUT_V2_MAX_DIMENSION ||
        (uint64_t)request->canvas_width * (uint64_t)request->canvas_height > canvas_pixel_limit)
        return IMG_ERR_SECURITY;

    size_t row_bytes = 0;
    size_t canvas_bytes = 0;
    if (!img_layout_render_mul_size(request->canvas_width, 3u, &row_bytes) ||
        !img_layout_render_mul_size(row_bytes, request->canvas_height, &canvas_bytes) ||
        row_bytes > UINT32_MAX || canvas_bytes == 0)
        return IMG_ERR_NOMEM;

    for (uint32_t i = 0; i < request->placement_count; i++) {
        const img_layout_render_rect_t *rect = &request->placements[i];
        uint32_t right = 0;
        uint32_t bottom = 0;
        if (rect->width == 0 || rect->height == 0 ||
            !img_layout_render_add_u32(rect->x, rect->width, &right) ||
            !img_layout_render_add_u32(rect->y, rect->height, &bottom) ||
            right > request->canvas_width || bottom > request->canvas_height)
            return IMG_ERR_SECURITY;

        for (uint32_t j = 0; j < i; j++) {
            if (img_layout_render_rectangles_overlap(rect, &request->placements[j]))
                return IMG_ERR_SECURITY;
        }
    }

    return IMG_SUCCESS;
}

static img_result_t img_layout_render_decode_nonowning(img_engine_t *engine, const uint8_t *input,
                                                       size_t input_size, img_buffer_t *out) {
    if (!engine || !input || input_size == 0 || !out)
        return IMG_ERR_SECURITY;

    img_prepared_decoder_t decoder;
    img_result_t result = img_api_prepare_decoder(engine, &decoder);
    if (result != IMG_SUCCESS)
        return result;

    /* img_api_decode_prepared calls the registered decoder without taking input ownership. */
    result = img_api_decode_prepared(&decoder, input, input_size, out);
    img_api_decoder_destroy(&decoder);
    return result;
}

static img_result_t img_layout_render_copy_request(img_layout_render_v1_state_t *state,
                                                   const img_layout_render_v1_request_t *request) {
    size_t placement_bytes = 0;
    if (!img_layout_render_mul_size(request->placement_count, sizeof(*state->placements),
                                    &placement_bytes))
        return IMG_ERR_NOMEM;

    state->placements = calloc(1, placement_bytes);
    if (!state->placements)
        return IMG_ERR_NOMEM;

    memcpy(state->placements, request->placements, placement_bytes);
    state->request = *request;
    state->request.placements = state->placements;
    return IMG_SUCCESS;
}

static img_result_t img_layout_render_fit_geometry(const img_buffer_t *source,
                                                   const img_layout_render_rect_t *rect,
                                                   uint32_t *fitted_width, uint32_t *fitted_height,
                                                   uint32_t *destination_x,
                                                   uint32_t *destination_y) {
    if (!source || source->width == 0 || source->height == 0 || !fitted_width || !fitted_height ||
        !destination_x || !destination_y)
        return IMG_ERR_SECURITY;

    const uint64_t source_width = source->width;
    const uint64_t source_height = source->height;
    const uint64_t destination_width = rect->width;
    const uint64_t destination_height = rect->height;
    const uint64_t width_by_source_height = destination_width * source_height;
    const uint64_t height_by_source_width = destination_height * source_width;
    uint64_t fit_width = 0;
    uint64_t fit_height = 0;

    if (width_by_source_height <= height_by_source_width) {
        fit_width = destination_width;
        fit_height = (source_height * destination_width) / source_width;
    } else {
        fit_height = destination_height;
        fit_width = (source_width * destination_height) / source_height;
    }

    if (fit_width == 0)
        fit_width = 1;
    if (fit_height == 0)
        fit_height = 1;
    if (fit_width > destination_width || fit_height > destination_height ||
        fit_width > UINT32_MAX || fit_height > UINT32_MAX)
        return IMG_ERR_SECURITY;

    *fitted_width = (uint32_t)fit_width;
    *fitted_height = (uint32_t)fit_height;
    *destination_x = rect->x + (rect->width - *fitted_width) / 2u;
    *destination_y = rect->y + (rect->height - *fitted_height) / 2u;
    return IMG_SUCCESS;
}

static void img_layout_render_fill_canvas(img_canvas_t *canvas,
                                          const img_layout_render_v1_request_t *request) {
    const size_t bytes = (size_t)canvas->buf.stride * canvas->buf.height;
    if (request->background_r == request->background_g &&
        request->background_g == request->background_b) {
        memset(canvas->buf.data, request->background_r, bytes);
        return;
    }

    for (size_t index = 0; index < bytes; index += 3u) {
        canvas->buf.data[index] = request->background_r;
        canvas->buf.data[index + 1u] = request->background_g;
        canvas->buf.data[index + 2u] = request->background_b;
    }
}

static void img_layout_render_state_destroy(img_layout_render_v1_state_t *state) {
    if (!state)
        return;
    img_runtime_release_raw_buffer(state->engine, &state->source);
    img_encoded_free(state->result.jpeg_data);
    free(state->placements);
    free(state);
}

img_result_t img_layout_render_v1_execute(img_layout_render_v1_state_t *state, img_ctx_t *ctx) {
    if (!state || !ctx || !ctx->local_pool || state->source.channels != 3 || !state->source.data)
        return IMG_ERR_SECURITY;

    img_canvas_t canvas = {0};
    img_result_t result = img_canvas_alloc_buffer(
        &canvas, ctx->local_pool, state->request.canvas_width, state->request.canvas_height);
    if (result != IMG_SUCCESS)
        return result;

    img_layout_render_fill_canvas(&canvas, &state->request);
    for (uint32_t index = 0; index < state->request.placement_count; index++) {
        const img_layout_render_rect_t *rect = &state->placements[index];
        uint32_t fitted_width = 0;
        uint32_t fitted_height = 0;
        uint32_t destination_x = 0;
        uint32_t destination_y = 0;
        img_buffer_t cropped_view = {0};
        img_buffer_t scaled = {0};
        const img_buffer_t *resize_source = &state->source;

        if (state->request.crop_mode == IMG_LAYOUT_RENDER_CROP_CENTERED_FILL ||
            state->request.crop_mode == IMG_LAYOUT_RENDER_CROP_USER_RECTANGLE) {
            img_layout_crop_rect_t crop = {0};
            const int crop_valid =
                state->request.crop_mode == IMG_LAYOUT_RENDER_CROP_CENTERED_FILL
                    ? img_layout_centered_crop(state->source.width, state->source.height,
                                               rect->width, rect->height, &crop)
                    : img_layout_normalized_crop_to_pixels(
                          state->source.width, state->source.height, rect->width, rect->height,
                          &state->request.normalized_crop, &crop);
            if (!crop_valid ||
                !img_layout_crop_borrowed_view(&state->source, &crop, &cropped_view)) {
                result = IMG_ERR_SECURITY;
                goto cleanup;
            }
            resize_source = &cropped_view;
            fitted_width = rect->width;
            fitted_height = rect->height;
            destination_x = rect->x;
            destination_y = rect->y;
        } else {
            result = img_layout_render_fit_geometry(&state->source, rect, &fitted_width,
                                                    &fitted_height, &destination_x, &destination_y);
            if (result != IMG_SUCCESS)
                goto cleanup;
        }

        result = img_layout_resize_bilinear(ctx, resize_source, &scaled, fitted_width,
                                            fitted_height, ctx->local_pool);
        if (result != IMG_SUCCESS) {
            if (scaled.data)
                img_runtime_release_raw_buffer(state->engine, &scaled);
            goto cleanup;
        }

        img_blit_avx2(&canvas.buf, &scaled, destination_x, destination_y);
        img_runtime_release_raw_buffer(state->engine, &scaled);
        state->result.executed_count++;
    }

    if (state->result.expected_count != state->result.planned_count ||
        state->result.planned_count != state->result.executed_count) {
        result = IMG_ERR_INTERNAL;
        goto cleanup;
    }

    if (state->request.canvas_observer) {
        state->request.canvas_observer(&canvas.buf, state->result.expected_count,
                                       state->result.planned_count, state->result.executed_count,
                                       state->request.canvas_observer_context);
    }

    if (!g_io_vtable.encode) {
        result = IMG_ERR_INTERNAL;
        goto cleanup;
    }
    result = state->request.max_output_bytes
                 ? img_encode_from_buffer_bounded(ctx, &canvas.buf, &state->result.jpeg_data,
                                                  &state->result.jpeg_size,
                                                  state->request.max_output_bytes)
                 : (img_result_t)g_io_vtable.encode(ctx, &canvas.buf, &state->result.jpeg_data,
                                                    &state->result.jpeg_size);
    if (result != IMG_SUCCESS || !state->result.jpeg_data || state->result.jpeg_size == 0) {
        img_encoded_free(state->result.jpeg_data);
        state->result.jpeg_data = NULL;
        state->result.jpeg_size = 0;
        result = result == IMG_SUCCESS ? IMG_ERR_IO : result;
    }

cleanup:
    img_canvas_release(&canvas, ctx->local_pool);
    if (result != IMG_SUCCESS) {
        img_encoded_free(state->result.jpeg_data);
        state->result.jpeg_data = NULL;
        state->result.jpeg_size = 0;
    }
    return result;
}

img_result_t img_layout_render_v1(img_engine_t *engine, const uint8_t *encoded_source,
                                  size_t encoded_source_size,
                                  const img_layout_render_v1_request_t *request,
                                  img_layout_render_v1_result_t *result) {
    if (!result)
        return IMG_ERR_SECURITY;
    memset(result, 0, sizeof(*result));
    result->status = IMG_ERR_SECURITY;

    img_result_t status = img_layout_render_validate_request(request);
    if (status != IMG_SUCCESS || !engine || !encoded_source || encoded_source_size == 0) {
        result->status = status == IMG_SUCCESS ? IMG_ERR_SECURITY : status;
        return result->status;
    }

    img_layout_render_v1_state_t *state = calloc(1, sizeof(*state));
    if (!state) {
        result->status = IMG_ERR_NOMEM;
        return result->status;
    }
    state->engine = engine;
    state->result.expected_count = request->expected_count;
    state->result.planned_count = request->placement_count;

    status = img_layout_render_copy_request(state, request);
    if (status == IMG_SUCCESS)
        status = img_layout_render_decode_nonowning(engine, encoded_source, encoded_source_size,
                                                    &state->source);
    if (status != IMG_SUCCESS)
        goto done;

    img_task_t task = {
        .kind = IMG_TASK_KIND_LAYOUT_RENDER,
        .state = IMG_TASK_READY,
        .status = IMG_ERR_INTERNAL,
        .payload.layout_render = {.state = state},
    };
    if (img_runtime_submit_task(engine, &task) != 0) {
        status = IMG_ERR_INTERNAL;
        goto done;
    }
    status = img_runtime_wait_task(&task);

done:
    state->result.status = status;
    if (status == IMG_SUCCESS) {
        *result = state->result;
        state->result.jpeg_data = NULL;
        state->result.jpeg_size = 0;
    } else {
        result->status = status;
        result->expected_count = state->result.expected_count;
        result->planned_count = state->result.planned_count;
        result->executed_count = state->result.executed_count;
    }
    img_layout_render_state_destroy(state);
    return status;
}

void img_layout_render_v1_result_release(img_layout_render_v1_result_t *result) {
    if (!result)
        return;
    img_encoded_free(result->jpeg_data);
    result->jpeg_data = NULL;
    result->jpeg_size = 0;
}
