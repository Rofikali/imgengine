// runtime/per_core.h

#ifndef IMGENGINE_PER_CORE_H
#define IMGENGINE_PER_CORE_H

#include <stdint.h>
#include "hot/pipeline_threaded.h"
#include "hot/batch_exec.h"

#define MAX_CORES 64

/*
 * 🔥 FUSED BATCH KERNEL
 */
typedef void (*img_fused_batch_kernel_fn)(
    img_ctx_t *,
    img_batch_t *,
    void *);

typedef struct
{
    img_threaded_program_t program;
    img_fused_batch_kernel_fn fused;

} img_core_pipeline_t;

extern img_core_pipeline_t g_core_pipelines[MAX_CORES];

#endif
