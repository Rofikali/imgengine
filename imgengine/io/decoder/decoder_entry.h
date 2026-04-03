// io/decoder/decoder_entry.h

#ifndef IMGENGINE_IO_DECODER_ENTRY_H
#define IMGENGINE_IO_DECODER_ENTRY_H

#include <stddef.h>
#include <stdint.h>
#include "core/context_internal.h"
#include "api/v1/img_types.h"
// #include "api/v1/img_buffer_utils.h"

/**
 * Decode input bytes into img_buffer (ZERO-COPY into slab)
 */
int img_decode_to_buffer(img_ctx_t *ctx,
                         uint8_t *input,
                         size_t size,
                         img_buffer_t *out);

#endif