// ./include/io/decoder/decoder_dispatch.h
#ifndef IMGENGINE_DECODER_DISPATCH_H
#define IMGENGINE_DECODER_DISPATCH_H

#include <stddef.h>
#include <stdint.h>
#include "core/result.h"

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_stream img_stream_t;

typedef enum {
    IMG_DECODE_STRATEGY_AUTO = 0,
    IMG_DECODE_STRATEGY_BULK = 1,
    IMG_DECODE_STRATEGY_STREAM = 2,
} img_decode_strategy_t;

/* Dispatch entry used as the default decode function in the vtable. */
img_result_t img_decode_dispatch(
    img_ctx_t *ctx,
    const uint8_t *input,
    size_t size,
    img_buffer_t *out);

/* Set runtime strategy (callable after engine init). */
void img_io_set_decode_strategy(img_decode_strategy_t s);

#endif /* IMGENGINE_DECODER_DISPATCH_H */
