#include "api/v1/img_plugin_api.h"
#include "core/pipeline/opcodes.h"
#include "core/buffer/buffer.h"

typedef struct
{
    uint32_t x, y, w, h;
} crop_params_t;

static void crop_exec(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;

    if (!buf || !params) return;

    crop_params_t *p = params;

    if (p->x + p->w > buf->width ||
        p->y + p->h > buf->height)
        return;

    buf->data += (p->y * buf->stride) + (p->x * buf->channels);
    buf->width = p->w;
    buf->height = p->h;
}

static img_plugin_descriptor_t plugin = {
    .name = "crop",
    .abi_version = IMG_PLUGIN_ABI_VERSION,
    .op_code = OP_CROP,
    .capabilities = IMG_CAP_SINGLE,
    .single_exec = crop_exec,
};

IMG_REGISTER_PLUGIN(plugin);