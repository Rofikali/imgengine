#define _GNU_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "api/api_internal.h"
#include "api/api_job_internal.h"
#include "api/v1/img_api.h"
#include "pipeline/layout_render_private.h"
#include "runtime/job_exec.h"

static const uint8_t k_png_fixture[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
    0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25,
    0xdb, 0x56, 0xca, 0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0x00, 0x00, 0xff,
    0xff, 0xff, 0x41, 0x1d, 0x34, 0x11, 0x00, 0x00, 0x00, 0x0a, 0x49, 0x44, 0x41, 0x54, 0x08,
    0xd7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xe2, 0x21, 0xbc, 0x33, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

typedef struct {
    uint8_t *data;
    size_t mapping_size;
} guarded_input_t;

static int guarded_input_init(guarded_input_t *input, const uint8_t *bytes, size_t size) {
    const long page_size = sysconf(_SC_PAGESIZE);
    if (!input || !bytes || size == 0 || page_size <= 0 || size > (size_t)page_size)
        return 0;
    input->data =
        mmap(NULL, (size_t)page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (input->data == MAP_FAILED) {
        input->data = NULL;
        return 0;
    }
    input->mapping_size = (size_t)page_size;
    memcpy(input->data, bytes, size);
    return 1;
}

static int guarded_input_is_live(const guarded_input_t *input) {
    unsigned char resident = 0;
    if (!input || !input->data)
        return 0;
    errno = 0;
    return mincore(input->data, input->mapping_size, &resident) == 0;
}

static int guarded_input_matches(const guarded_input_t *input, const uint8_t *expected,
                                 size_t expected_size) {
    return guarded_input_is_live(input) && memcmp(input->data, expected, expected_size) == 0;
}

static void guarded_input_destroy(guarded_input_t *input) {
    if (!input || !input->data)
        return;
    munmap(input->data, input->mapping_size);
    input->data = NULL;
    input->mapping_size = 0;
}

static int test_prepared_decoder_borrows_success(img_engine_t *engine) {
    guarded_input_t input = {0};
    img_prepared_decoder_t decoder = {0};
    img_buffer_t decoded = {0};
    const int initialized = guarded_input_init(&input, k_png_fixture, sizeof(k_png_fixture));
    const img_result_t prepared =
        initialized ? img_api_prepare_decoder(engine, &decoder) : IMG_ERR_NOMEM;
    const img_result_t status =
        prepared == IMG_SUCCESS
            ? img_api_decode_prepared(&decoder, input.data, sizeof(k_png_fixture), &decoded)
            : prepared;
    const int passed = status == IMG_SUCCESS && decoded.data &&
                       guarded_input_matches(&input, k_png_fixture, sizeof(k_png_fixture));
    img_runtime_release_raw_buffer(engine, &decoded);
    img_api_decoder_destroy(&decoder);
    guarded_input_destroy(&input);
    return passed;
}

static int test_prepared_decoder_borrows_failure(img_engine_t *engine) {
    static const uint8_t malformed[] = {0x89, 0x50, 0x4e, 0x47, 0x00, 0x01, 0x02};
    guarded_input_t input = {0};
    img_prepared_decoder_t decoder = {0};
    img_buffer_t decoded = {0};
    const int initialized = guarded_input_init(&input, malformed, sizeof(malformed));
    const img_result_t prepared =
        initialized ? img_api_prepare_decoder(engine, &decoder) : IMG_ERR_NOMEM;
    const img_result_t status =
        prepared == IMG_SUCCESS
            ? img_api_decode_prepared(&decoder, input.data, sizeof(malformed), &decoded)
            : prepared;
    const int passed = status != IMG_SUCCESS && decoded.data == NULL &&
                       guarded_input_matches(&input, malformed, sizeof(malformed));
    img_runtime_release_raw_buffer(engine, &decoded);
    img_api_decoder_destroy(&decoder);
    guarded_input_destroy(&input);
    return passed;
}

static int test_internal_raw_borrows_input(img_engine_t *engine) {
    guarded_input_t input = {0};
    uint8_t *output = NULL;
    size_t output_size = 0;
    if (!guarded_input_init(&input, k_png_fixture, sizeof(k_png_fixture)))
        return 0;
    const img_result_t status =
        img_api_process_raw(engine, input.data, sizeof(k_png_fixture), &output, &output_size);
    const int passed = status == IMG_SUCCESS && output && output_size > 0 &&
                       guarded_input_matches(&input, k_png_fixture, sizeof(k_png_fixture));
    img_encoded_free(output);
    guarded_input_destroy(&input);
    return passed;
}

static int test_queued_render_keeps_only_decoded_owned_data(img_engine_t *engine) {
    const img_layout_render_rect_t placement = {.x = 0, .y = 0, .width = 2, .height = 2};
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 2,
        .canvas_height = 2,
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };
    guarded_input_t input = {0};
    img_layout_render_v1_result_t result = {0};
    if (!guarded_input_init(&input, k_png_fixture, sizeof(k_png_fixture)))
        return 0;
    const img_result_t status =
        img_layout_render_v1(engine, input.data, sizeof(k_png_fixture), &request, &result);
    const int passed = status == IMG_SUCCESS && result.jpeg_data && result.jpeg_size > 0 &&
                       guarded_input_matches(&input, k_png_fixture, sizeof(k_png_fixture));
    img_layout_render_v1_result_release(&result);
    guarded_input_destroy(&input);
    return passed;
}

static int test_queued_render_failure_keeps_borrow(img_engine_t *engine) {
    static const uint8_t malformed[] = {0x89, 0x50, 0x4e, 0x47, 0x00, 0x01, 0x02};
    const img_layout_render_rect_t placement = {.x = 0, .y = 0, .width = 2, .height = 2};
    const img_layout_render_v1_request_t request = {
        .version = IMG_LAYOUT_RENDER_V1_PRIVATE_VERSION,
        .canvas_width = 2,
        .canvas_height = 2,
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };
    guarded_input_t input = {0};
    img_layout_render_v1_result_t result = {0};
    if (!guarded_input_init(&input, malformed, sizeof(malformed)))
        return 0;
    const img_result_t status =
        img_layout_render_v1(engine, input.data, sizeof(malformed), &request, &result);
    const int passed = status != IMG_SUCCESS && result.jpeg_data == NULL && result.jpeg_size == 0 &&
                       guarded_input_matches(&input, malformed, sizeof(malformed));
    img_layout_render_v1_result_release(&result);
    guarded_input_destroy(&input);
    return passed;
}

static int test_guard_detects_legacy_mmap_release(void) {
    guarded_input_t input = {0};
    if (!guarded_input_init(&input, k_png_fixture, sizeof(k_png_fixture)))
        return 0;
    if (!guarded_input_is_live(&input)) {
        guarded_input_destroy(&input);
        return 0;
    }
    const img_result_t status = img_api_release_input(input.data, input.mapping_size);
    const int passed = status == IMG_SUCCESS && !guarded_input_is_live(&input);
    input.data = NULL;
    input.mapping_size = 0;
    return passed;
}

int main(void) {
    img_engine_t *engine = img_api_init(1);
    if (!engine) {
        fprintf(stderr, "engine initialization failed\n");
        return 1;
    }
    const int passed =
        test_prepared_decoder_borrows_success(engine) &&
        test_prepared_decoder_borrows_failure(engine) && test_internal_raw_borrows_input(engine) &&
        test_queued_render_keeps_only_decoded_owned_data(engine) &&
        test_queued_render_failure_keeps_borrow(engine) && test_guard_detects_legacy_mmap_release();
    img_api_shutdown(engine);
    if (!passed) {
        fprintf(stderr, "borrowed-input ownership invariant failed\n");
        return 1;
    }
    printf("[ownership] borrowed decoder, ABI, queued render, failure, and legacy mmap guard "
           "passed\n");
    return 0;
}
