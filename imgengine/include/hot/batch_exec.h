// include/hot/batch_exec.h

#ifndef IMGENGINE_HOT_BATCH_EXEC_H
#define IMGENGINE_HOT_BATCH_EXEC_H

#include "pipeline/batch.h"
#include "core/context_internal.h"
#include "api/v1/img_types.h"

void img_batch_execute(
    img_ctx_t *ctx,
    img_batch_t *batch,
    img_pipeline_desc_t *pipe);

#endif