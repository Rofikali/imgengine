// /* memory/numa.c */

#define _GNU_SOURCE
#include <sched.h>
#include <numa.h>
#include <numaif.h>
#include <stdlib.h>
#include "memory/numa.h"

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
    if (numa_available() < 0)
    {
        size = (size + 63) & ~63;
        return aligned_alloc(64, size);
    }

    return numa_alloc_onnode(size, node);
}

void img_numa_free(void *ptr, size_t size)
{
    if (!ptr)
        return;

    if (numa_available() < 0)
    {
        free(ptr);
        return;
    }

    numa_free(ptr, size);
}

// #define _GNU_SOURCE

// #include "memory/numa.h"

// #include <numa.h>
// #include <numaif.h> // 🔥 important for NUMA APIs
// #include <sched.h>  // 🔥 sched_getcpu
// #include <stdlib.h>

// int img_numa_get_node(void)
// {
//     if (numa_available() < 0)
//         return 0;

//     return numa_node_of_cpu(sched_getcpu());
// }

// void *img_numa_alloc_onnode(size_t size, int node)
// {
//     if (numa_available() < 0)
//         return aligned_alloc(64, size);

//     return numa_alloc_onnode(size, node);
// }

// void img_numa_free(void *ptr, size_t size)
// {
//     if (numa_available() < 0)
//     {
//         free(ptr);
//         return;
//     }

//     numa_free(ptr, size);
// }
