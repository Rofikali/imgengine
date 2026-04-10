#ifndef IMGENGINE_PIPELINE_BATCH_H
#define IMGENGINE_PIPELINE_BATCH_H

#include <stdint.h>

typedef struct img_buffer img_buffer_t;

#define IMG_BATCH_MAX 64

typedef struct __attribute__((aligned(64))) {
    uint32_t count;
    uint32_t _pad; // 🔥 avoid false sharing

    img_buffer_t *buffers[IMG_BATCH_MAX];

} img_batch_t;

#endif