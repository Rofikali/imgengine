// ./include/io/streaming_decoder_internal.h
#ifndef IMGENGINE_STREAMING_DECODER_INTERNAL_H
#define IMGENGINE_STREAMING_DECODER_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include "core/result.h"

typedef struct img_slab_pool img_slab_pool_t;
typedef struct img_stream img_stream_t;
typedef struct img_buffer img_buffer_t;

img_result_t img_decode_stream_core(
    img_slab_pool_t *pool,
    img_stream_t *stream,
    img_buffer_t *out);

#endif /* IMGENGINE_STREAMING_DECODER_INTERNAL_H */
