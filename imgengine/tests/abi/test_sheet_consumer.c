#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <imgengine/api/v1/imgengine.h>
#include <imgengine/api/v2/sheet.h>

_Static_assert(sizeof(imgengine_sheet_request_v1_t) == 72, "sheet request size changed on LP64");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, placements) == 56,
               "sheet placement pointer offset changed on LP64");

static const uint8_t k_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
    0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00,
    0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08,
    0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99,
    0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static int output_is_empty(const imgengine_layout_output_v2_t *output) {
    return !output->data && output->size == 0 && output->canvas_width == 0 &&
           output->canvas_height == 0 && output->expected_count == 0 &&
           output->planned_count == 0 && output->executed_count == 0;
}

static int expect_status(imgengine_engine_t *engine, const imgengine_sheet_request_v1_t *request,
                         size_t max_output_bytes, imgengine_status_t expected) {
    imgengine_layout_output_v2_t output = {(uint8_t *)"sentinel", 8, 1, 1, 1, 1, 1};
    const imgengine_status_t actual =
        imgengine_sheet_render_v1_bounded(engine, request, max_output_bytes, &output);
    return actual != expected || !output_is_empty(&output);
}

static imgengine_sheet_request_v1_t valid_request(const imgengine_layout_rect_v2_t *placement,
                                                  uint32_t width, uint32_t height) {
    return (imgengine_sheet_request_v1_t){
        .struct_size = sizeof(imgengine_sheet_request_v1_t),
        .version = IMGENGINE_SHEET_RENDER_V1_VERSION,
        .input = k_png,
        .input_size = sizeof(k_png),
        .canvas_width = width,
        .canvas_height = height,
        .background_r = 255,
        .background_g = 255,
        .background_b = 255,
        .crop_mode = IMGENGINE_SHEET_CROP_CENTERED_FILL,
        .placements = placement,
        .placement_count = 1,
        .expected_count = 1,
    };
}

static int test_valid_small_sheet(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {.x = 1, .y = 1, .width = 4, .height = 4};
    const imgengine_sheet_request_v1_t request = valid_request(&placement, 6, 6);
    imgengine_layout_output_v2_t output = {0};
    const imgengine_status_t status =
        imgengine_sheet_render_v1_bounded(engine, &request, 16u * 1024u * 1024u, &output);
    const int failed = status != IMGENGINE_STATUS_OK || !output.data || output.size < 2 ||
                       output.data[0] != 0xff || output.data[1] != 0xd8 ||
                       output.canvas_width != 6 || output.canvas_height != 6 ||
                       output.expected_count != 1 || output.planned_count != 1 ||
                       output.executed_count != 1;
    imgengine_layout_output_release(&output);
    imgengine_layout_output_release(&output);
    return failed || !output_is_empty(&output);
}

static int test_a4_sheet(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {.x = 0, .y = 0, .width = 1, .height = 1};
    const imgengine_sheet_request_v1_t request = valid_request(&placement, 2480, 3508);
    imgengine_layout_output_v2_t output = {0};
    const imgengine_status_t status =
        imgengine_sheet_render_v1_bounded(engine, &request, 16u * 1024u * 1024u, &output);
    const int failed = status != IMGENGINE_STATUS_OK || !output.data || output.size < 2 ||
                       output.canvas_width != 2480 || output.canvas_height != 3508 ||
                       output.expected_count != 1 || output.planned_count != 1 ||
                       output.executed_count != 1;
    imgengine_layout_output_release(&output);
    return failed || !output_is_empty(&output);
}

static int test_request_rejections(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {.x = 0, .y = 0, .width = 1, .height = 1};
    imgengine_sheet_request_v1_t request = valid_request(&placement, 6, 6);

    request.struct_size = sizeof(request) - 1u;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.struct_size = sizeof(request);
    request.version = 0;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.version = IMGENGINE_SHEET_RENDER_V1_VERSION;
    request.reserved0 = 1;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.reserved0 = 0;
    request.reserved1[2] = 1;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.reserved1[2] = 0;
    request.crop_mode = 0;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.crop_mode = IMGENGINE_SHEET_CROP_CENTERED_FILL;
    request.expected_count = 2;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;
    request.expected_count = 1;
    request.canvas_width = 4097;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_RESOURCE_LIMIT))
        return 1;
    request.canvas_width = 3200;
    request.canvas_height = 3200;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_RESOURCE_LIMIT))
        return 1;
    request.canvas_width = 6;
    request.canvas_height = 6;
    if (expect_status(engine, &request, 0, IMGENGINE_STATUS_INVALID_ARGUMENT) ||
        expect_status(engine, &request, 1, IMGENGINE_STATUS_RESOURCE_LIMIT) ||
        expect_status(engine, &request, (size_t)INT_MAX + 1u, IMGENGINE_STATUS_INVALID_ARGUMENT))
        return 1;

    imgengine_layout_rect_v2_t placements[65] = {{0}};
    request.placements = placements;
    request.placement_count = 65;
    request.expected_count = 65;
    if (expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_RESOURCE_LIMIT))
        return 1;

    static const uint8_t malformed[] = {0x00, 0x01, 0x02, 0x03};
    request = valid_request(&placement, 6, 6);
    request.input = malformed;
    request.input_size = sizeof(malformed);
    return expect_status(engine, &request, 16u * 1024u * 1024u, IMGENGINE_STATUS_INVALID_IMAGE);
}

int main(void) {
    imgengine_engine_options_t options = {0};
    options.struct_size = sizeof(options);
    options.worker_count = 1;
    imgengine_engine_t *engine = NULL;
    if (imgengine_engine_create(&options, &engine) != IMGENGINE_STATUS_OK || !engine) {
        fprintf(stderr, "sheet ABI engine creation failed\n");
        return 1;
    }

    const int failed = test_valid_small_sheet(engine) || test_request_rejections(engine) ||
                       test_a4_sheet(engine);
    imgengine_engine_destroy(engine);
    if (failed) {
        fprintf(stderr, "sheet ABI 2.2 consumer failed\n");
        return 1;
    }
    puts("[abi] sheet ABI 2.2 consumer passed");
    return 0;
}
