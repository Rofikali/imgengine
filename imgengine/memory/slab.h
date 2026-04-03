/* memory/slab.h */
#ifndef IMGENGINE_MEMORY_SLAB_H
#define IMGENGINE_MEMORY_SLAB_H

#include <stddef.h>
#include <stdint.h>

// 🔥 Forward declaration (REAL FIX)
typedef struct img_slab_pool img_slab_pool_t;

// ===== API =====
img_slab_pool_t *img_slab_create(size_t total_size, size_t block_size);

uint8_t *img_slab_alloc(img_slab_pool_t *pool);

void img_slab_free(img_slab_pool_t *pool, void *ptr);

void img_slab_destroy(img_slab_pool_t *pool);

#endif
