#include "core/engine/engine.h"
#include "core/context/ctx_internal.h"

#include <stdlib.h>
#include <string.h>

img_engine_t *img_engine_create(uint32_t workers)
{
    if (workers == 0)
        return NULL;

    img_engine_t *e = calloc(1, sizeof(*e));
    if (!e)
        return NULL;

    e->worker_count = workers;

    return e;
}

void img_engine_destroy(img_engine_t *e)
{
    if (!e)
        return;

    free(e);
}