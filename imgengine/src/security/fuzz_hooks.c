// ./src/security/fuzz_hooks.c

// ./src/security/fuzz_hooks.c



#include "security/input_validator.h"
#include <stdint.h>
#include <stddef.h>

// 🔥 AFL / libFuzzer ENTRY
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 12)
        return 0;

    uint32_t w = *(uint32_t *)(data + 0);
    uint32_t h = *(uint32_t *)(data + 4);
    uint32_t fake = *(uint32_t *)(data + 8);

    (void)fake;

    img_security_validate_request(w, h, size);

    return 0;
}