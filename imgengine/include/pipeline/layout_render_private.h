#ifndef IMGENGINE_PIPELINE_LAYOUT_RENDER_PRIVATE_H
#define IMGENGINE_PIPELINE_LAYOUT_RENDER_PRIVATE_H

#include <stddef.h>
#include <stdint.h>

#include "core/buffer.h"
#include "core/result.h"

typedef struct img_engine img_engine_t;
typedef struct img_ctx img_ctx_t;

#define IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION 1u
#define IMG_LAYOUT_RENDER_CROP_FIT 0u
#define IMG_LAYOUT_RENDER_CROP_CENTERED_FILL 1u
#define IMG_LAYOUT_RENDER_V1_MAX_CANVAS_PIXELS 10000000u

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} img_layout_render_rect_t;

/*
 * Private verification hook. It is called synchronously by the worker after
 * the complete RGB canvas is rendered and before JPEG encoding. `canvas` is
 * borrowed renderer-owned storage and is valid only for the callback.
 */
typedef void (*img_layout_render_v1_canvas_observer_t)(const img_buffer_t *canvas,
                                                       uint32_t expected_count,
                                                       uint32_t planned_count,
                                                       uint32_t executed_count, void *context);

typedef struct {
    uint32_t version;
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint8_t background_r;
    uint8_t background_g;
    uint8_t background_b;
    const img_layout_render_rect_t *placements;
    uint32_t placement_count;
    uint32_t expected_count;
    size_t max_output_bytes;    /* zero preserves existing unbounded behavior */
    uint32_t crop_mode;         /* zero is the frozen FIT behavior */
    uint32_t max_canvas_pixels; /* zero preserves the existing 4 MP limit */
    img_layout_render_v1_canvas_observer_t canvas_observer;
    void *canvas_observer_context;
} img_layout_render_v1_request_t;

typedef struct {
    img_result_t status;
    uint32_t expected_count;
    uint32_t planned_count;
    uint32_t executed_count;
    uint8_t *jpeg_data;
    size_t jpeg_size;
} img_layout_render_v1_result_t;

typedef struct img_layout_render_v1_state img_layout_render_v1_state_t;

img_result_t img_layout_render_v1(img_engine_t *engine, const uint8_t *encoded_source,
                                  size_t encoded_source_size,
                                  const img_layout_render_v1_request_t *request,
                                  img_layout_render_v1_result_t *result);

void img_layout_render_v1_result_release(img_layout_render_v1_result_t *result);

img_result_t img_layout_render_v1_execute(img_layout_render_v1_state_t *state, img_ctx_t *ctx);

#endif /* IMGENGINE_PIPELINE_LAYOUT_RENDER_PRIVATE_H */
