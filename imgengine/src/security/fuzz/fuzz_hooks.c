#include "security/validation/input_validator.h"
#include <stdint.h>
#include <stddef.h>

/*
 * 🔥 libFuzzer ENTRY
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 12)
        return 0;

    uint32_t w = *(const uint32_t *)(data + 0);
    uint32_t h = *(const uint32_t *)(data + 4);

    img_security_validate_request(w, h, size);

    return 0;
}