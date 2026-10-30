#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <turbojpeg.h>

#include "core/buffer.h"
#include "core/context_internal.h"
#include "io/encoder/encoder_entry.h"

/*
 * This is deliberately a link-time test-only failure injection.  It reaches
 * the bounded encoder's destination malloc without creating a production
 * configuration switch or changing the public ABI.
 */
static atomic_bool g_fail_next_malloc = false;

void *__real_malloc(size_t size);

void *__wrap_malloc(size_t size) {
    if (atomic_exchange_explicit(&g_fail_next_malloc, false, memory_order_acq_rel))
        return NULL;
    return __real_malloc(size);
}

static int output_is_empty(const uint8_t *data, size_t size) { return data == NULL && size == 0; }

int main(void) {
    uint8_t pixels[] = {17, 31, 47};
    img_ctx_t context = {0};
    img_buffer_t buffer = {
        .data = pixels,
        .width = 1,
        .height = 1,
        .channels = 3,
        .stride = 3,
    };
    const unsigned long bound = tjBufSize(1, 1, TJSAMP_444);
    uint8_t *output = (uint8_t *)(uintptr_t)1;
    size_t output_size = 1;

    if (bound == 0 || bound > SIZE_MAX) {
        fprintf(stderr, "invalid TurboJPEG bound\n");
        return 1;
    }
    if (img_encode_from_buffer_bounded(&context, &buffer, &output, &output_size, 0) !=
            IMG_ERR_SECURITY ||
        !output_is_empty(output, output_size)) {
        fprintf(stderr, "zero bounded-output budget was not failure-atomic\n");
        return 1;
    }
    output = (uint8_t *)(uintptr_t)1;
    output_size = 1;
    if (img_encode_from_buffer_bounded(&context, &buffer, &output, &output_size,
                                       (size_t)bound - 1u) != IMG_ERR_SECURITY ||
        !output_is_empty(output, output_size)) {
        fprintf(stderr, "under-bound output budget was not failure-atomic\n");
        return 1;
    }

    atomic_store_explicit(&g_fail_next_malloc, true, memory_order_release);
    output = (uint8_t *)(uintptr_t)1;
    output_size = 1;
    if (img_encode_from_buffer_bounded(&context, &buffer, &output, &output_size, (size_t)bound) !=
            IMG_ERR_NOMEM ||
        !output_is_empty(output, output_size)) {
        fprintf(stderr, "bounded-output allocation failure was not failure-atomic\n");
        return 1;
    }

    if (img_encode_from_buffer_bounded(&context, &buffer, &output, &output_size, (size_t)bound) !=
            IMG_SUCCESS ||
        !output || output_size == 0 || output_size > (size_t)bound || output[0] != 0xff ||
        output[1] != 0xd8) {
        fprintf(stderr, "bounded encoder did not recover after allocation failure\n");
        free(output);
        return 1;
    }
    free(output);
    img_encoder_release_thread();
    return 0;
}
