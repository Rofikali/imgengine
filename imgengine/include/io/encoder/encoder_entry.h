// io/encoder/encoder_entry.h

#ifndef IMGENGINE_IO_ENCODER_ENTRY_H
#define IMGENGINE_IO_ENCODER_ENTRY_H

#include <stddef.h>
#include <stdint.h>
#include "core/context_internal.h"
#include "api/v1/img_types.h"

/**
 * Encode img_buffer into output bytes
 */
int img_encode_from_buffer(img_ctx_t *ctx,
                           img_buffer_t *buf,
                           uint8_t **out_data,
                           size_t *out_size);

#endif