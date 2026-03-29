// imgengine/memory_pool.h

#ifndef IMG_MEMORY_POOL_H
#define IMG_MEMORY_POOL_H

#include <stddef.h>

typedef struct {
    unsigned char *buffer;
    size_t size;
    size_t offset;
} mem_pool_t;

void mp_init(mem_pool_t *mp, size_t size);
void *mp_alloc(mem_pool_t *mp, size_t size);
void mp_reset(mem_pool_t *mp);
void mp_destroy(mem_pool_t *mp);

#endif