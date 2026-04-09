#include "api/v1/img_plugin_api.h"
#include "core/pipeline/opcodes.h"

static void grayscale_exec(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    uint32_t n = buf->width * buf->height * buf->channels;
    uint8_t *data = buf->data;

    for (uint32_t i = 0; i < n; i++)
        data[i] = (data[i] * 77) >> 8;
}

static img_plugin_descriptor_t plugin = {
    .name = "grayscale",
    .abi_version = IMG_PLUGIN_ABI_VERSION,
    .op_code = OP_GRAYSCALE,
    .capabilities = IMG_CAP_SINGLE,
    .single_exec = grayscale_exec,
};

IMG_REGISTER_PLUGIN(plugin);