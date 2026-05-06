// ./include/io/io_vtable.h
#ifndef IMGENGINE_IO_VTABLE_H
#define IMGENGINE_IO_VTABLE_H

#include <stddef.h>
#include <stdint.h>
#include "core/result.h"

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

typedef img_result_t (*img_decode_fn)(img_ctx_t *ctx, const uint8_t *input, size_t size,
                                      img_buffer_t *out);

typedef img_result_t (*img_encode_fn)(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **out_data,
                                      size_t *out_size);

typedef img_result_t (*img_encode_pdf_fn)(const img_buffer_t *buf, const char *path, uint32_t dpi);

typedef struct {
    img_decode_fn decode;
    img_encode_fn encode;
    img_encode_pdf_fn encode_pdf;
} img_io_vtable_t;

extern img_io_vtable_t g_io_vtable;

void img_io_register(img_decode_fn decode, img_encode_fn encode, img_encode_pdf_fn encode_pdf);

#endif /* IMGENGINE_IO_VTABLE_H */
