#ifndef IMGENGINE_API_V2_LAYOUT_H
#define IMGENGINE_API_V2_LAYOUT_H

/*
 * Additive IMGENGINE_2.0 layout capability. ABI v1 remains unchanged: callers
 * create the opaque engine using imgengine/api/v1/imgengine.h and may pass that
 * handle here. This header deliberately exposes no scheduler, allocator, or
 * private renderer types.
 */

#include "imgengine/api/v1/imgengine.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMGENGINE_LAYOUT_RENDER_V2_VERSION 1u

typedef struct imgengine_layout_rect_v2 {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} imgengine_layout_rect_v2_t;

typedef struct imgengine_layout_request_v2 {
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
    const imgengine_layout_rect_v2_t *placements;
    uint32_t placement_count;
    uint32_t expected_count;
} imgengine_layout_request_v2_t;

typedef struct imgengine_layout_output_v2 {
    uint8_t *data;
    size_t size;
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint32_t expected_count;
    uint32_t planned_count;
    uint32_t executed_count;
} imgengine_layout_output_v2_t;

/*
 * `input` and `placements` are borrowed only for this synchronous terminal
 * call. The native capability decodes the source and copies queued metadata
 * before scheduler submission. On success `output` is caller-owned and must
 * be released with imgengine_layout_output_release exactly once.
 */
IMGENGINE_API imgengine_status_t
imgengine_layout_render_v2(imgengine_engine_t *engine, const imgengine_layout_request_v2_t *request,
                           imgengine_layout_output_v2_t *output);
IMGENGINE_API void imgengine_layout_output_release(imgengine_layout_output_v2_t *output);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_V2_LAYOUT_H */
