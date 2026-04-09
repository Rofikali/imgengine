#ifndef IMGENGINE_BATCH_H
#define IMGENGINE_BATCH_H

#include <stdint.h>

#define IMG_BATCH_MAX 64

typedef struct img_buffer img_buffer_t;

typedef struct
{
    img_buffer_t *items[IMG_BATCH_MAX];
    uint32_t count;

} img_batch_t;

#endif