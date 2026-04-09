#ifndef IMGENGINE_NUMA_H
#define IMGENGINE_NUMA_H

#include <stdint.h>

int img_numa_node(void);
void *img_numa_alloc(size_t size, int node);

#endif