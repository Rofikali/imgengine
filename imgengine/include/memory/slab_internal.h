
/* memory/slab_internal.h */

#ifndef IMGENGINE_SLAB_INTERNAL_H
#define IMGENGINE_SLAB_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

typedef struct slab_block
{
    struct slab_block *next;
} slab_block_t;

struct img_slab_pool
{
    slab_block_t *free_list;

    uint8_t *memory;
    size_t total_size;

    size_t block_size;
    size_t block_count;

    int numa_node;
};

#endif