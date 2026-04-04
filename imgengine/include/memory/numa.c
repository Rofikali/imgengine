// /* memory/numa.c */

#define _GNU_SOURCE

#include "memory/numa.h"

#include <numa.h>   // numa_available, numa_alloc_onnode, numa_free
#include <numaif.h> // sometimes needed for numa functions
#include <sched.h>  // sched_getcpu
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
    {
        return aligned_alloc(64, size);
    }

    return numa_alloc_onnode(size, node); // ✅ FIXED
}

void img_numa_free(void *ptr, size_t size)
{
    if (!ptr)
        return;

    size = align64(size);

    if (numa_available() < 0)
    {
        free(ptr);
        return;
    }

    numa_free(ptr, size); // ✅ FIXED
}