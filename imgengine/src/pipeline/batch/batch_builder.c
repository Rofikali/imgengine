#include "pipeline/batch/batch.h"

void img_batch_add(img_batch_t *b, img_buffer_t *buf)
{
    if (b->count < IMG_BATCH_MAX)
        b->items[b->count++] = buf;
}

void img_batch_reset(img_batch_t *b)
{
    b->count = 0;
}