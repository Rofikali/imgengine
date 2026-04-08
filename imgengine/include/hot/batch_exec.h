// include/hot/batch_exec.h

#ifndef IMGENGINE_BATCH_EXEC_H
#define IMGENGINE_BATCH_EXEC_H

#include "core/context_internal.h"
#include "pipeline/pipeline_types.h"
#include "hot/batch_exec.h"

#include <stdint.h>

// Forward declaration for external users
typedef struct img_pipeline_runtime img_pipeline_runtime_t;
typedef struct img_buffer img_buffer_t;

// 🔥 REAL STRUCT (NOT OPAQUE — HOT PATH NEEDS IT)
#define IMG_BATCH_MAX 64

typedef struct img_batch
{
    img_buffer_t *buffers[IMG_BATCH_MAX];
    uint32_t count;

} img_batch_t;
void img_batch_execute(
    img_ctx_t *__restrict ctx,
    img_batch_t *__restrict batch,
    const img_pipeline_runtime_t *__restrict pipe);

#endif