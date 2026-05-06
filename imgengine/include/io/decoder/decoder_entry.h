// ./include/io/decoder/decoder_entry.h

// io/decoder/decoder_entry.h

#ifndef IMGENGINE_IO_DECODER_ENTRY_H
#define IMGENGINE_IO_DECODER_ENTRY_H

#include <stddef.h>
#include <stdint.h>

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

int img_decode_to_buffer(img_ctx_t *ctx,
                         const uint8_t *input, // ✅ FIXED
                         size_t size, img_buffer_t *out);

#endif