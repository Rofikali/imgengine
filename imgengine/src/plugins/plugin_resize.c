// ./src/plugins/plugin_resize.c

#include "pipeline/plugin_abi.h"
#include "core/buffer.h"
#include <stddef.h>

static void resize_single(img_ctx_t *ctx, img_buffer_t *buf, void *) {
    (void)ctx;

    /*
     * 🔥 DEFAULT FALLBACK (NO PARAM VERSION)
     */
    buf->width /= 2;
    buf->height /= 2;
}

static img_plugin_descriptor_t plugin = {.name = "resize",
                                         .abi_version = IMG_PLUGIN_ABI_VERSION,
                                         .op_code = 1,
                                         .capabilities = IMG_CAP_SINGLE,
                                         .single_exec = resize_single,
                                         .batch_exec = NULL};

IMG_REGISTER_PLUGIN(plugin);
