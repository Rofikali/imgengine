#ifndef IMGENGINE_CTX_H
#define IMGENGINE_CTX_H

#include <stdint.h>

typedef struct img_ctx
{
    uint32_t thread_id;
    uint32_t numa_node;

    uint64_t caps;

    void *scratch;

} img_ctx_t;

#endif