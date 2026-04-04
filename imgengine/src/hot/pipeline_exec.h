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
