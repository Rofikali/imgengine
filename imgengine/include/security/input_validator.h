
/* security/input_validator.h */
#ifndef IMGENGINE_SECURITY_VALIDATOR_H
#define IMGENGINE_SECURITY_VALIDATOR_H

#include "api/v1/img_error.h"
#include <stdint.h>
#include <stddef.h>

img_result_t img_security_validate_request(uint32_t w, uint32_t h, size_t file_size);

#endif
