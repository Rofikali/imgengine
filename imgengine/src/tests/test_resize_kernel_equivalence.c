#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/arch_interface.h"
#include "arch/cpu_caps.h"
#include "arch/resize_params.h"
#include "core/context_internal.h"
#include "pipeline/fused_params.h"
#include "pipeline/pipeline_fuse_exec_internal.h"

static uint32_t next_value(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

/* Test-only RGB luma reference; never calls a production grayscale helper. */
static uint8_t grayscale_reference(uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) {
    uint32_t value = ((uint32_t)r * 77u + (uint32_t)g * 150u + (uint32_t)b * 29u) >> 8;
    value += brightness;
    return value > 255u ? 255u : (uint8_t)value;
}

static void grayscale_reference_apply(img_buffer_t *buffer, uint16_t brightness) {
    for (uint32_t y = 0; y < buffer->height; y++) {
        uint8_t *row = buffer->data + (size_t)y * buffer->stride;
        for (uint32_t x = 0; x < buffer->width; x++) {
            uint8_t gray =
                grayscale_reference(row[x * 3u], row[x * 3u + 1u], row[x * 3u + 2u], brightness);
            row[x * 3u] = gray;
            row[x * 3u + 1u] = gray;
            row[x * 3u + 2u] = gray;
        }
    }
}

typedef struct {
    int standalone_scalar;
    int standalone_avx;
    int fused_scalar;
    int fused_avx;
} grayscale_results_t;

static int grayscale_case(uint32_t width, uint32_t height, uint32_t padding, uint16_t brightness,
                          int unaligned, grayscale_results_t *results) {
    static const uint8_t fixture_rgb[] = {
        0, 0,   0, 255, 255, 255, 128, 128, 128, 255, 0,  0,
        0, 255, 0, 0,   0,   255, 19,  73,  211, 250, 17, 91,
    };
    const uint32_t stride = width * 3u + padding;
    const size_t bytes = (size_t)stride * height;
    uint8_t *reference_storage = calloc(bytes + 1u, 1u);
    uint8_t *scalar_storage = calloc(bytes + 1u, 1u);
    uint8_t *avx_storage = calloc(bytes + 1u, 1u);
    uint8_t *fused_scalar_storage = calloc(bytes + 1u, 1u);
    uint8_t *fused_avx_storage = calloc(bytes + 1u, 1u);
    if (!reference_storage || !scalar_storage || !avx_storage || !fused_scalar_storage ||
        !fused_avx_storage)
        goto fail;
    const size_t offset = unaligned ? 1u : 0u;
    uint8_t *reference_data = reference_storage + offset;
    uint8_t *scalar_data = scalar_storage + offset;
    uint8_t *avx_data = avx_storage + offset;
    uint8_t *fused_scalar_data = fused_scalar_storage + offset;
    uint8_t *fused_avx_data = fused_avx_storage + offset;
    for (size_t i = 0; i < bytes; i++)
        reference_data[i] = fixture_rgb[i % sizeof(fixture_rgb)];
    memcpy(scalar_data, reference_data, bytes);
    memcpy(avx_data, reference_data, bytes);
    memcpy(fused_scalar_data, reference_data, bytes);
    memcpy(fused_avx_data, reference_data, bytes);
    img_buffer_t reference = {
        .data = reference_data, .width = width, .height = height, .channels = 3u, .stride = stride};
    img_buffer_t scalar = {
        .data = scalar_data, .width = width, .height = height, .channels = 3u, .stride = stride};
    img_buffer_t avx = {
        .data = avx_data, .width = width, .height = height, .channels = 3u, .stride = stride};
    img_buffer_t fused_scalar = {.data = fused_scalar_data,
                                 .width = width,
                                 .height = height,
                                 .channels = 3u,
                                 .stride = stride};
    img_buffer_t fused_avx = {
        .data = fused_avx_data, .width = width, .height = height, .channels = 3u, .stride = stride};
    grayscale_reference_apply(&reference, brightness);
    img_arch_grayscale_scalar(NULL, &scalar, NULL);
    results->standalone_scalar =
        brightness == 0u && memcmp(reference_data, scalar_data, bytes) == 0;
    img_arch_grayscale_avx2(NULL, &avx, NULL);
    results->standalone_avx = brightness == 0u && memcmp(reference_data, avx_data, bytes) == 0;
    img_fused_params_t parameters = {
        .has_grayscale = 1u, .has_brightness = brightness != 0u, .brightness_value = brightness};
    img_ctx_t context = {.fused_params = &parameters};
    fused_exec_scalar(&context, &fused_scalar);
    fused_exec_avx2(&context, &fused_avx);
    results->fused_scalar = memcmp(reference_data, fused_scalar_data, bytes) == 0;
    results->fused_avx = memcmp(reference_data, fused_avx_data, bytes) == 0;
    free(reference_storage);
    free(scalar_storage);
    free(avx_storage);
    free(fused_scalar_storage);
    free(fused_avx_storage);
    return 1;
fail:
    free(reference_storage);
    free(scalar_storage);
    free(avx_storage);
    free(fused_scalar_storage);
    free(fused_avx_storage);
    return 0;
}

static int test_grayscale_oracle(void) {
    grayscale_results_t one = {0};
    grayscale_results_t tail = {0};
    grayscale_results_t simd = {0};
    grayscale_results_t padded = {0};
    grayscale_results_t brightness = {0};
    if (!grayscale_case(1u, 1u, 0u, 0u, 0, &one) || !grayscale_case(10u, 2u, 2u, 0u, 1, &tail) ||
        !grayscale_case(11u, 3u, 5u, 0u, 0, &simd) ||
        !grayscale_case(11u, 3u, 5u, 17u, 0, &padded) ||
        !grayscale_case(37u, 4u, 7u, 200u, 1, &brightness))
        return 0;
    if (!one.standalone_scalar || !tail.standalone_scalar)
        return 0;
    printf("[grayscale] tail scalar=%d avx=%d fused-scalar=%d fused-avx=%d; "
           "simd scalar=%d avx=%d fused-scalar=%d fused-avx=%d; "
           "padded fused-scalar=%d fused-avx=%d; brightness fused-scalar=%d fused-avx=%d\n",
           tail.standalone_scalar, tail.standalone_avx, tail.fused_scalar, tail.fused_avx,
           simd.standalone_scalar, simd.standalone_avx, simd.fused_scalar, simd.fused_avx,
           padded.fused_scalar, padded.fused_avx, brightness.fused_scalar, brightness.fused_avx);
    if (!tail.standalone_avx || !tail.fused_scalar || !tail.fused_avx || !simd.standalone_avx ||
        !simd.fused_scalar || !simd.fused_avx || !padded.fused_scalar || !padded.fused_avx ||
        !brightness.fused_scalar || !brightness.fused_avx) {
        fprintf(stderr, "grayscale scalar/AVX2/fused equivalence mismatch\n");
        return 0;
    }
    printf("[grayscale] SIMD-001 scalar/AVX2/fused RGB-luma equivalence passed\n");
    return 1;
}

int main(void) {
    if (!img_cpu_has_avx2(img_cpu_detect_caps())) {
        printf("[resize] AVX2 unavailable; kernel equivalence skipped\n");
        return 0;
    }

    uint32_t state = 0x5245535au;
    for (uint32_t case_index = 0; case_index < 128u; case_index++) {
        uint32_t src_width = 1u + next_value(&state) % 129u;
        uint32_t src_height = 1u + next_value(&state) % 97u;
        uint32_t dst_width = 1u + next_value(&state) % 137u;
        uint32_t dst_height = 1u + next_value(&state) % 101u;
        size_t src_size = (size_t)src_width * src_height * 3u;
        size_t dst_size = (size_t)dst_width * dst_height * 3u;
        uint8_t *src_data = malloc(src_size);
        uint8_t *scalar_data = calloc(dst_size, 1u);
        uint8_t *avx_data = calloc(dst_size, 1u);
        uint32_t *x_index = malloc((size_t)dst_width * sizeof(*x_index));
        uint32_t *y_index = malloc((size_t)dst_height * sizeof(*y_index));
        uint16_t *x_weight = malloc((size_t)dst_width * sizeof(*x_weight));
        uint16_t *y_weight = malloc((size_t)dst_height * sizeof(*y_weight));

        if (!src_data || !scalar_data || !avx_data || !x_index || !y_index || !x_weight ||
            !y_weight) {
            free(src_data);
            free(scalar_data);
            free(avx_data);
            free(x_index);
            free(y_index);
            free(x_weight);
            free(y_weight);
            fprintf(stderr, "allocation failed at case %u\n", case_index);
            return 1;
        }

        for (size_t index = 0; index < src_size; index++)
            src_data[index] = (uint8_t)next_value(&state);

        img_buffer_t src = {
            .data = src_data,
            .width = src_width,
            .height = src_height,
            .channels = 3u,
            .stride = src_width * 3u,
        };
        img_buffer_t scalar = {
            .data = scalar_data,
            .width = dst_width,
            .height = dst_height,
            .channels = 3u,
            .stride = dst_width * 3u,
        };
        img_buffer_t avx = scalar;
        avx.data = avx_data;
        uint32_t scale_x = (dst_width > 1u && src_width > 1u)
                               ? (uint32_t)(((uint64_t)(src_width - 1u) << 16) / (dst_width - 1u))
                               : 0u;
        uint32_t scale_y = (dst_height > 1u && src_height > 1u)
                               ? (uint32_t)(((uint64_t)(src_height - 1u) << 16) / (dst_height - 1u))
                               : 0u;
        for (uint32_t index = 0; index < dst_width; index++) {
            uint64_t fixed_point = (uint64_t)index * scale_x;
            x_index[index] = (uint32_t)(fixed_point >> 16);
            x_weight[index] = (uint16_t)fixed_point;
        }
        for (uint32_t index = 0; index < dst_height; index++) {
            uint64_t fixed_point = (uint64_t)index * scale_y;
            y_index[index] = (uint32_t)(fixed_point >> 16);
            y_weight[index] = (uint16_t)fixed_point;
        }
        resize_params_t params = {
            .src = &src,
            .target_w = dst_width,
            .target_h = dst_height,
            .scale_x = scale_x,
            .scale_y = scale_y,
            .x_index = (case_index & 1u) ? x_index : NULL,
            .y_index = (case_index & 1u) ? y_index : NULL,
            .x_weight = (case_index & 1u) ? x_weight : NULL,
            .y_weight = (case_index & 1u) ? y_weight : NULL,
        };

        resize_scalar(NULL, &scalar, &params);
        img_arch_avx2_resize(NULL, &avx, &params);
        if (memcmp(scalar_data, avx_data, dst_size) != 0) {
            fprintf(stderr, "AVX2 resize differs from scalar at case %u: %ux%u -> %ux%u\n",
                    case_index, src_width, src_height, dst_width, dst_height);
            free(src_data);
            free(scalar_data);
            free(avx_data);
            free(x_index);
            free(y_index);
            free(x_weight);
            free(y_weight);
            return 1;
        }

        free(src_data);
        free(scalar_data);
        free(avx_data);
        free(x_index);
        free(y_index);
        free(x_weight);
        free(y_weight);
    }

    if (!test_grayscale_oracle())
        return 1;
    printf("[resize] 128 AVX2/scalar and grayscale oracle cases passed\n");
    return 0;
}
