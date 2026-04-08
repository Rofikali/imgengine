#include "security/validation/input_validator.h"
#include "security/bounds/bounds_check.h"

#define IMG_MAX_DIM 16384
#define IMG_MAX_PIXELS (16384ULL * 16384ULL)
#define IMG_MAX_RATIO 200ULL
#define IMG_MAX_MEMORY (1ULL << 32) // 4GB

img_result_t img_security_validate_request(
    uint32_t w,
    uint32_t h,
    size_t file_size)
{
    // =========================
    // BASIC STRUCTURAL VALIDATION
    // =========================
    if (IMG_UNLIKELY(w == 0 || h == 0))
        return IMG_ERR_SECURITY;

    if (IMG_UNLIKELY(w > IMG_MAX_DIM || h > IMG_MAX_DIM))
        return IMG_ERR_SECURITY;

    // =========================
    // PIXEL COUNT SAFE
    // =========================
    uint64_t pixels = (uint64_t)w * (uint64_t)h;

    if (IMG_UNLIKELY(pixels == 0 || pixels > IMG_MAX_PIXELS))
        return IMG_ERR_SECURITY;

    // =========================
    // FILE SIZE DEFENSE
    // =========================
    if (IMG_UNLIKELY(file_size == 0))
        return IMG_ERR_SECURITY;

    // Compression bomb protection
    if (IMG_UNLIKELY(pixels > file_size * IMG_MAX_RATIO))
        return IMG_ERR_SECURITY;

    // =========================
    // MEMORY ESTIMATION
    // =========================
    uint64_t required;

    if (!img_safe_mul(pixels, 4, &required))
        return IMG_ERR_SECURITY;

    if (IMG_UNLIKELY(required > IMG_MAX_MEMORY))
        return IMG_ERR_SECURITY;

    return IMG_SUCCESS;
}