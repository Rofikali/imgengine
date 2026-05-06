// ./src/io/decoder/decoder_dispatch.c

#define _GNU_SOURCE

#include "io/decoder/decoder_dispatch.h"
#include "io/decoder/decoder_entry.h"
#include "io/streaming_decoder.h"
#include "io/io_vfs.h"

#include <stdlib.h>
#include <string.h>

/* Default threshold for choosing streaming vs bulk when in AUTO mode */
#define IMG_DECODE_AUTO_THRESHOLD (128 * 1024)

static img_decode_strategy_t g_decode_strategy = IMG_DECODE_STRATEGY_AUTO;

void img_io_set_decode_strategy(img_decode_strategy_t s) { g_decode_strategy = s; }

img_result_t img_decode_dispatch(img_ctx_t *ctx, const uint8_t *input, size_t size,
                                 img_buffer_t *out) {
    if (!ctx || !input || size == 0 || !out)
        return IMG_ERR_INTERNAL;

    img_decode_strategy_t mode = g_decode_strategy;
    if (mode == IMG_DECODE_STRATEGY_AUTO) {
        mode = (size > IMG_DECODE_AUTO_THRESHOLD) ? IMG_DECODE_STRATEGY_STREAM
                                                  : IMG_DECODE_STRATEGY_BULK;
    }

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
