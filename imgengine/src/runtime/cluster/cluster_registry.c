#include "runtime/cluster/cluster_registry.h"

#define MAX_NODES 32

static img_node_t nodes[MAX_NODES];
static uint32_t count;

void img_cluster_register(img_node_t *n)
{
    if (count < MAX_NODES)
        nodes[count++] = *n;
}

img_node_t *img_cluster_pick(void)
{
    if (!count) return NULL;
    return &nodes[0];
}