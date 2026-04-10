
// include/pipeline/batch.h

#ifndef IMGENGINE_PIPELINE_BATCH_H
#define IMGENGINE_PIPELINE_BATCH_H

#include <stdint.h>

// Forward declaration for external users
typedef struct img_buffer img_buffer_t;

// 🔥 REAL STRUCT (NOT OPAQUE — HOT PATH NEEDS IT)
#define IMG_BATCH_MAX 64

typedef struct img_batch
{
    img_buffer_t *buffers[IMG_BATCH_MAX];
    uint32_t count;

} img_batch_t;

// typedef struct
// {
//     uint32_t count;
//     img_buffer_t **buffers; // ✅ NOT single pointer
// } img_batch_t;

#endif