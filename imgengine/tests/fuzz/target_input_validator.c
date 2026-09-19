#include <stddef.h>
#include <stdint.h>

#include "security/input_validator.h"

static uint32_t read_u32_le(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 8)
        return 0;

    uint32_t width = read_u32_le(data);
    uint32_t height = read_u32_le(data + 4);
    size_t file_size = size;
    if (size >= 12)
        file_size = (size_t)read_u32_le(data + 8);
    if (size >= 16 && sizeof(size_t) >= sizeof(uint64_t))
        file_size |= (size_t)((uint64_t)read_u32_le(data + 12) << 32);

    (void)img_security_validate_request(width, height, file_size);
    return 0;
}
