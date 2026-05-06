// ./include/io/encoder/encoder_entry.h

// io/encoder/encoder_entry.h

#ifndef IMGENGINE_IO_ENCODER_ENTRY_H
#define IMGENGINE_IO_ENCODER_ENTRY_H

#include <stddef.h>
#include <stdint.h>

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_buffer img_buffer_t;

/**
 * Encode img_buffer into output bytes
 */
int img_encode_from_buffer_ex(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **out_data,
                              size_t *out_size, int quality, int subsamp);

int img_encode_from_buffer(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **out_data, size_t *out_size);

#endif
