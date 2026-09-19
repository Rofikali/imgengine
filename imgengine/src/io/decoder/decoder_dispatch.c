// ./src/io/decoder/decoder_dispatch.c

#define _GNU_SOURCE

#include "io/decoder/decoder_dispatch.h"
#include "io/decoder/decoder_entry.h"
#include "io/streaming_decoder.h"
#include "io/io_vfs.h"

#include <stdlib.h>
#include <string.h>

static img_decode_strategy_t g_decode_strategy = IMG_DECODE_STRATEGY_AUTO;

static int img_is_png(const uint8_t *input, size_t size) {
    static const uint8_t signature[] = {137, 80, 78, 71, 13, 10, 26, 10};

    return size >= sizeof(signature) && memcmp(input, signature, sizeof(signature)) == 0;
}

void img_io_set_decode_strategy(img_decode_strategy_t s) { g_decode_strategy = s; }

img_result_t img_decode_dispatch(img_ctx_t *ctx, const uint8_t *input, size_t size,
                                 img_buffer_t *out) {
    if (!ctx || !input || size == 0 || !out)
        return IMG_ERR_INTERNAL;

    if (img_is_png(input, size))
        return img_decode_stb(ctx, input, size, out);

    img_decode_strategy_t mode = g_decode_strategy;
    if (mode == IMG_DECODE_STRATEGY_AUTO)
        mode = IMG_DECODE_STRATEGY_BULK;

    if (mode == IMG_DECODE_STRATEGY_BULK) {
        return (img_result_t)img_decode_to_buffer(ctx, input, size, out);
    }

    /* STREAM mode: adapt in-memory buffer into an img_stream and call streaming path */
    img_stream_t stream = {0};
    img_result_t rc = img_vfs_open_mem(&stream, input, size);
    if (rc != IMG_SUCCESS)
        return rc;

    return img_decode_stream(ctx, &stream, out);
}
