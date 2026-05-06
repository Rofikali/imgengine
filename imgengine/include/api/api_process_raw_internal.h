// ./include/api/api_process_raw_internal.h
#ifndef IMGENGINE_API_PROCESS_RAW_INTERNAL_H
#define IMGENGINE_API_PROCESS_RAW_INTERNAL_H

#include "api/api_internal.h"
#include "core/buffer.h"

#include <stddef.h>
#include <stdint.h>

img_result_t img_api_process_raw_validate(const img_engine_t *engine, const uint8_t *input,
                                          uint8_t **output, size_t *output_size, size_t input_size);

img_result_t img_api_process_raw_prepare(const img_engine_t *engine, const uint8_t *input,
                                         uint8_t **output, size_t *output_size, size_t input_size,
                                         img_ctx_t *ctx);

img_result_t img_api_process_raw_execute(img_engine_t *engine, img_ctx_t *ctx, const uint8_t *input,
                                         size_t input_size, img_buffer_t *buf, uint8_t **output,
                                         size_t *output_size);

void img_api_process_raw_free_buffer(img_engine_t *engine, img_buffer_t *buf);

img_result_t img_api_process_raw_decode(img_ctx_t *ctx, const uint8_t *input, size_t input_size,
                                        img_buffer_t *buf);

img_result_t img_api_process_raw_encode(img_ctx_t *ctx, img_buffer_t *buf, uint8_t **output,
                                        size_t *output_size);

#endif /* IMGENGINE_API_PROCESS_RAW_INTERNAL_H */
