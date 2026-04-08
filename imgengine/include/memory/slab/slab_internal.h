#ifndef IMGENGINE_SLAB_INTERNAL_H
#define IMGENGINE_SLAB_INTERNAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

typedef struct slab_block
{
    struct slab_block *next;
} slab_block_t;

typedef struct
{
    uint8_t *base;
    size_t block_size;
    size_t total_blocks;

    _Atomic(slab_block_t *) free_list;

} img_slab_pool_t;

#endif