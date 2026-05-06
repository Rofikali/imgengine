// ./include/memory/slab.h

/* memory/slab.h */

#ifndef IMGENGINE_MEMORY_SLAB_H
#define IMGENGINE_MEMORY_SLAB_H

#include <stddef.h>
#include <stdint.h>

// 🔥 OPAQUE
typedef struct img_slab_pool img_slab_pool_t;

// ================= LIFECYCLE =================

img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size);
void img_slab_destroy(img_slab_pool_t *pool);

// ================= HOT PATH =================

// 🔥 returns NULL if no block
uint8_t *img_slab_alloc(img_slab_pool_t *pool);

// 🔥 MUST be returned to same pool
void img_slab_free(img_slab_pool_t *pool, void *ptr);
void img_slab_recycle(img_slab_pool_t *pool, void *ptr);

// 🔥 NEW: expose block size (critical safety)
size_t img_slab_block_size(img_slab_pool_t *pool);

#endif
