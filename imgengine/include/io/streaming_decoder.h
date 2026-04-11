// ./include/io/streaming_decoder.h

// include/io/streaming_decoder.h

#ifndef IMGENGINE_STREAMING_DECODER_H
#define IMGENGINE_STREAMING_DECODER_H

#include <stddef.h>
#include <stdint.h>

// Forward declarations
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_stream img_stream_t;

// 🔥 Decode from stream (NOT full buffer)
int img_decode_stream(
    img_ctx_t *ctx,
    img_stream_t *stream,
    img_buffer_t *out);

#endif