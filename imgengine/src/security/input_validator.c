// ./src/security/input_validator.c

// ./src/security/input_validator.c



#include "security/input_validator.h"

#include <stddef.h>

#define MAX_PIXELS (16384ULL * 16384ULL)
#define MAX_RATIO 400ULL // tighter than before

img_result_t img_security_validate_request(
    uint32_t w,
    uint32_t h,
    size_t file_size)
{
    // ================================
    // BASIC CHECK
    // ================================
    if (w == 0 || h == 0)
        return IMG_ERR_SECURITY;

    if (w > 16384 || h > 16384)
        return IMG_ERR_SECURITY;

    // ================================
    // OVERFLOW SAFE PIXEL COUNT
    // ================================
    uint64_t pixels = (uint64_t)w * (uint64_t)h;

    if (pixels == 0 || pixels > MAX_PIXELS)
        return IMG_ERR_SECURITY;

    // ================================
    // COMPRESSION BOMB DEFENSE
    // ================================
    if (file_size == 0)
        return IMG_ERR_SECURITY;

    if (pixels > file_size * MAX_RATIO)
        return IMG_ERR_SECURITY;

    // ================================
    // STRIDE / MEMORY CHECK (ESTIMATE)
    // ================================
    uint64_t required = pixels * 4; // RGBA worst-case

    if (required > (1ULL << 32)) // 4GB hard cap
        return IMG_ERR_SECURITY;

    return IMG_SUCCESS;
}