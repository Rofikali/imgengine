// pipeline/batch.h

#ifndef IMGENGINE_PIPELINE_BATCH_H
#define IMGENGINE_PIPELINE_BATCH_H

#include "api/v1/img_types.h"

#define IMG_BATCH_MAX 64

typedef struct
{
    img_buffer_t *buffers[IMG_BATCH_MAX];
    uint32_t count;
} img_batch_t;

#endif