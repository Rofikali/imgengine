// plugins/plugin_grayscale.c

#include "api/v1/img_plugin_api.h"
#include "core/opcodes.h"
#include "arch/arch_interface.h"

#include <stddef.h>

static void grayscale_exec(img_ctx_t *ctx, img_buffer_t *buf)
{
    (void)ctx;

    uint32_t n = buf->width * buf->height * buf->channels;
    uint8_t *data = buf->data;

    for (uint32_t i = 0; i < n; i++)
        data[i] = (data[i] * 77) >> 8;
}

static img_plugin_descriptor_t grayscale_plugin = {
    .name = "grayscale",
    .abi_version = IMG_PLUGIN_ABI_VERSION,
    .op_code = OP_GRAYSCALE,
    .capabilities = IMG_CAP_SINGLE,
    .single_exec = grayscale_exec,
    .batch_exec = NULL,
};

/*
 * 🔥 REGISTER (NO FUNCTION CALL, NO COST)
 */
IMG_REGISTER_PLUGIN(grayscale_plugin);
