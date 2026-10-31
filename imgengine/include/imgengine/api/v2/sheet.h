#ifndef IMGENGINE_API_V2_SHEET_H
#define IMGENGINE_API_V2_SHEET_H
/*
 * Additive IMGENGINE_2.2 physical-sheet rendering capability.
 * ABI v1 and the existing ABI-v2 FIT layout request remain unchanged.
 */

#include <stddef.h>
#include <stdint.h>

#include "imgengine/api/v2/layout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMGENGINE_SHEET_RENDER_V1_VERSION 1u
#define IMGENGINE_SHEET_RENDER_V2_VERSION 2u
#define IMGENGINE_SHEET_CROP_CENTERED_FILL 1u
#define IMGENGINE_SHEET_CROP_USER_RECTANGLE 2u
#define IMGENGINE_SHEET_V1_MAX_CANVAS_PIXELS 10000000u

/*
 * The input and placements are borrowed for this synchronous call.
 * crop_mode must be CENTERED_FILL. The native renderer derives crop geometry
 * from its one decoded source buffer; callers cannot supply crop coordinates.
 * Every reserved field must be zero.
 */
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
 * ABI 2.3 crop-aware request. The normalized rectangle uses the fixed scale
 * 1,000,000 and is applied to every placement. C validates it against decoded
 * source dimensions and each target placement. The v1 request and symbol
 * remain unchanged.
 */
typedef struct imgengine_normalized_crop_v1 {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} imgengine_normalized_crop_v1_t;

typedef struct imgengine_sheet_request_v2 {
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
    imgengine_normalized_crop_v1_t crop;
} imgengine_sheet_request_v2_t;

IMGENGINE_API imgengine_status_t imgengine_sheet_render_v2_bounded(
    imgengine_engine_t *engine, const imgengine_sheet_request_v2_t *request,
    size_t max_output_bytes, imgengine_layout_output_v2_t *output);

/*
 * A bounded JPEG output budget is mandatory and server-owned.
 * On success, release the returned output exactly once with
 * imgengine_layout_output_release(). On every failure, output is all zeroes.
 */
IMGENGINE_API imgengine_status_t imgengine_sheet_render_v1_bounded(
    imgengine_engine_t *engine, const imgengine_sheet_request_v1_t *request,
    size_t max_output_bytes, imgengine_layout_output_v2_t *output);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_V2_SHEET_H */
