// ./src/cold/validation.c

// src/cold/validation.c

#include "pipeline/pipeline_types.h"
#include "pipeline/pipeline_compiled.h"
#include "cold/validation.h"
#include "core/opcodes.h"
#include <limits.h>

// #define MAX_PIXELS (16384ULL * 16384ULL)
// #define MAX_CHANNELS 4

img_result_t img_validate_header(uint32_t w, uint32_t h, uint32_t ch) {
    if (w == 0 || h == 0)
        return IMG_ERR_FORMAT;

    if (w > MAX_IMG_DIM || h > MAX_IMG_DIM)
        return IMG_ERR_SECURITY;

    if (w > 0 && h > UINT32_MAX / w)
        return IMG_ERR_SECURITY;

    uint64_t pixels = (uint64_t)w * h;

    if (pixels > MAX_PIXELS)
        return IMG_ERR_SECURITY;

    if (ch == 0 || ch > MAX_CHANNELS)
        return IMG_ERR_FORMAT;

    return IMG_SUCCESS;
}

bool img_validate_pipeline_safety(const img_pipeline_desc_t *pipe) {
    if (!pipe)
        return false;

    if (pipe->count == 0 || pipe->count > IMG_MAX_PIPELINE_OPS)
        return false;

    for (uint32_t i = 0; i < pipe->count; i++) {
        uint32_t op = pipe->ops[i].op_code;

        if (op == 0 || op >= OP_CUSTOM_BASE)
            return false;
    }

    return true;
}