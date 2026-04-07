// arch/x86_64/scalar/resize_scalar.c

#include "arch/arch_interface.h"
#include "core/buffer.h"
#include "plugins/plugin_resize.h"
#include "pipeline/kernel_adapter.h"

#include <string.h>

/*
 * 🔥 MAIN RESIZE ENTRY (KERNEL ABI)
 */
void resize_scalar(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    void *params)
{
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src)
        return;

    /*
     * 🔥 SIMPLE MODE (dimension update only)
     */
    buf->width = p->target_w;
    buf->height = p->target_h;
}

/*
 * 🔥 HORIZONTAL PASS
 */
void img_arch_resize_h_scalar(
    img_ctx_t *ctx,
    img_buffer_t *dst,
    void *params)
{
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src || !p->x_index)
        return;

    img_buffer_t *src = p->src;

    for (uint32_t y = 0; y < src->height; y++)
    {
        uint8_t *src_row = src->data + y * src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        for (uint32_t x = 0; x < dst->width; x++)
        {
            uint32_t sx = p->x_index[x];

            for (uint32_t c = 0; c < src->channels; c++)
            {
                dst_row[x * src->channels + c] =
                    src_row[sx * src->channels + c];
            }
        }
    }
}

/*
 * 🔥 VERTICAL PASS
 */
void img_arch_resize_v_scalar(
    img_ctx_t *ctx,
    img_buffer_t *dst,
    void *params)
{
    (void)ctx;

    resize_params_t *p = (resize_params_t *)params;

    if (!p || !p->src || !p->y_index)
        return;

    img_buffer_t *src = p->src;

    for (uint32_t y = 0; y < dst->height; y++)
    {
        uint32_t sy = p->y_index[y];

        uint8_t *src_row = src->data + sy * src->stride;
        uint8_t *dst_row = dst->data + y * dst->stride;

        memcpy(dst_row, src_row, dst->width * dst->channels);
    }
}

// #include "arch/arch_interface.h"
// #include "core/buffer.h"
// #include "plugins/plugin_resize.h"
// #include <string.h>
// #include "pipeline/kernel_adapter.h"
// #include "core/buffer.h"

// typedef struct
// {
//     uint32_t new_w;
//     uint32_t new_h;
// } resize_params_t;

// void resize_scalar(
//     img_ctx_t *ctx,
//     img_buffer_t *buf,
//     void *params)
// {
//     (void)ctx;

//     resize_params_t *p = (resize_params_t *)params;

//     if (!p)
//         return;

//     buf->width = p->new_w;
//     buf->height = p->new_h;
// }

// void img_arch_resize_h_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params)
// {
//     (void)ctx;

//     resize_params_t *p = (resize_params_t *)params;

//     for (uint32_t y = 0; y < p->src->height; y++)
//     {
//         uint8_t *src_row = p->src->data + y * p->src->stride;
//         uint8_t *dst_row = dst->data + y * dst->stride;

//         for (uint32_t x = 0; x < dst->width; x++)
//         {
//             uint32_t sx = p->x_index[x];

//             for (uint32_t c = 0; c < p->src->channels; c++)
//             {
//                 dst_row[x * p->src->channels + c] =
//                     src_row[sx * p->src->channels + c];
//             }
//         }
//     }
// }

// void img_arch_resize_v_scalar(img_ctx_t *ctx, img_buffer_t *dst, void *params)
// {
//     (void)ctx;

//     resize_params_t *p = (resize_params_t *)params;

//     for (uint32_t y = 0; y < dst->height; y++)
//     {
//         uint32_t sy = p->y_index[y];

//         uint8_t *src_row = p->src->data + sy * p->src->stride;
//         uint8_t *dst_row = dst->data + y * dst->stride;

//         memcpy(dst_row, src_row, dst->width * dst->channels);
//     }
// }
