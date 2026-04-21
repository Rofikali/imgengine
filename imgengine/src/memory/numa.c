// ./src/memory/numa.c
// src/memory/numa.c

#define _GNU_SOURCE

#include "memory/align.h"
#include "memory/numa.h"
#include "memory/poison.h"

#include <numa.h>
#include <numaif.h>
#include <sched.h>
#include <stdlib.h>

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
    size = img_align64(size);

    if (numa_available() < 0)
        return aligned_alloc(64, size);

    return numa_alloc_onnode(size, node);
}

void img_numa_free(void *ptr, size_t size)
{
    if (!ptr)
        return;

    size = img_align64(size);

    img_poison_block(ptr, size);

    if (numa_available() < 0)
    {
        free(ptr);
        return;
    }

    numa_free(ptr, size);
}

void img_numa_release(void *ptr, size_t size)
{
    img_numa_free(ptr, size);
}
