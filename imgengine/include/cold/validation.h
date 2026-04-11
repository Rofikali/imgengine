// ./include/cold/validation.h

/* src/cold/validation.h */

#ifndef IMGENGINE_COLD_VALIDATION_H
#define IMGENGINE_COLD_VALIDATION_H

#include <stdbool.h>
#include <stdint.h>
#include "api/v1/img_error.h"

// forward
typedef struct img_pipeline_desc img_pipeline_desc_t;

// 🔥 LIMITS (DOS SAFE)
#define MAX_IMG_DIM 16384
#define MAX_PIXELS (MAX_IMG_DIM * MAX_IMG_DIM)
#define MAX_CHANNELS 4

img_result_t img_validate_header(
    uint32_t w,
    uint32_t h,
    uint32_t channels);

// 🔥 STRICT VALIDATION
bool img_validate_pipeline_safety(
    const img_pipeline_desc_t *pipe);

#endif