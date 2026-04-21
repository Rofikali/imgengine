// ./src/pipeline/canvas_free_buffer.c
#include "pipeline/canvas.h"

void img_canvas_release(img_canvas_t *canvas, img_slab_pool_t *pool)
{
    if (canvas && canvas->buf.data && pool)
    {
        img_slab_recycle(pool, canvas->buf.data);
        canvas->buf.data = NULL;
    }
}
