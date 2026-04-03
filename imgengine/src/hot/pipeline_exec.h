/* src/hot/pipeline_exec.h */

#ifndef IMGENGINE_HOT_PIPELINE_EXEC_H
#define IMGENGINE_HOT_PIPELINE_EXEC_H

#include "core/context_internal.h"
#include "api/v1/img_types.h"

void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_desc_t *pipe,
    img_buffer_t *buf);

#endif

// #ifndef IMGENGINE_HOT_PIPELINE_EXEC_H
// #define IMGENGINE_HOT_PIPELINE_EXEC_H

// #include "core/context.h"
// #include "core/image.h"
// #include <stdint.h>
// #include "pipeline/pipeline.h"

// /**
//  * Execute pipeline (ZERO-BRANCH DISPATCH)
//  */

// /**
//  * @brief The High-Performance execution loop.
//  * No safety checks, restrict pointers, and O(1) jump table dispatch.
//  */
// void img_pipeline_execute_hot(img_ctx_t *ctx, img_pipeline_graph_t *graph, img_buffer_t *buf);

// #endif