#define _GNU_SOURCE
#include "memory/numa/numa.h"

#include <stdlib.h>
#ifdef __linux__
#include <numa.h>
#endif

int img_numa_node(void)
{
#ifdef __linux__
    return numa_node_of_cpu(sched_getcpu());
#else
    return 0;
#endif
}

void *img_numa_alloc(size_t size, int node)
{
#ifdef __linux__
    return numa_alloc_onnode(size, node);
#else
    (void)node;
    return aligned_alloc(64, size);
#endif
}