// ./src/memory/numa.c
// src/memory/numa.c

#define _GNU_SOURCE

#include "memory/numa.h"
#include "memory/poison.h"

#include <numa.h>
#include <numaif.h>
#include <sched.h>
#include <stdlib.h>

static inline size_t align64(size_t x)
{
    return (x + 63) & ~63;
}

int img_numa_get_node(void)
{
    if (numa_available() < 0)
        return 0;

    int cpu = sched_getcpu();
    if (cpu < 0)
        return 0;

    return numa_node_of_cpu(cpu);
}

void *img_numa_alloc_onnode(size_t size, int node)
{
    size = align64(size);

    if (numa_available() < 0)
        return aligned_alloc(64, size);

    return numa_alloc_onnode(size, node);
}

void img_numa_free(void *ptr, size_t size)
{
    if (!ptr)
        return;

    size = align64(size);

    img_poison_block(ptr, size);

    if (numa_available() < 0)
    {
        free(ptr);
        return;
    }

    numa_free(ptr, size);
}
