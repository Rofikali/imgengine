// ./include/runtime/queue_mpmc.h

// include/runtime/queue_mpmc.h

#ifndef IMGENGINE_RUNTIME_QUEUE_MPMC_H
#define IMGENGINE_RUNTIME_QUEUE_MPMC_H

#include <stdalign.h> // <--- ADD THIS LINE
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

/*
 * 🔥 Cell structure (cache-line aware)
 */
typedef struct cell_t
{
    _Atomic size_t seq;
    void *data;
} cell_t;

/*
 * 🔥 MPMC Queue (Vyukov bounded queue)
 */
typedef struct __attribute__((aligned(64)))
{
    size_t size;
    size_t mask;

    cell_t *cells;

    alignas(64) _Atomic size_t head;
    alignas(64) _Atomic size_t tail;

} img_mpmc_queue_t;

/*
 * Lifecycle
 */
int img_mpmc_init(img_mpmc_queue_t *q, size_t size);
void img_mpmc_destroy(img_mpmc_queue_t *q);

/*
 * Lock-free operations
 */
int img_mpmc_push(img_mpmc_queue_t *q, void *data);
void *img_mpmc_pop(img_mpmc_queue_t *q);

#endif
