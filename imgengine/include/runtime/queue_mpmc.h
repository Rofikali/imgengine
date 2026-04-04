#ifndef IMGENGINE_RUNTIME_QUEUE_MPMC_H
#define IMGENGINE_RUNTIME_QUEUE_MPMC_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

// Forward declaration
typedef struct cell_t cell_t;

// 🔥 CACHE-LINE ALIGNED QUEUE
typedef struct __attribute__((aligned(64)))
{
    size_t size;
    size_t mask;

    cell_t *cells;

    _Atomic size_t head;
    char pad1[64]; // prevent false sharing

    _Atomic size_t tail;
    char pad2[64]; // prevent false sharing

} img_mpmc_queue_t;

int img_mpmc_init(img_mpmc_queue_t *q, size_t size);
void img_mpmc_destroy(img_mpmc_queue_t *q);

int img_mpmc_push(img_mpmc_queue_t *q, void *data);
void *img_mpmc_pop(img_mpmc_queue_t *q);

#endif