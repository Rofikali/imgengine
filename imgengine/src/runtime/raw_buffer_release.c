// ./src/runtime/raw_buffer_release.c
#include "runtime/job_exec.h"
#include "core/context_internal.h"
#include "memory/slab.h"

void img_runtime_release_raw_buffer(img_engine_t *engine, img_buffer_t *buf) {
    if (!buf || !buf->data)
        return;

    img_slab_pool_t *pool = buf->owner_pool;
    if (!pool && engine)
        pool = engine->global_pool;

    if (!pool)
        return;

    img_slab_free(pool, buf->data);
    buf->data = NULL;
    buf->owner_pool = NULL;
}
