#ifndef IMGENGINE_CTX_INTERNAL_H
#define IMGENGINE_CTX_INTERNAL_H

#include "core/context/ctx.h"
#include "memory/slab/slab.h"

/*
 * 🔥 INTERNAL ENGINE (STRICTLY PRIVATE)
 */
typedef struct img_engine
{
    uint32_t worker_count;

    void *workers;

    img_slab_pool_t *global_pool;

    uint64_t caps;

} img_engine_t;

#endif