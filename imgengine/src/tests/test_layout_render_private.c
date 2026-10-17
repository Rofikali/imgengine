#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <turbojpeg.h>

#include "api/api_internal.h"
#include "api/v1/img_api.h"
#include "memory/slab.h"
#include "pipeline/layout_render_private.h"
#include "pipeline/layout_resize_internal.h"
#include "runtime/job_exec.h"

static const uint8_t k_png_fixture[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
    0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25,
    0xdb, 0x56, 0xca, 0x00, 0x00, 0x00, 0x20, 0x63, 0x48, 0x52, 0x4d, 0x00, 0x00, 0x7a, 0x26,
    0x00, 0x00, 0x80, 0x84, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x80, 0xe8, 0x00, 0x00, 0x75,
    0x30, 0x00, 0x00, 0xea, 0x60, 0x00, 0x00, 0x3a, 0x98, 0x00, 0x00, 0x17, 0x70, 0x9c, 0xba,
    0x51, 0x3c, 0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0x00, 0x00, 0xff, 0xff,
    0xff, 0x41, 0x1d, 0x34, 0x11, 0x00, 0x00, 0x00, 0x01, 0x62, 0x4b, 0x47, 0x44, 0x01, 0xff,
    0x02, 0x2d, 0xde, 0x00, 0x00, 0x00, 0x07, 0x74, 0x49, 0x4d, 0x45, 0x07, 0xea, 0x09, 0x1e,
    0x0b, 0x2b, 0x13, 0x38, 0x46, 0x83, 0xcf, 0x00, 0x00, 0x00, 0x0a, 0x49, 0x44, 0x41, 0x54,
    0x08, 0xd7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xe2, 0x21, 0xbc, 0x33, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static int make_jpeg_rgb(const uint8_t *rgb, uint32_t width, uint32_t height, uint8_t **out,
                         size_t *out_size) {
    tjhandle compressor = tjInitCompress();
    unsigned char *jpeg = NULL;
    unsigned long jpeg_size = 0;

    if (!compressor)
        return 0;
    if (tjCompress2(compressor, rgb, (int)width, (int)(width * 3u), (int)height, TJPF_RGB, &jpeg,
                    &jpeg_size, TJSAMP_444, 100, TJFLAG_ACCURATEDCT) != 0) {
        tjDestroy(compressor);
        return 0;
    }
    tjDestroy(compressor);
    *out = jpeg;
    *out_size = jpeg_size;
    return 1;
}

static int make_jpeg(uint8_t **out, size_t *out_size) {
    static const uint8_t rgb[] = {255, 0, 0, 0, 0, 255};
    return make_jpeg_rgb(rgb, 2, 1, out, out_size);
}

/* Test-only scalar FIT oracle.  It deliberately does not call native FIT/resize. */
static int reference_fit(uint32_t sw, uint32_t sh, const img_layout_render_rect_t *rect,
                         uint32_t *fw, uint32_t *fh, uint32_t *x, uint32_t *y) {
    uint64_t left = (uint64_t)rect->width * sh;
    uint64_t right = (uint64_t)rect->height * sw;
    if (!sw || !sh || !rect->width || !rect->height)
        return 0;
    if (left <= right) {
        *fw = rect->width;
        *fh = (uint32_t)(((uint64_t)sh * rect->width) / sw);
    } else {
        *fh = rect->height;
        *fw = (uint32_t)(((uint64_t)sw * rect->height) / sh);
    }
    if (!*fw)
        *fw = 1;
    if (!*fh)
        *fh = 1;
    *x = rect->x + (rect->width - *fw) / 2u;
    *y = rect->y + (rect->height - *fh) / 2u;
    return 1;
}

static int test_reference_geometry(void) {
    uint32_t fw = 0, fh = 0, x = 0, y = 0;
    const img_layout_render_rect_t horizontal = {.x = 3, .y = 5, .width = 5, .height = 5};
    const img_layout_render_rect_t vertical = {.x = 3, .y = 5, .width = 5, .height = 5};
    const img_layout_render_rect_t exact = {.x = 0, .y = 0, .width = 6, .height = 3};
    const img_layout_render_rect_t odd = {.x = 2, .y = 4, .width = 5, .height = 5};

    return reference_fit(4, 2, &horizontal, &fw, &fh, &x, &y) && fw == 5 && fh == 2 && x == 3 &&
           y == 6 && reference_fit(2, 4, &vertical, &fw, &fh, &x, &y) && fw == 2 && fh == 5 &&
           x == 4 && y == 5 && reference_fit(4, 2, &exact, &fw, &fh, &x, &y) && fw == 6 &&
           fh == 3 && x == 0 && y == 0 && reference_fit(2, 1, &odd, &fw, &fh, &x, &y) && fw == 5 &&
           fh == 2 && x == 2 && y == 5;
}

/*
 * The reference sampling equation matches the frozen scalar convention:
 * source coordinate = destination index * floor(((source - 1) << 16) /
 * (destination - 1)); each axis is a 16.16 fraction, and bilinear results
 * round by adding 2^31 before the final 32-bit shift.  This test-only code
 * never calls the production FIT, resize, blit, renderer, or scheduler.
 */
static void reference_resize_rgb(const img_buffer_t *source, uint8_t *destination,
                                 uint32_t destination_width, uint32_t destination_height) {
    const uint32_t ratio_x =
        destination_width > 1u
            ? (uint32_t)(((uint64_t)(source->width - 1u) << 16) / (destination_width - 1u))
            : 0u;
    const uint32_t ratio_y =
        destination_height > 1u
            ? (uint32_t)(((uint64_t)(source->height - 1u) << 16) / (destination_height - 1u))
            : 0u;
    for (uint32_t dy = 0; dy < destination_height; dy++) {
        const uint32_t y_fp = dy * ratio_y;
        const uint32_t y0 = y_fp >> 16;
        const uint32_t y1 = y0 + 1u < source->height ? y0 + 1u : y0;
        const uint32_t fy = y_fp & 0xffffu;
        const uint32_t wy0 = 65536u - fy;
        for (uint32_t dx = 0; dx < destination_width; dx++) {
            const uint32_t x_fp = dx * ratio_x;
            const uint32_t x0 = x_fp >> 16;
            const uint32_t x1 = x0 + 1u < source->width ? x0 + 1u : x0;
            const uint32_t fx = x_fp & 0xffffu;
            const uint32_t wx0 = 65536u - fx;
            const uint8_t *p00 = source->data + (size_t)y0 * source->stride + (size_t)x0 * 3u;
            const uint8_t *p10 = source->data + (size_t)y0 * source->stride + (size_t)x1 * 3u;
            const uint8_t *p01 = source->data + (size_t)y1 * source->stride + (size_t)x0 * 3u;
            const uint8_t *p11 = source->data + (size_t)y1 * source->stride + (size_t)x1 * 3u;
            uint8_t *out = destination + ((size_t)dy * destination_width + dx) * 3u;
            for (uint32_t channel = 0; channel < 3u; channel++) {
                const uint64_t top = (uint64_t)p00[channel] * wx0 + (uint64_t)p10[channel] * fx;
                const uint64_t bottom = (uint64_t)p01[channel] * wx0 + (uint64_t)p11[channel] * fx;
                out[channel] = (uint8_t)((top * wy0 + bottom * fy + (1ULL << 31)) >> 32);
            }
        }
    }
}

typedef struct {
    const char *name;
    const uint8_t *expected;
    size_t expected_size;
    uint32_t width;
    uint32_t height;
    uint32_t count;
    int observed;
    int passed;
} canvas_observation_t;

static void observe_renderer_canvas(const img_buffer_t *canvas, uint32_t expected_count,
                                    uint32_t planned_count, uint32_t executed_count,
                                    void *context) {
    canvas_observation_t *observation = context;
    observation->observed = 1;
    observation->passed =
        canvas && canvas->data && canvas->width == observation->width &&
        canvas->height == observation->height && canvas->channels == 3 &&
        canvas->stride == canvas->width * 3u && expected_count == observation->count &&
        planned_count == observation->count && executed_count == observation->count &&
        memcmp(canvas->data, observation->expected, observation->expected_size) == 0;
    if (!observation->passed)
        fprintf(stderr, "renderer RGB oracle mismatch: %s\n", observation->name);
}

static int reference_render_case(img_engine_t *engine, const char *name,
                                 const uint8_t *encoded_source, size_t encoded_source_size,
                                 uint32_t canvas_width, uint32_t canvas_height,
                                 const img_layout_render_rect_t *placements, uint32_t count,
                                 const uint8_t background[3]) {
    const size_t canvas_bytes = (size_t)canvas_width * canvas_height * 3u;
    uint8_t *reference = malloc(canvas_bytes);
    img_prepared_decoder_t decoder = {0};
    img_buffer_t source = {0};
    img_layout_render_v1_result_t result = {0};
    canvas_observation_t observation = {.name = name,
                                        .expected = reference,
                                        .expected_size = canvas_bytes,
                                        .width = canvas_width,
                                        .height = canvas_height,
                                        .count = count};
    if (!reference) {
        free(reference);
        return 0;
    }
    for (size_t index = 0; index < canvas_bytes; index += 3u) {
        reference[index] = background[0];
        reference[index + 1u] = background[1];
        reference[index + 2u] = background[2];
    }
    if (img_api_prepare_decoder(engine, &decoder) != IMG_SUCCESS ||
        img_api_decode_prepared(&decoder, encoded_source, encoded_source_size, &source) !=
            IMG_SUCCESS) {
        free(reference);
        img_api_decoder_destroy(&decoder);
        return 0;
    }
    for (uint32_t index = 0; index < count; index++) {
        uint32_t fitted_width = 0, fitted_height = 0, destination_x = 0, destination_y = 0;
        if (!reference_fit(source.width, source.height, &placements[index], &fitted_width,
                           &fitted_height, &destination_x, &destination_y)) {
            img_runtime_release_raw_buffer(engine, &source);
            img_api_decoder_destroy(&decoder);
            free(reference);
            return 0;
        }
        uint8_t *scaled_reference = calloc((size_t)fitted_width * fitted_height, 3u);
        if (!scaled_reference) {
            free(scaled_reference);
            img_runtime_release_raw_buffer(engine, &source);
            img_api_decoder_destroy(&decoder);
            free(reference);
            return 0;
        }
        reference_resize_rgb(&source, scaled_reference, fitted_width, fitted_height);
        for (uint32_t y = 0; y < fitted_height; y++) {
            memcpy(reference + ((size_t)(destination_y + y) * canvas_width + destination_x) * 3u,
                   scaled_reference + (size_t)y * fitted_width * 3u, (size_t)fitted_width * 3u);
        }
        free(scaled_reference);
    }
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = canvas_width,
        .canvas_height = canvas_height,
        .background_r = background[0],
        .background_g = background[1],
        .background_b = background[2],
        .placements = placements,
        .placement_count = count,
        .expected_count = count,
        .canvas_observer = observe_renderer_canvas,
        .canvas_observer_context = &observation,
    };
    const img_result_t status =
        img_layout_render_v1(engine, encoded_source, encoded_source_size, &request, &result);
    const int passed = status == IMG_SUCCESS && observation.observed && observation.passed &&
                       result.expected_count == count && result.planned_count == count &&
                       result.executed_count == count;
    img_layout_render_v1_result_release(&result);
    img_runtime_release_raw_buffer(engine, &source);
    img_api_decoder_destroy(&decoder);
    free(reference);
    return passed;
}

static int test_scalar_rgb_oracle(img_engine_t *engine) {
    static const uint8_t wide_pixels[] = {
        10,  20,  30,  40,  50,  60,  70,  80,  90,  100, 110, 120,
        130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240,
    };
    static const uint8_t tall_pixels[] = {
        9,   19,  29,  39,  49,  59,  69,  79,  89,  99,  109, 119,
        129, 139, 149, 159, 169, 179, 189, 199, 209, 219, 229, 239,
    };
    const uint8_t background[] = {17, 31, 47};
    uint8_t *wide_jpeg = NULL;
    uint8_t *tall_jpeg = NULL;
    size_t wide_jpeg_size = 0;
    size_t tall_jpeg_size = 0;
    const img_layout_render_rect_t one[] = {{.x = 1, .y = 1, .width = 5, .height = 5}};
    const img_layout_render_rect_t exact[] = {{.x = 0, .y = 0, .width = 6, .height = 3}};
    const img_layout_render_rect_t png_one[] = {{.x = 1, .y = 1, .width = 3, .height = 2}};
    const img_layout_render_rect_t multiple[] = {
        {.x = 0, .y = 0, .width = 5, .height = 5},
        {.x = 5, .y = 0, .width = 3, .height = 3},
    };
    const int generated = make_jpeg_rgb(wide_pixels, 4, 2, &wide_jpeg, &wide_jpeg_size) &&
                          make_jpeg_rgb(tall_pixels, 2, 4, &tall_jpeg, &tall_jpeg_size);
    const int passed = generated &&
                       reference_render_case(engine, "horizontal-letterbox", wide_jpeg,
                                             wide_jpeg_size, 7, 7, one, 1, background) &&
                       reference_render_case(engine, "vertical-letterbox", tall_jpeg,
                                             tall_jpeg_size, 7, 7, one, 1, background) &&
                       reference_render_case(engine, "exact-aspect", wide_jpeg, wide_jpeg_size, 6,
                                             3, exact, 1, background) &&
                       reference_render_case(engine, "multiple-placements", wide_jpeg,
                                             wide_jpeg_size, 8, 5, multiple, 2, background) &&
                       reference_render_case(engine, "png-decode-render", k_png_fixture,
                                             sizeof(k_png_fixture), 5, 4, png_one, 1, background);
    tjFree(wide_jpeg);
    tjFree(tall_jpeg);
    return passed;
}

static int assert_jpeg_result(img_engine_t *engine, img_layout_render_v1_result_t *result,
                              uint32_t expected_width, uint32_t expected_height) {
    img_prepared_decoder_t decoder;
    img_buffer_t decoded = {0};
    if (result->status != IMG_SUCCESS || !result->jpeg_data || result->jpeg_size == 0 ||
        result->expected_count != result->planned_count ||
        result->planned_count != result->executed_count) {
        fprintf(stderr, "unexpected private layout result\n");
        return 0;
    }
    if (img_api_prepare_decoder(engine, &decoder) != IMG_SUCCESS ||
        img_api_decode_prepared(&decoder, result->jpeg_data, result->jpeg_size, &decoded) !=
            IMG_SUCCESS ||
        decoded.width != expected_width || decoded.height != expected_height ||
        decoded.channels != 3) {
        fprintf(stderr, "output JPEG did not decode to expected RGB dimensions\n");
        img_api_decoder_destroy(&decoder);
        img_runtime_release_raw_buffer(engine, &decoded);
        return 0;
    }
    img_runtime_release_raw_buffer(engine, &decoded);
    img_api_decoder_destroy(&decoder);
    return 1;
}

static int test_jpeg_and_completion(img_engine_t *engine) {
    uint8_t *jpeg = NULL;
    size_t jpeg_size = 0;
    const img_layout_render_rect_t placements[] = {
        {.x = 0, .y = 0, .width = 4, .height = 4},
        {.x = 4, .y = 0, .width = 4, .height = 4},
    };
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 8,
        .canvas_height = 6,
        .background_r = 3,
        .background_g = 7,
        .background_b = 11,
        .placements = placements,
        .placement_count = 2,
        .expected_count = 2,
    };
    img_layout_render_v1_result_t result = {0};

    if (!make_jpeg(&jpeg, &jpeg_size)) {
        fprintf(stderr, "failed to generate JPEG fixture\n");
        return 0;
    }
    img_result_t status = img_layout_render_v1(engine, jpeg, jpeg_size, &request, &result);
    tjFree(jpeg);
    if (status != IMG_SUCCESS || !assert_jpeg_result(engine, &result, 8, 6)) {
        fprintf(stderr, "JPEG layout render failed: %d\n", status);
        img_layout_render_v1_result_release(&result);
        return 0;
    }
    img_layout_render_v1_result_release(&result);
    img_layout_render_v1_result_release(&result);
    return result.jpeg_data == NULL && result.jpeg_size == 0;
}

static int test_png(img_engine_t *engine) {
    const img_layout_render_rect_t placement = {.x = 1, .y = 1, .width = 3, .height = 2};
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 5,
        .canvas_height = 4,
        .background_r = 19,
        .background_g = 23,
        .background_b = 29,
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };
    img_layout_render_v1_result_t result = {0};
    img_result_t status =
        img_layout_render_v1(engine, k_png_fixture, sizeof(k_png_fixture), &request, &result);
    if (status != IMG_SUCCESS || !assert_jpeg_result(engine, &result, 5, 4)) {
        fprintf(stderr, "PNG layout render failed: %d\n", status);
        img_layout_render_v1_result_release(&result);
        return 0;
    }
    img_layout_render_v1_result_release(&result);
    return 1;
}

static int test_rejections(img_engine_t *engine) {
    uint8_t *jpeg = NULL;
    size_t jpeg_size = 0;
    const img_layout_render_rect_t overlapping[] = {
        {.x = 0, .y = 0, .width = 3, .height = 3},
        {.x = 2, .y = 2, .width = 3, .height = 3},
    };
    img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 6,
        .canvas_height = 6,
        .placements = overlapping,
        .placement_count = 2,
        .expected_count = 2,
    };
    img_layout_render_v1_result_t result = {0};
    if (!make_jpeg(&jpeg, &jpeg_size)) {
        fprintf(stderr, "failed to generate rejection JPEG fixture\n");
        return 0;
    }

    if (img_layout_render_v1(engine, jpeg, jpeg_size, &request, &result) != IMG_ERR_SECURITY ||
        result.jpeg_data != NULL) {
        fprintf(stderr, "overlap was not rejected\n");
        tjFree(jpeg);
        return 0;
    }
    request.placements = &overlapping[0];
    request.placement_count = 1;
    request.expected_count = 2;
    if (img_layout_render_v1(engine, jpeg, jpeg_size, &request, &result) != IMG_ERR_SECURITY ||
        result.jpeg_data != NULL) {
        fprintf(stderr, "count mismatch was not rejected\n");
        tjFree(jpeg);
        return 0;
    }
    request.expected_count = 1;
    request.placements =
        &(img_layout_render_rect_t){.x = UINT32_MAX, .y = 0, .width = 1, .height = 1};
    if (img_layout_render_v1(engine, jpeg, jpeg_size, &request, &result) != IMG_ERR_SECURITY ||
        result.jpeg_data != NULL) {
        fprintf(stderr, "overflow rectangle was not rejected\n");
        tjFree(jpeg);
        return 0;
    }
    request.placements = &overlapping[0];
    if (img_layout_render_v1(engine, (const uint8_t *)"bad", 3, &request, &result) == IMG_SUCCESS ||
        result.jpeg_data != NULL) {
        fprintf(stderr, "malformed input was accepted\n");
        tjFree(jpeg);
        return 0;
    }
    tjFree(jpeg);
    return 1;
}

static int test_partial_execution_has_no_output(img_engine_t *engine) {
    uint8_t *jpeg = NULL;
    size_t jpeg_size = 0;
    const img_layout_render_rect_t rect = {.x = 0, .y = 0, .width = 1, .height = 1};
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 100000u,
        .canvas_height = 100000u,
        .placements = &rect,
        .placement_count = 1,
        .expected_count = 1,
    };
    img_layout_render_v1_result_t result = {0};
    if (!make_jpeg(&jpeg, &jpeg_size))
        return 0;
    const img_result_t status = img_layout_render_v1(engine, jpeg, jpeg_size, &request, &result);
    tjFree(jpeg);
    const int passed = status != IMG_SUCCESS && result.jpeg_data == NULL && result.jpeg_size == 0 &&
                       result.planned_count == 1 && result.executed_count == 0;
    img_layout_render_v1_result_release(&result);
    return passed;
}

int main(void) {
    img_engine_t *engine = img_api_init(1);
    if (!engine) {
        fprintf(stderr, "engine initialization failed\n");
        return 1;
    }
    const int passed = test_reference_geometry() && test_scalar_rgb_oracle(engine) &&
                       test_jpeg_and_completion(engine) && test_png(engine) &&
                       test_rejections(engine) && test_partial_execution_has_no_output(engine);
    img_api_shutdown(engine);
    if (!passed) {
        fprintf(stderr, "private layout render test failed\n");
        return 1;
    }
    printf("[layout-render] JPEG/PNG, scheduler, ownership, and rejection cases passed\n");
    return 0;
}
