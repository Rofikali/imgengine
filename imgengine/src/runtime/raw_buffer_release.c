#include "runtime/job_exec.h"

#include "api/api_internal.h"
#include "memory/slab.h"

void img_runtime_release_raw_buffer(
    img_engine_t *engine,
    img_buffer_t *buf)
{
    if (engine && buf && buf->data)
        img_slab_free(engine->global_pool, buf->data);
}
