#include "api/v1/img_plugin_api.h"
#include "core/pipeline/opcodes.h"
#include "core/buffer/buffer.h"

static void resize_exec(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    buf->width /= 2;
    buf->height /= 2;
}

static img_plugin_descriptor_t plugin = {
    .name = "resize",
    .abi_version = IMG_PLUGIN_ABI_VERSION,
    .op_code = OP_RESIZE,
    .capabilities = IMG_CAP_SINGLE,
    .single_exec = resize_exec,
    .batch_exec = NULL,
};

IMG_REGISTER_PLUGIN(plugin);