// io/decoder/decoder_entry.h

#ifndef IMGENGINE_IO_DECODER_ENTRY_H
#define IMGENGINE_IO_DECODER_ENTRY_H

#include <stddef.h>
#include <stdint.h>
#include "core/context_internal.h"

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

/**
 * Decode input bytes into img_buffer (ZERO-COPY into slab)
 */
int img_decode_to_buffer(img_ctx_t *ctx,
                         uint8_t *input,
                         size_t size,
                         img_buffer_t *out);

#endif