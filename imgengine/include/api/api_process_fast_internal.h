// ./include/api/api_process_fast_internal.h
#ifndef IMGENGINE_API_PROCESS_FAST_INTERNAL_H
#define IMGENGINE_API_PROCESS_FAST_INTERNAL_H

#include "api/api_internal.h"
#include "core/buffer.h"

img_result_t decode_image_secure(img_engine_t *engine, const uint8_t *input, size_t size,
                                 img_buffer_t *out_buf);

img_result_t img_api_process_fast_validate(img_engine_t *engine, const uint8_t *input,
                                           img_pipeline_desc_t *pipe, img_buffer_t *out_buf,
                                           size_t input_size);

img_result_t img_api_process_fast_decode(img_engine_t *engine, const uint8_t *input,
                                         size_t input_size, img_buffer_t *out_buf);

#endif /* IMGENGINE_API_PROCESS_FAST_INTERNAL_H */
