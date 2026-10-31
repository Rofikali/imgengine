#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <imgengine/api/v2/sheet.h>

_Static_assert(IMGENGINE_SHEET_RENDER_V1_VERSION == 1u, "sheet request version changed");
_Static_assert(IMGENGINE_SHEET_CROP_CENTERED_FILL == 1u, "centered-fill enum changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, struct_size) == 0,
               "sheet struct_size offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, input) == 8,
               "sheet input offset changed on LP64");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, input_size) == 16,
               "sheet input_size offset changed on LP64");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, crop_mode) == 36,
               "sheet crop_mode offset changed on LP64");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, placements) == 56,
               "sheet placements offset changed on LP64");
_Static_assert(sizeof(imgengine_sheet_request_v1_t) == 72,
               "sheet request size changed on LP64");

static const uint8_t k_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
    0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00,
    0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08,
    0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99,
    0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static int output_is_empty(const imgengine_layout_output_v2_t *output) {
    return output && !output->data && output->size == 0 && output->canvas_width == 0 &&
           output->canvas_height == 0 && output->expected_count == 0 &&
           output->planned_count == 0 && output->executed_count == 0;
}

static imgengine_sheet_request_v1_t valid_request(const imgengine_layout_rect_v2_t *placement,
                                                   uint32_t width, uint32_t height) {
    imgengine_sheet_request_v1_t request = {
        .struct_size = sizeof(request),
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
    return request;
}

static int valid_sheet_render(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {.x = 2, .y = 2, .width = 4, .height = 4};
    const imgengine_sheet_request_v1_t request = valid_request(&placement, 8, 8);
    imgengine_layout_output_v2_t output = {0};
    const imgengine_status_t status = imgengine_sheet_render_v1_bounded(
        engine, &request, 16u * 1024u * 1024u, &output);
    const int passed = status == IMGENGINE_STATUS_OK && output.data && output.size > 2 &&
                       output.data[0] == 0xff && output.data[1] == 0xd8 &&
                       output.canvas_width == 8 && output.canvas_height == 8 &&
                       output.expected_count == 1 && output.planned_count == 1 &&
                       output.executed_count == 1;
    if (!passed)
        fprintf(stderr, "valid sheet render failed: %s\n", imgengine_status_message(status));
    imgengine_layout_output_release(&output);
    return passed && output_is_empty(&output);
}

static int rejected_requests_are_zeroed(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {.x = 0, .y = 0, .width = 4, .height = 4};
    imgengine_sheet_request_v1_t request = valid_request(&placement, 8, 8);
    imgengine_layout_output_v2_t output = {0};

    request.crop_mode = 0;
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 8, 8);
    request.reserved0 = 1;
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 8, 8);
    request.reserved1[2] = 1;
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 8, 8);
    request.struct_size = sizeof(request) - 1u;
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 8, 8);
    request.expected_count = 2;
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 8, 8);
    if (imgengine_sheet_render_v1_bounded(engine, &request, 0, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT || !output_is_empty(&output))
        return 0;

    if (imgengine_sheet_render_v1_bounded(engine, &request, 1, &output) !=
            IMGENGINE_STATUS_RESOURCE_LIMIT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 4097, 8);
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_RESOURCE_LIMIT || !output_is_empty(&output))
        return 0;

    request = valid_request(&placement, 4096, 2442);
    if (imgengine_sheet_render_v1_bounded(engine, &request, 1024, &output) !=
            IMGENGINE_STATUS_RESOURCE_LIMIT || !output_is_empty(&output))
        return 0;

    return 1;
}

static int a4_300_dpi_canvas_is_admitted(imgengine_engine_t *engine) {
    /* 210 x 297 mm at 300 DPI, half-up: 2480 x 3508 = 8,699,840 pixels. */
    const imgengine_layout_rect_v2_t placement = {.x = 100, .y = 100, .width = 4, .height = 4};
    const imgengine_sheet_request_v1_t request = valid_request(&placement, 2480, 3508);
    imgengine_layout_output_v2_t output = {0};
    const imgengine_status_t status = imgengine_sheet_render_v1_bounded(
        engine, &request, 64u * 1024u * 1024u, &output);
    const int passed = status == IMGENGINE_STATUS_OK && output.data && output.size > 2 &&
                       output.canvas_width == 2480 && output.canvas_height == 3508 &&
                       output.executed_count == 1;
    if (!passed)
        fprintf(stderr, "A4 sheet render failed: %s\n", imgengine_status_message(status));
    imgengine_layout_output_release(&output);
    return passed && output_is_empty(&output);
}

int main(void) {
    const imgengine_engine_options_t options = {.struct_size = sizeof(options), .worker_count = 1};
    imgengine_engine_t *engine = NULL;
    if (imgengine_engine_create(&options, &engine) != IMGENGINE_STATUS_OK || !engine) {
        fprintf(stderr, "engine initialization failed\n");
        return 1;
    }

    const int passed = valid_sheet_render(engine) && rejected_requests_are_zeroed(engine) &&
                       a4_300_dpi_canvas_is_admitted(engine);
    imgengine_engine_destroy(engine);
    if (!passed) {
        fprintf(stderr, "public sheet ABI consumer test failed\n");
        return 1;
    }
    printf("[abi-sheet] validation, bounded ownership, and A4 300-DPI render passed\n");
    return 0;
}
