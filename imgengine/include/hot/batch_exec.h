
// include/hot/batch_exec.h

#ifndef IMGENGINE_HOT_BATCH_EXEC_H
#define IMGENGINE_HOT_BATCH_EXEC_H

#include <stdint.h>

// forward declarations
typedef struct img_ctx img_ctx_t;
typedef struct img_batch img_batch_t;
typedef struct img_pipeline_runtime img_pipeline_runtime_t;

// 🔥 HOT BATCH EXEC
void img_batch_execute(
    img_ctx_t *__restrict ctx,
    img_batch_t *__restrict batch,
    const img_pipeline_runtime_t *__restrict pipe);

#endif