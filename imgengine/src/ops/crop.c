// // src/ops/crop.c

// #include "imgengine/image.h"

// int img_center_crop(const img_t *src, img_t *dst,
//                     mem_pool_t *mp,
//                     int target_w, int target_h)
// {

//     float src_ratio = (float)src->width / src->height;
//     float dst_ratio = (float)target_w / target_h;

//     int crop_w, crop_h;
//     int x_off = 0, y_off = 0;

//     if (src_ratio > dst_ratio)
//     {
//         crop_h = src->height;
//         crop_w = (int)(crop_h * dst_ratio);
//         x_off = (src->width - crop_w) / 2;
//     }
//     else
//     {
//         crop_w = src->width;
//         crop_h = (int)(crop_w / dst_ratio);
//         y_off = (src->height - crop_h) / 2;
//     }

//     if (!img_create(dst, mp, crop_w, crop_h, 3))
//         return 0;

//     for (int y = 0; y < crop_h; y++)
//     {
//         for (int x = 0; x < crop_w; x++)
//         {

//             int src_i = ((y + y_off) * src->width + (x + x_off)) * 3;
//             int dst_i = (y * crop_w + x) * 3;

//             dst->data[dst_i + 0] = src->data[src_i + 0];
//             dst->data[dst_i + 1] = src->data[src_i + 1];
//             dst->data[dst_i + 2] = src->data[src_i + 2];
//         }
//     }

//     return 1;
// }

#include "imgengine/image.h"
#include <string.h>

int img_center_crop(const img_t *src, img_t *dst, mem_pool_t *mp, int target_w, int target_h)
{
    float src_ratio = (float)src->width / src->height;
    float dst_ratio = (float)target_w / target_h;

    int crop_w = (src_ratio > dst_ratio) ? (int)(src->height * dst_ratio) : src->width;
    int crop_h = (src_ratio > dst_ratio) ? src->height : (int)(src->width / dst_ratio);

    int x_off = (src->width - crop_w) / 2;
    int y_off = (src->height - crop_h) / 2;

    if (!img_create(dst, mp, crop_w, crop_h, 3))
        return 0;

    // Kernel Grade: Use row-by-row memcpy. It's significantly faster than manual loops.
    size_t row_bytes = crop_w * 3;
    for (int y = 0; y < crop_h; y++)
    {
        unsigned char *src_row = src->data + ((y + y_off) * src->width + x_off) * 3;
        unsigned char *dst_row = dst->data + (y * crop_w) * 3;
        memcpy(dst_row, src_row, row_bytes);
    }
    return 1;
}
