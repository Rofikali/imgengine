#ifndef IMGENGINE_PIPELINE_THREADED_H
#define IMGENGINE_PIPELINE_THREADED_H

#include <stdint.h>

typedef struct img_scheduler img_scheduler_t;
typedef struct img_pipeline_runtime img_pipeline_runtime_t;
typedef struct img_buffer img_buffer_t;

int img_pipeline_submit(
    img_scheduler_t *sched,
    img_pipeline_runtime_t *pipe,
    img_buffer_t *buf);

#endif