// ./src/api/api_process_fast_validate.c
#include "api/api_process_fast_internal.h"
#include "security/input_validator.h"

img_result_t img_api_process_fast_validate(img_engine_t *engine, const uint8_t *input,
                                           img_pipeline_desc_t *pipe, img_buffer_t *out_buf,
                                           size_t input_size) {
    if (!engine || !input || !pipe || !out_buf)
        return IMG_ERR_SECURITY;

    *out_buf = (img_buffer_t){0};
    return img_security_validate_request(4096, 4096, input_size);
}
