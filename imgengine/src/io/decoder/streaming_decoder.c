// ./src/io/decoder/streaming_decoder.c
#include "io/streaming_decoder.h"
#include "io/streaming_decoder_internal.h"
#include "core/context_internal.h"

img_result_t img_decode_stream(img_ctx_t *ctx, img_stream_t *stream, img_buffer_t *out) {
    if (!ctx || !stream || !out)
        return IMG_ERR_INTERNAL;

    return img_decode_stream_core(ctx->local_pool, stream, out);
}
