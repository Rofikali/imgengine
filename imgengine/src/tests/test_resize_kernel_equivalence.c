#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "arch/arch_interface.h"
#include "arch/cpu_caps.h"
#include "arch/resize_params.h"
#include "core/opcodes.h"
#include "pipeline/jump_table.h"
#include "pipeline/pipeline_compiled.h"
#include "pipeline/pipeline_fuse_internal.h"
#include "pipeline/pipeline_types.h"
#include "runtime/rpc_server_internal.h"
#include "startup/engine_init_internal.h"

void img_pipeline_execute_compiled_hot(img_ctx_t *ctx, const img_pipeline_compiled_t *pipe,
                                       img_buffer_t *buf);

static uint32_t next_value(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static void grayscale_reference(uint8_t *data, size_t size) {
    for (size_t index = 0; index < size; index += 3u) {
        uint8_t gray =
            (uint8_t)((data[index] * 77u + data[index + 1u] * 150u + data[index + 2u] * 29u) >> 8u);
        data[index] = gray;
        data[index + 1u] = gray;
        data[index + 2u] = gray;
    }
}

static int write_all(int fd, const void *data, size_t size) {
    const uint8_t *cursor = data;
    while (size > 0u) {
        ssize_t written = write(fd, cursor, size);
        if (written <= 0)
            return -1;
        cursor += (size_t)written;
        size -= (size_t)written;
    }
    return 0;
}

static int read_all(int fd, void *data, size_t size) {
    uint8_t *cursor = data;
    while (size > 0u) {
        ssize_t received = read(fd, cursor, size);
        if (received <= 0)
            return -1;
        cursor += (size_t)received;
        size -= (size_t)received;
    }
    return 0;
}

/*
 * Deliberately independent of AVX availability. It exercises the full-engine
 * ordering: hardware dispatch first, then the section-registered built-in
 * plugin override.
 */
static int test_registered_grayscale_dispatch(void) {
    const uint8_t source[] = {255u, 0u, 0u, 0u, 255u, 0u, 0u, 0u, 255u, 7u, 19u, 251u};
    uint8_t expected[sizeof(source)];
    uint8_t direct[sizeof(source)];
    uint8_t compiled_data[sizeof(source)];
    img_buffer_t buffer = {
        .data = direct,
        .width = 4u,
        .height = 1u,
        .channels = 3u,
        .stride = 12u,
    };

    memcpy(expected, source, sizeof(source));
    grayscale_reference(expected, sizeof(expected));

    img_engine_init_compute(img_cpu_detect_caps());
    img_kernel_fn hardware_grayscale = g_jump_table[OP_GRAYSCALE];
    img_engine_init_plugins_and_io();
    img_kernel_fn registered_grayscale = g_jump_table[OP_GRAYSCALE];
    if (!hardware_grayscale || !registered_grayscale ||
        hardware_grayscale == registered_grayscale) {
        fprintf(stderr, "grayscale plugin registration did not override hardware dispatch\n");
        return 1;
    }

    memcpy(direct, source, sizeof(source));
    registered_grayscale(NULL, &buffer, NULL);
    if (memcmp(direct, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "registered grayscale dispatch differs from RGB-luma oracle\n");
        return 1;
    }

    img_pipeline_desc_t description = {0};
    description.count = 1u;
    description.ops[0].op_code = OP_GRAYSCALE;
    img_pipeline_compiled_t compiled = {0};
    if (img_pipeline_compile(&description, &compiled) != 0) {
        fprintf(stderr, "could not compile grayscale pipeline after registration\n");
        return 1;
    }
    memcpy(compiled_data, source, sizeof(source));
    buffer.data = compiled_data;
    img_pipeline_execute_compiled_hot(NULL, &compiled, &buffer);
    if (memcmp(compiled_data, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "compiled grayscale pipeline differs from RGB-luma oracle\n");
        return 1;
    }

    return 0;
}

/*
 * The hardware-selected entry is observable before built-in plugin
 * registration. Keep this separate from the plugin test so a later plugin
 * override cannot mask a non-equivalent SIMD implementation.
 */
static int test_hardware_grayscale_dispatch(void) {
    uint8_t actual[36];
    uint8_t expected[sizeof(actual)];
    img_buffer_t buffer = {
        .data = actual,
        .width = 12u,
        .height = 1u,
        .channels = 3u,
        .stride = sizeof(actual),
    };

    for (size_t index = 0; index < sizeof(actual); index++)
        actual[index] = (uint8_t)(index * 37u + 11u);
    memcpy(expected, actual, sizeof(actual));
    grayscale_reference(expected, sizeof(expected));

    img_engine_init_compute(img_cpu_detect_caps());
    if (!g_jump_table[OP_GRAYSCALE]) {
        fprintf(stderr, "hardware grayscale dispatch was not initialized\n");
        return 1;
    }
    g_jump_table[OP_GRAYSCALE](NULL, &buffer, NULL);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fprintf(stderr, "hardware grayscale dispatch differs from RGB-luma oracle\n");
        return 1;
    }
    return 0;
}

static int test_fused_grayscale_dispatch(void) {
    const uint8_t source[] = {255u, 0u, 0u, 0u, 255u, 0u, 0u, 0u, 255u, 7u, 19u, 251u};
    uint8_t single_data[sizeof(source)];
    uint8_t batch_data[sizeof(source)];
    uint8_t expected[sizeof(source)];
    img_buffer_t single = {
        .data = single_data, .width = 4u, .height = 1u, .channels = 3u, .stride = sizeof(source)};
    img_buffer_t batch_buffer = single;
    img_batch_t batch = {.buffers = {&batch_buffer}, .count = 1u};
    img_pipeline_desc_t description = {0};
    img_pipeline_fused_t fused = {0};
    img_ctx_t context = {0};

    description.count = 1u;
    description.ops[0].op_code = OP_GRAYSCALE;
    memcpy(expected, source, sizeof(source));
    grayscale_reference(expected, sizeof(expected));

    if (img_pipeline_fuse(&description, &fused) != 0 || !fused.fn || !fused.fn_batch) {
        fprintf(stderr, "could not build fused grayscale pipeline\n");
        return 1;
    }

    memcpy(single_data, source, sizeof(source));
    context.fused_params = &g_fused_params;
    fused.fn(&context, &single);
    if (memcmp(single_data, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "single fused grayscale differs from RGB-luma oracle\n");
        return 1;
    }

    memcpy(batch_data, source, sizeof(source));
    batch_buffer.data = batch_data;
    img_pipeline_execute_fused_batch(&context, &fused, &batch);
    if (memcmp(batch_data, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "batch fused grayscale differs from RGB-luma oracle\n");
        return 1;
    }
    return 0;
}

/* The RPC server dispatches g_jump_table directly; prove that route separately.
 */
static int test_rpc_grayscale_dispatch(void) {
    const uint8_t source[] = {255u, 17u, 3u, 2u, 129u, 251u};
    uint8_t expected[sizeof(source)];
    uint8_t output[sizeof(source)];
    int pair[2] = {-1, -1};
    img_rpc_request_t request = {
        .op_code = OP_GRAYSCALE,
        .buffer =
            {.width = 2u, .height = 1u, .channels = 3u, .stride = 6u, .data_size = sizeof(source)},
    };
    img_rpc_response_t response = {0};
    img_ctx_t context = {0};

    memcpy(expected, source, sizeof(source));
    grayscale_reference(expected, sizeof(expected));
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0 ||
        write_all(pair[0], &request, sizeof(request)) != 0 ||
        write_all(pair[0], source, sizeof(source)) != 0 || shutdown(pair[0], SHUT_WR) != 0) {
        fprintf(stderr, "could not set up grayscale RPC dispatch test\n");
        if (pair[0] >= 0)
            close(pair[0]);
        if (pair[1] >= 0)
            close(pair[1]);
        return 1;
    }

    int server_result = img_rpc_server_process_client(&context, pair[1]);
    pair[1] = -1;
    struct pollfd ready = {.fd = pair[0], .events = POLLIN | POLLHUP};
    if (poll(&ready, 1u, 1000) <= 0) {
        fprintf(stderr, "RPC grayscale dispatch did not produce a response (server=%d)\n",
                server_result);
        close(pair[0]);
        return 1;
    }
    int read_result =
        read_all(pair[0], &response, sizeof(response)) || read_all(pair[0], output, sizeof(output));
    close(pair[0]);
    if (server_result != 0 || read_result != 0 || response.status != 0 ||
        memcmp(output, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "RPC grayscale dispatch differs from RGB-luma oracle\n");
        return 1;
    }
    return 0;
}

static int test_resize_equivalence(void) {
    if (!img_cpu_has_avx2(img_cpu_detect_caps())) {
        printf("[resize] AVX2 unavailable; resize equivalence skipped\n");
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

        img_buffer_t src = {.data = src_data,
                            .width = src_width,
                            .height = src_height,
                            .channels = 3u,
                            .stride = src_width * 3u};
        img_buffer_t scalar = {.data = scalar_data,
                               .width = dst_width,
                               .height = dst_height,
                               .channels = 3u,
                               .stride = dst_width * 3u};
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
        resize_params_t params = {.src = &src,
                                  .target_w = dst_width,
                                  .target_h = dst_height,
                                  .scale_x = scale_x,
                                  .scale_y = scale_y,
                                  .x_index = (case_index & 1u) ? x_index : NULL,
                                  .y_index = (case_index & 1u) ? y_index : NULL,
                                  .x_weight = (case_index & 1u) ? x_weight : NULL,
                                  .y_weight = (case_index & 1u) ? y_weight : NULL};

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

int main(void) {
    if (test_hardware_grayscale_dispatch() != 0 || test_registered_grayscale_dispatch() != 0 ||
        test_fused_grayscale_dispatch() != 0 || test_rpc_grayscale_dispatch() != 0)
        return 1;
    return test_resize_equivalence();
}
