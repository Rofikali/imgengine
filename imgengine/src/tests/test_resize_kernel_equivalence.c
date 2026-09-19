#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/arch_interface.h"
#include "arch/cpu_caps.h"
#include "arch/resize_params.h"

static uint32_t next_value(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
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

    printf("[resize] 128 AVX2/scalar equivalence cases passed\n");
    return 0;
}
