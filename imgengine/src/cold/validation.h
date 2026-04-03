
/* src/cold/validation.h */

#ifndef IMGENGINE_COLD_VALIDATION_H
#define IMGENGINE_COLD_VALIDATION_H

#include <stdbool.h>
#include <stdint.h>

#include "api/v1/img_error.h"
#include "api/v1/img_types.h"
#include "pipeline/pipeline.h"

// ================= LIMITS =================

#define MAX_IMG_DIM 16384

// ================= API =================

img_result_t img_validate_header(
    uint32_t w,
    uint32_t h,
    uint32_t channels);

/**
 * @brief Validates pipeline before execution (cold path)
 */
// bool img_validate_pipeline_safety(const img_pipeline_t *pipe);

bool img_validate_pipeline_safety(const img_pipeline_desc_t *pipe);

#endif

// #ifndef IMGENGINE_COLD_VALIDATION_H
// #define IMGENGINE_COLD_VALIDATION_H

// #include <stdbool.h>
// #include <stdint.h>
// #include "api/v1/img_error.h"
// #include "api/v1/img_types.h"
// #include "pipeline/pipeline.h" // CRITICAL: Defines img_pipeline_graph_t

// // Hard limits to prevent OOM/DoS (64MB Slab limit)
// #define MAX_IMG_DIM 16384

// img_result_t img_validate_header(uint32_t w, uint32_t h, uint32_t channels);

// /**
//  * @brief Checks the DAG for infinite loops or invalid OpCodes
//  * before it hits the Hot Path.
//  */
// bool img_validate_pipeline_safety(const img_pipeline_graph_t *pipe);

// #endif
