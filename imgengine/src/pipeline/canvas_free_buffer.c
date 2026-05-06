// ./src/pipeline/canvas_free_buffer.c
#include "pipeline/canvas.h"

void img_canvas_release(img_canvas_t *canvas, img_slab_pool_t *pool) {
    if (!canvas || !canvas->buf.data || canvas->cache_owned)
        return;

    if (!pool)
        pool = canvas->buf.owner_pool;

    if (pool) {
        img_slab_recycle(pool, canvas->buf.data);
        canvas->buf.data = NULL;
        canvas->buf.owner_pool = NULL;
        canvas->buf.width = 0;
        canvas->buf.height = 0;
        canvas->buf.channels = 0;
        canvas->buf.stride = 0;
        canvas->bg_signature = 0;
        canvas->initialized = 0;
        canvas->cache_owned = 0;
    }
}
