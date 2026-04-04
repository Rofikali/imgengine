// src/cold/validation.c

#include "cold/validation.h"

// ================= HEADER VALIDATION =================

img_result_t img_validate_header(
    uint32_t w,
    uint32_t h,
    uint32_t ch)
{
    if (w == 0 || h == 0 || w > MAX_IMG_DIM || h > MAX_IMG_DIM)
        return IMG_ERR_FORMAT;

    if (ch != 3 && ch != 4)
        return IMG_ERR_FORMAT;

    return IMG_SUCCESS;
}

// ================= PIPELINE VALIDATION =================

bool img_validate_pipeline_safety(const img_pipeline_desc_t *pipe)
{
    if (!pipe)
        return false;

    if (pipe->count == 0 || pipe->count > IMG_MAX_PIPELINE_OPS)
        return false;

    for (uint32_t i = 0; i < pipe->count; i++)
    {
        uint32_t op = pipe->ops[i].op_code;

        // 🔥 STRICT RANGE
        if (op == 0 || op >= 64)
            return false;
    }

    return true;
}
