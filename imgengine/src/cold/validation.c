
// src/cold/validation.c

#include "cold/validation.h"

img_result_t img_validate_header(
    uint32_t w,
    uint32_t h,
    uint32_t ch)
{
    if (w == 0 || h == 0 || w > 16384 || h > 16384)
        return IMG_ERR_FORMAT;

    if (ch != 3 && ch != 4)
        return IMG_ERR_FORMAT;

    return IMG_SUCCESS;
}