#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "api/imgengine_engine_private.h"
#include "imgengine/api/v2/layout.h"
#include "pipeline/layout_render_private.h"

static imgengine_status_t imgengine_layout_map_result(img_result_t result) {
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

void imgengine_layout_output_release(imgengine_layout_output_v2_t *output) {
    if (!output)
        return;
    img_layout_render_v1_result_t result = {.jpeg_data = output->data, .jpeg_size = output->size};
    img_layout_render_v1_result_release(&result);
    memset(output, 0, sizeof(*output));
}

imgengine_status_t imgengine_layout_render_v2(imgengine_engine_t *engine,
                                              const imgengine_layout_request_v2_t *request,
                                              imgengine_layout_output_v2_t *output) {
    if (!output)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    memset(output, 0, sizeof(*output));
    if (!engine || !request || request->struct_size < sizeof(*request) ||
        request->version != IMGENGINE_LAYOUT_RENDER_V2_VERSION || request->reserved0 != 0 ||
        !request->input || request->input_size == 0 || !request->placements ||
        request->placement_count == 0 || request->expected_count != request->placement_count)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;

#if SIZE_MAX <= UINT32_MAX
    if (request->placement_count > SIZE_MAX / sizeof(img_layout_render_rect_t))
        return IMGENGINE_STATUS_RESOURCE_LIMIT;
#endif
    img_layout_render_rect_t *placements = calloc(request->placement_count, sizeof(*placements));
    if (!placements)
        return IMGENGINE_STATUS_OUT_OF_MEMORY;
    for (uint32_t index = 0; index < request->placement_count; index++) {
        placements[index].x = request->placements[index].x;
        placements[index].y = request->placements[index].y;
        placements[index].width = request->placements[index].width;
        placements[index].height = request->placements[index].height;
    }
    img_layout_render_v1_request_t private_request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = request->canvas_width,
        .canvas_height = request->canvas_height,
        .background_r = request->background_r,
        .background_g = request->background_g,
        .background_b = request->background_b,
        .placements = placements,
        .placement_count = request->placement_count,
        .expected_count = request->expected_count,
    };
    img_layout_render_v1_result_t result = {0};

    pthread_mutex_lock(&engine->operation_lock);
    const img_result_t status = img_layout_render_v1(
        engine->native_engine, request->input, request->input_size, &private_request, &result);
    pthread_mutex_unlock(&engine->operation_lock);
    free(placements);
    if (status != IMG_SUCCESS) {
        img_layout_render_v1_result_release(&result);
        return imgengine_layout_map_result(status);
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
