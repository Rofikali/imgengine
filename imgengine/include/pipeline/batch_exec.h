
// include/pipeline/batch_exec.h 

#ifndef IMGENGINE_BATCH_EXEC_H
#define IMGENGINE_BATCH_EXEC_H

#include "core/context_internal.h"
#include "pipeline/batch.h"
// #include "api/v1/img_types.h"

typedef void (*img_fused_kernel_fn)(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *params);

void img_batch_execute(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *pipeline);

#endif