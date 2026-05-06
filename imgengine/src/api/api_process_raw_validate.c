// ./src/api/api_process_raw_validate.c
#include "api/api_process_raw_internal.h"
#include "security/input_validator.h"

img_result_t img_api_process_raw_validate(const img_engine_t *engine, const uint8_t *input,
                                          uint8_t **output, size_t *output_size,
                                          size_t input_size) {
    if (!engine || !input || !output || !output_size)
        return IMG_ERR_SECURITY;

    *output = NULL;
    *output_size = 0;

    return img_security_validate_request(4096, 4096, input_size);
}
