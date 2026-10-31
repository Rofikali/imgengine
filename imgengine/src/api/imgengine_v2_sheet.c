#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "api/imgengine_engine_private.h"
#include "imgengine/api/v2/sheet.h"
#include "pipeline/layout_render_private.h"

static imgengine_status_t imgengine_sheet_map_result(img_result_t result) {
    switch (result) {
    case IMG_SUCCESS:
        return IMGENGINE_STATUS_OK;
    case IMG_ERR_NOMEM:
        return IMGENGINE_STATUS_OUT_OF_MEMORY;
    case IMG_ERR_FORMAT:
        return IMGENGINE_STATUS_INVALID_IMAGE;
    case IMG_ERR_SECURITY:
        return IMGENGINE_STATUS_RESOURCE_LIMIT;
    case IMG_ERR_HW_UNSUP:
        return IMGENGINE_STATUS_UNAVAILABLE;
    case IMG_ERR_IO:
    case IMG_ERR_INTERNAL:
    default:
        return IMGENGINE_STATUS_INTERNAL_ERROR;
    }
}

static imgengine_status_t imgengine_sheet_render_common(
    imgengine_engine_t *engine, const imgengine_sheet_request_v1_t *request,
    uint32_t private_crop_mode, const imgengine_normalized_crop_v1_t *normalized_crop,
    size_t max_output_bytes, imgengine_layout_output_v2_t *output) {
    if (!output)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    memset(output, 0, sizeof(*output));

    if (!engine || !engine->native_engine || !request ||
        request->struct_size < sizeof(imgengine_sheet_request_v1_t) ||
        request->version != IMGENGINE_SHEET_RENDER_V1_VERSION || request->reserved0 != 0 ||
        !request->input || request->input_size == 0 || !request->placements ||
        request->placement_count == 0 || request->expected_count != request->placement_count ||
        request->crop_mode != IMGENGINE_SHEET_CROP_CENTERED_FILL)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;

    for (size_t index = 0; index < sizeof(request->reserved1) / sizeof(request->reserved1[0]);
         index++) {
        if (request->reserved1[index] != 0)
            return IMGENGINE_STATUS_INVALID_ARGUMENT;
    }

    if (request->placement_count > IMGENGINE_LAYOUT_V2_MAX_PLACEMENTS ||
        request->canvas_width == 0 || request->canvas_height == 0 ||
        request->canvas_width > IMGENGINE_LAYOUT_V2_MAX_DIMENSION ||
        request->canvas_height > IMGENGINE_LAYOUT_V2_MAX_DIMENSION ||
        (uint64_t)request->canvas_width * (uint64_t)request->canvas_height >
            IMGENGINE_SHEET_V1_MAX_CANVAS_PIXELS)
        return IMGENGINE_STATUS_RESOURCE_LIMIT;

    if (max_output_bytes == 0 || max_output_bytes > INT_MAX)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;

    img_layout_render_rect_t *placements = calloc(request->placement_count, sizeof(*placements));
    if (!placements)
        return IMGENGINE_STATUS_OUT_OF_MEMORY;

    for (uint32_t index = 0; index < request->placement_count; index++) {
        placements[index].x = request->placements[index].x;
        placements[index].y = request->placements[index].y;
        placements[index].width = request->placements[index].width;
        placements[index].height = request->placements[index].height;
    }

    const img_layout_render_v1_request_t private_request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = request->canvas_width,
        .canvas_height = request->canvas_height,
        .background_r = request->background_r,
        .background_g = request->background_g,
        .background_b = request->background_b,
        .placements = placements,
        .placement_count = request->placement_count,
        .expected_count = request->expected_count,
        .max_output_bytes = max_output_bytes,
        .crop_mode = private_crop_mode,
        .normalized_crop = normalized_crop
                               ? (img_layout_normalized_crop_t){normalized_crop->x, normalized_crop->y,
                                                               normalized_crop->width,
                                                               normalized_crop->height}
                               : (img_layout_normalized_crop_t){0},
        .max_canvas_pixels = IMGENGINE_SHEET_V1_MAX_CANVAS_PIXELS,
    };
    img_layout_render_v1_result_t result = {0};

    pthread_mutex_lock(&engine->operation_lock);
    const img_result_t status = img_layout_render_v1(
        engine->native_engine, request->input, request->input_size, &private_request, &result);
    pthread_mutex_unlock(&engine->operation_lock);
    free(placements);

    if (status != IMG_SUCCESS) {
        img_layout_render_v1_result_release(&result);
        return imgengine_sheet_map_result(status);
    }

    if (!result.jpeg_data || result.jpeg_size == 0 ||
        result.expected_count != request->expected_count ||
        result.planned_count != request->placement_count ||
        result.executed_count != request->placement_count) {
        img_layout_render_v1_result_release(&result);
        return IMGENGINE_STATUS_INTERNAL_ERROR;
    }

    output->data = result.jpeg_data;
    output->size = result.jpeg_size;
    output->canvas_width = request->canvas_width;
    output->canvas_height = request->canvas_height;
    output->expected_count = result.expected_count;
    output->planned_count = result.planned_count;
    output->executed_count = result.executed_count;
    result.jpeg_data = NULL;
    result.jpeg_size = 0;
    return IMGENGINE_STATUS_OK;
}

imgengine_status_t imgengine_sheet_render_v1_bounded(imgengine_engine_t *engine,
                                                     const imgengine_sheet_request_v1_t *request,
                                                     size_t max_output_bytes,
                                                     imgengine_layout_output_v2_t *output) {
    return imgengine_sheet_render_common(engine, request, IMG_LAYOUT_RENDER_CROP_CENTERED_FILL,
                                         NULL, max_output_bytes, output);
}

imgengine_status_t imgengine_sheet_render_v2_bounded(imgengine_engine_t *engine,
                                                     const imgengine_sheet_request_v2_t *request,
                                                     size_t max_output_bytes,
                                                     imgengine_layout_output_v2_t *output) {
    if (!output)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    memset(output, 0, sizeof(*output));
    if (!request || request->struct_size < sizeof(*request) ||
        request->version != IMGENGINE_SHEET_RENDER_V2_VERSION ||
        request->crop_mode != IMGENGINE_SHEET_CROP_USER_RECTANGLE)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;

    const imgengine_sheet_request_v1_t base = {
        .struct_size = sizeof(imgengine_sheet_request_v1_t),
        .version = IMGENGINE_SHEET_RENDER_V1_VERSION,
        .input = request->input,
        .input_size = request->input_size,
        .canvas_width = request->canvas_width,
        .canvas_height = request->canvas_height,
        .background_r = request->background_r,
        .background_g = request->background_g,
        .background_b = request->background_b,
        .reserved0 = request->reserved0,
        .reserved1 = {request->reserved1[0], request->reserved1[1], request->reserved1[2],
                      request->reserved1[3]},
        .crop_mode = IMGENGINE_SHEET_CROP_CENTERED_FILL,
        .placements = request->placements,
        .placement_count = request->placement_count,
        .expected_count = request->expected_count,
    };
    /* The common validator checks the shared reserved fields and all sheet limits. */
    return imgengine_sheet_render_common(engine, &base, IMG_LAYOUT_RENDER_CROP_USER_RECTANGLE,
                                         &request->crop, max_output_bytes, output);
}
