// ./include/hot/batch_exec.h

#ifndef IMGENGINE_HOT_BATCH_EXEC_H
#define IMGENGINE_HOT_BATCH_EXEC_H

#include <stdint.h>

typedef struct img_ctx img_ctx_t;
typedef struct img_batch img_batch_t;
typedef struct img_pipeline_runtime img_pipeline_runtime_t;

/*
 * img_batch_execute_rt()
 *
 * Hot path: batch execution via compiled runtime pipeline.
 * Iterates jump table per op — used when pipeline is NOT fused.
 * Called by worker_loop() for standard (non-fused) pipelines.
 */
void img_batch_execute_rt(img_ctx_t *__restrict ctx, img_batch_t *__restrict batch,
                          const img_pipeline_runtime_t *__restrict pipe);

#endif /* IMGENGINE_HOT_BATCH_EXEC_H */
