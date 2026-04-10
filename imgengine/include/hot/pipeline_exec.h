
/* src/hot/pipeline_exec.h */
#ifndef IMGENGINE_HOT_PIPELINE_EXEC_H
#define IMGENGINE_HOT_PIPELINE_EXEC_H

// #include "core/context_internal.h"
// #include <stdint.h>

// 🔥 STRICT RUNTIME PIPE (NO ABI CONFUSION)
typedef struct img_pipeline_runtime img_pipeline_runtime_t;

// forward declarations
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

// 🔥 HOT EXEC (L8 CRITICAL)
void img_pipeline_execute_hot(
    img_ctx_t *__restrict ctx,
    const img_pipeline_runtime_t *__restrict pipe,
    img_buffer_t *__restrict buf);

#endif
