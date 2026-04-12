// src/pipeline/bleed.c

#define _GNU_SOURCE

#include "core/buffer.h"
#include "pipeline/layout.h"
#include "api/v1/img_error.h"
#include <string.h>

img_result_t img_apply_bleed(
    img_buffer_t *canvas,
    const img_layout_t *layout,
    uint32_t bleed_px,
    uint32_t gap)
{
    if (!canvas || !layout || bleed_px == 0)
        return IMG_SUCCESS;

    const uint32_t ch = 3;

    for (uint32_t i = 0; i < layout->count; i++)
    {
        const img_cell_t *c = &layout->cells[i];

        int32_t cx = c->x;
        int32_t cy = c->y;
        int32_t cw = c->w;
        int32_t chh = c->h;

        int32_t safe_left = cx - (int32_t)(gap / 2);
        int32_t safe_right = cx + cw + (int32_t)(gap / 2);
        int32_t safe_top = cy - (int32_t)(gap / 2);
        int32_t safe_bottom = cy + chh + (int32_t)(gap / 2);

        /* TOP */
        for (int32_t b = 1; b <= (int32_t)bleed_px; b++)
        {
            int32_t ty = cy - b;
            if (ty < safe_top)
                break;

            for (int32_t x = cx; x < cx + cw; x++)
            {
                uint8_t *dst = canvas->data + ty * canvas->stride + x * ch;
                uint8_t *src = canvas->data + cy * canvas->stride + x * ch;

                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
        }

        /* BOTTOM */
        for (int32_t b = 1; b <= (int32_t)bleed_px; b++)
        {
            int32_t ty = cy + chh - 1 + b;
            if (ty > safe_bottom)
                break;

            for (int32_t x = cx; x < cx + cw; x++)
            {
                uint8_t *dst = canvas->data + ty * canvas->stride + x * ch;
                uint8_t *src = canvas->data + (cy + chh - 1) * canvas->stride + x * ch;

                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
        }
    }

    return IMG_SUCCESS;
}

// img_result_t img_apply_bleed(
//     img_buffer_t *canvas,
//     const img_layout_t *layout,
//     uint32_t bleed_px)
// {
//     if (!canvas || !layout || bleed_px == 0)
//         return IMG_SUCCESS;

//     const uint32_t ch = 3;
//     const int32_t cw = (int32_t)canvas->width;
//     const int32_t cht = (int32_t)canvas->height;
//     const uint32_t str = canvas->stride;

//     /* canvas buffer end — never write past this */
//     const uint8_t *buf_end = canvas->data + (size_t)cht * str;

//     for (uint32_t i = 0; i < layout->count; i++)
//     {
//         const img_cell_t *c = &layout->cells[i];

//         const int32_t cx = (int32_t)c->x;
//         const int32_t cy = (int32_t)c->y;
//         const int32_t cw2 = (int32_t)c->w;
//         const int32_t ch2 = (int32_t)c->h;
//         const int32_t bp = (int32_t)bleed_px;

//         /* ── TOP bleed ── */
//         for (int32_t b = 1; b <= bp; b++)
//         {
//             int32_t ty = cy - b;
//             if (ty < 0)
//                 break;

//             const uint8_t *src = canvas->data + (size_t)cy * str + (size_t)cx * ch;
//             uint8_t *dst = canvas->data + (size_t)ty * str + (size_t)cx * ch;

//             /* clamp copy width */
//             int32_t cpw = cw2;
//             if (cx + cpw > cw)
//                 cpw = cw - cx;
//             if (cpw <= 0)
//                 continue;

//             /* safety: dst must be within buffer */
//             if (dst < canvas->data || dst + cpw * ch > buf_end)
//                 continue;

//             memcpy(dst, src, (size_t)cpw * ch);
//         }

//         /* ── BOTTOM bleed ── */
//         for (int32_t b = 1; b <= bp; b++)
//         {
//             int32_t ty = cy + ch2 - 1 + b;
//             if (ty >= cht)
//                 break;

//             const uint8_t *src = canvas->data + (size_t)(cy + ch2 - 1) * str + (size_t)cx * ch;
//             uint8_t *dst = canvas->data + (size_t)ty * str + (size_t)cx * ch;

//             int32_t cpw = cw2;
//             if (cx + cpw > cw)
//                 cpw = cw - cx;
//             if (cpw <= 0)
//                 continue;

//             if (dst < canvas->data || dst + cpw * ch > buf_end)
//                 continue;

//             memcpy(dst, src, (size_t)cpw * ch);
//         }

//         /* ── LEFT + RIGHT bleed, row by row ── */
//         int32_t ry0 = cy - bp;
//         int32_t ry1 = cy + ch2 + bp;
//         if (ry0 < 0)
//             ry0 = 0;
//         if (ry1 > cht)
//             ry1 = cht;

//         for (int32_t py = ry0; py < ry1; py++)
//         {
//             uint8_t *row = canvas->data + (size_t)py * str;

//             /* LEFT */
//             if (cx >= 0 && cx < cw)
//             {
//                 const uint8_t *sl = row + (size_t)cx * ch;
//                 for (int32_t b = 1; b <= bp; b++)
//                 {
//                     int32_t tx = cx - b;
//                     if (tx < 0)
//                         break;
//                     uint8_t *d = row + (size_t)tx * ch;
//                     if (d < canvas->data || d + ch > buf_end)
//                         break;
//                     d[0] = sl[0];
//                     d[1] = sl[1];
//                     d[2] = sl[2];
//                 }
//             }

//             /* RIGHT */
//             int32_t rx = cx + cw2 - 1;
//             if (rx >= 0 && rx < cw)
//             {
//                 const uint8_t *sr = row + (size_t)rx * ch;
//                 for (int32_t b = 1; b <= bp; b++)
//                 {
//                     int32_t tx = rx + b;
//                     if (tx >= cw)
//                         break;
//                     uint8_t *d = row + (size_t)tx * ch;
//                     if (d < canvas->data || d + ch > buf_end)
//                         break;
//                     d[0] = sr[0];
//                     d[1] = sr[1];
//                     d[2] = sr[2];
//                 }
//             }
//         }
//     }

//     return IMG_SUCCESS;
// }
