/* src/hot/pipeline_exec.h */

#ifndef IMGENGINE_HOT_PIPELINE_EXEC_H
#define IMGENGINE_HOT_PIPELINE_EXEC_H

#include "core/context.h"
#include "core/image.h"
#include <stdint.h>
#include "pipeline/pipeline.h"

/**
 * Execute pipeline (ZERO-BRANCH DISPATCH)
 */
// void img_execute_pipeline(img_ctx_t *ctx,
//                           img_buffer_t *buf,
//                           uint32_t *ops,
//                           void **params,
//                           uint32_t count);

/**
 * @brief The High-Performance execution loop.
 * No safety checks, restrict pointers, and O(1) jump table dispatch.
 */
void img_pipeline_execute_hot(img_ctx_t *ctx, img_pipeline_graph_t *graph, img_buffer_t *buf);

#endif