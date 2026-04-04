// plugins/plugin_registry.c

#include "plugins/plugin_internal.h"
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

// externs
extern void plugin_resize_single(img_ctx_t *, img_buffer_t *, void *);
extern void plugin_crop_single(img_ctx_t *, img_buffer_t *, void *);
extern void plugin_grayscale_single(img_ctx_t *, img_buffer_t *, void *);

void img_plugins_init_all(void)
{
    img_register_op(OP_RESIZE, plugin_resize_single, NULL);
    img_register_op(OP_CROP, plugin_crop_single, NULL);
    img_register_op(OP_GRAYSCALE, plugin_grayscale_single, NULL);
}
