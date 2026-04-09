#include "pipeline/dispatch/jump_table.h"
#include "core/buffer/buffer.h"

static void resize_scalar(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    buf->width /= 2;
    buf->height /= 2;
}

void img_register_baseline_kernels(void)
{
    img_register_op(1, resize_scalar);
}