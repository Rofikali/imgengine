#ifndef IMGENGINE_API_V2_SHEET_H
#define IMGENGINE_API_V2_SHEET_H

/*
 * Additive IMGENGINE_2.2 physical-sheet capability.
 * The request borrows input bytes and placement metadata for this synchronous call.
 */

#include "imgengine/api/v2/layout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMGENGINE_SHEET_RENDER_V1_VERSION 1u
#define IMGENGINE_SHEET_CROP_CENTERED_FILL 1u
#define IMGENGINE_SHEET_V1_MAX_CANVAS_PIXELS 10000000u

typedef struct imgengine_sheet_request_v1 {
    uint32_t struct_size;
    uint32_t version;
    const uint8_t *input;
    size_t input_size;
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint8_t background_r;
    uint8_t background_g;
    uint8_t background_b;
    uint8_t reserved0;
    uint32_t crop_mode;
    uint32_t reserved1[4];
    const imgengine_layout_rect_v2_t *placements;
    uint32_t placement_count;
    uint32_t expected_count;
} imgengine_sheet_request_v1_t;

/*
 * Renders one centered-crop-to-fill sheet. max_output_bytes is a server-owned
 * encoded-JPEG destination budget, not a canvas-pixel budget.
 * On success output owns one complete JPEG and must be released exactly once
 * with imgengine_layout_output_release. On failure output is all zeroes.
 */
IMGENGINE_API imgengine_status_t imgengine_sheet_render_v1_bounded(
    imgengine_engine_t *engine, const imgengine_sheet_request_v1_t *request,
    size_t max_output_bytes, imgengine_layout_output_v2_t *output);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_V2_SHEET_H */
