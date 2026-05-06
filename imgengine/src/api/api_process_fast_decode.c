// ./src/api/api_process_fast_decode.c
#include "api/api_process_fast_internal.h"

img_result_t img_api_process_fast_decode(img_engine_t *engine, const uint8_t *input,
                                         size_t input_size, img_buffer_t *out_buf) {
    return decode_image_secure(engine, input, input_size, out_buf);
}
