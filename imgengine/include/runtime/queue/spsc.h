#ifndef IMGENGINE_QUEUE_SPSC_H
#define IMGENGINE_QUEUE_SPSC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct img_spsc_queue img_spsc_queue_t;

img_spsc_queue_t *img_spsc_create(uint32_t power);
void img_spsc_destroy(img_spsc_queue_t *q);

bool img_spsc_push(img_spsc_queue_t *q, void *data);
void *img_spsc_pop(img_spsc_queue_t *q);

#endif