// ./src/pipeline/canvas_alloc_buffer.c
#include "pipeline/canvas_internal.h"

img_result_t img_canvas_alloc_buffer(
    img_canvas_t *canvas,
    img_slab_pool_t *pool,
    uint32_t pw,
    uint32_t ph)
{
    if (canvas->buf.data &&
        canvas->buf.width == pw &&
        canvas->buf.height == ph &&
        canvas->buf.channels == 3 &&
        canvas->buf.stride == pw * 3)
    {
        return IMG_SUCCESS;
    }

    if (canvas->buf.data)
        img_canvas_release(canvas, pool);

    size_t required = (size_t)pw * ph * 3;
    size_t block = img_slab_block_size(pool);
    if (required > block)
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    canvas->buf.data = mem;
    canvas->buf.width = pw;
    canvas->buf.height = ph;
    canvas->buf.channels = 3;
    canvas->buf.stride = pw * 3;
    canvas->bg_signature = 0;
    canvas->initialized = 0;
    canvas->cache_owned = 0;
    return IMG_SUCCESS;
}
