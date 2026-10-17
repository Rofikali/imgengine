// ./src/plugins/plugin_grayscale.c

#include "arch/arch_interface.h"
#include "core/opcodes.h"
#include "pipeline/plugin_abi.h"

#include <stddef.h>

/*
 * A plugin may override the hardware-selected dispatch entry, but it must
 * preserve OP_GRAYSCALE's RGB-luma contract. Keep the extension point while
 * delegating to the canonical scalar implementation rather than applying a
 * byte-wise transform.
 */
static void grayscale_exec(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    img_arch_grayscale_scalar(ctx, buf, params);
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
