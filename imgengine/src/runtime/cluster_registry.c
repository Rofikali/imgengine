#include "runtime/cluster_registry.h"

#define MAX_NODES 32

static img_node_t nodes[MAX_NODES];
static uint32_t count = 0;

void img_cluster_register(img_node_t *node)
{
    if (count < MAX_NODES)
        nodes[count++] = *node;
}

img_node_t *img_cluster_pick(void)
{
    if (count == 0)
        return NULL;

    // 🔥 naive round-robin (replace with load-aware)
    return &nodes[0];
}