
// runtime/cluster_registry.h

#ifndef IMGENGINE_CLUSTER_REGISTRY_H
#define IMGENGINE_CLUSTER_REGISTRY_H

#include <stdint.h>

typedef struct
{
    uint32_t node_id;
    const char *ip;
    uint16_t port;
    uint32_t load;
} img_node_t;

void img_cluster_register(img_node_t *node);
img_node_t *img_cluster_pick(void);

#endif