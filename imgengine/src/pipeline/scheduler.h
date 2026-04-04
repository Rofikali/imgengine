/* pipeline/scheduler.h */
#ifndef IMGENGINE_PIPELINE_SCHEDULER_H
#define IMGENGINE_PIPELINE_SCHEDULER_H

#include "pipeline/pipeline.h"
// #include "hot/batch.h"

/**
 * Dispatch batch pipeline execution
 */
void img_scheduler_dispatch_batch(img_ctx_t *ctx,
                                  img_batch_t *batch,
                                  img_pipeline_graph_t *graph);

#endif