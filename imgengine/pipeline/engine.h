/* pipeline/engine.h */

#ifndef IMGENGINE_PIPELINE_ENGINE_H
#define IMGENGINE_PIPELINE_ENGINE_H

#include "core/context_internal.h"
#include "api/v1/img_types.h"

// void img_pipeline_execute(
//     img_ctx_t *ctx,
//     img_pipeline_desc_t *pipe,
//     img_buffer_t *buf);

void img_pipeline_execute(
    img_ctx_t *ctx,
    img_pipeline_t *pipe,
    img_buffer_t *buf);

#endif
