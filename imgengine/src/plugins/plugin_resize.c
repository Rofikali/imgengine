// plugins/plugin_resize.c

#include "api/v1/img_plugin_api.h"
#include "pipeline/pipeline_types.h"
#include "core/buffer.h"
#include <stdlib.h>

static void resize_single(img_ctx_t *ctx, img_buffer_t *buf, void *params)
{
    (void)ctx;
    (void)params;

    // dummy example
    buf->width /= 2;
    buf->height /= 2;
}

// 🔥 Descriptor
static img_plugin_descriptor_t plugin = {
    .abi_version = IMG_PLUGIN_ABI_VERSION,
    .op_code = 1,
    .name = "resize",
    .capabilities = IMG_CAP_SINGLE | IMG_CAP_ZERO_COPY,
    .single_exec = resize_single,
    .batch_exec = NULL};

// 🔥 Exported symbol
const img_plugin_descriptor_t *img_plugin_get_descriptor(void)
{
    return &plugin;
}

// #include "plugins/plugin_resize.h"
// #include "plugins/plugin_internal.h"
// #include "api/v1/img_types.h"
// #include "api/v1/img_buffer_utils.h"
// #include "memory/slab.h"
// #include "pipeline/jump_table.h"
// #include "core/opcodes.h"

// // 🔥 NO malloc → use slab
// static void precompute_indices(img_ctx_t *ctx, resize_params_t *p)
// {
//     p->x_index = img_slab_alloc(ctx->local_pool);
//     p->y_index = img_slab_alloc(ctx->local_pool);

//     for (uint32_t x = 0; x < p->target_w; x++)
//         p->x_index[x] = (x * p->scale_x) >> 16;

//     for (uint32_t y = 0; y < p->target_h; y++)
//         p->y_index[y] = (y * p->scale_y) >> 16;
// }

// void plugin_resize_single(img_ctx_t *ctx, img_buffer_t *buf, void *params)
// {
//     resize_params_t *p = (resize_params_t *)params;

//     p->scale_x = (buf->width << 16) / p->target_w;
//     p->scale_y = (buf->height << 16) / p->target_h;
//     p->src = buf;

//     precompute_indices(ctx, p);

//     uint8_t *tmp_mem = img_slab_alloc(ctx->local_pool);
//     img_buffer_t tmp = img_buffer_create(tmp_mem, p->target_w, buf->height, buf->channels);

//     uint8_t *out_mem = img_slab_alloc(ctx->local_pool);
//     img_buffer_t dst = img_buffer_create(out_mem, p->target_w, p->target_h, buf->channels);

//     g_jump_table[OP_RESIZE_H](ctx, &tmp, params);

//     p->src = &tmp; // 🔥 CRITICAL FIX

//     g_jump_table[OP_RESIZE_V](ctx, &dst, params);

//     img_slab_free(ctx->local_pool, buf->data);
//     *buf = dst;
// }
