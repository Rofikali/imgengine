// ./include/memory/numa.h

/* memory/numa.h */

#ifndef IMGENGINE_MEMORY_NUMA_H
#define IMGENGINE_MEMORY_NUMA_H

#include <stddef.h>

int img_numa_get_node(void);
void *img_numa_alloc_onnode(size_t size, int node);
void img_numa_free(void *ptr, size_t size);
void img_numa_release(void *ptr, size_t size);

#endif
