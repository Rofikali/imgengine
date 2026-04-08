#ifndef IMGENGINE_QUEUE_MPMC_H
#define IMGENGINE_QUEUE_MPMC_H

#include <stddef.h>

typedef struct img_mpmc_queue img_mpmc_queue_t;

int img_mpmc_init(img_mpmc_queue_t *q, size_t size);
void img_mpmc_destroy(img_mpmc_queue_t *q);

int img_mpmc_push(img_mpmc_queue_t *q, void *data);
void *img_mpmc_pop(img_mpmc_queue_t *q);

#endif