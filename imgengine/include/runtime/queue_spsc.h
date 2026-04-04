/* runtime/queue_spsc.h */
#ifndef IMGENGINE_RUNTIME_QUEUE_SPSC_H
#define IMGENGINE_RUNTIME_QUEUE_SPSC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct img_queue img_queue_t;

/**
 * Capacity = 2^power
 */
img_queue_t *img_queue_create(uint32_t power);

/**
 * Non-blocking push
 */
bool img_queue_push(img_queue_t *q, void *data);

/**
 * Non-blocking pop
 */
void *img_queue_pop(img_queue_t *q);

#endif