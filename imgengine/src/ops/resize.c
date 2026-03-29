// // src/ops/resize.c

// #include "imgengine/image.h"

// int img_resize(const img_t *src, img_t *dst,
//                mem_pool_t *mp,
//                int new_w, int new_h)
// {

//     if (!img_create(dst, mp, new_w, new_h, 3))
//         return 0;

//     int sw = src->width, sh = src->height;

//     for (int y = 0; y < new_h; y++)
//     {
//         for (int x = 0; x < new_w; x++)
//         {

//             float gx = (float)x / new_w * sw;
//             float gy = (float)y / new_h * sh;

//             int gxi = (int)gx;
//             int gyi = (int)gy;

//             for (int c = 0; c < 3; c++)
//             {
//                 dst->data[(y * new_w + x) * 3 + c] =
//                     src->data[(gyi * sw + gxi) * 3 + c];
//             }
//         }
//     }

//     return 1;
// }

#include "imgengine/image.h"
#include <immintrin.h>
#include <stdint.h>

int img_resize(const img_t *src, img_t *dst, mem_pool_t *mp, int new_w, int new_h)
{
    if (!img_create(dst, mp, new_w, new_h, 3))
        return 0;

    // Fixed-point scale factors (16.16 format)
    uint32_t x_ratio = (uint32_t)((src->width << 16) / new_w);
    uint32_t y_ratio = (uint32_t)((src->height << 16) / new_h);

    for (int y = 0; y < new_h; y++)
    {
        uint32_t src_y = (y * y_ratio) >> 16;
        unsigned char *src_row = src->data + (src_y * src->width * 3);
        unsigned char *dst_row = dst->data + (y * new_w * 3);

        for (int x = 0; x < new_w; x++)
        {
            uint32_t src_x = (x * x_ratio) >> 16;
            int src_idx = src_x * 3;
            int dst_idx = x * 3;

            // Manual Unrolling: Better for the compiler to vectorize
            dst_row[dst_idx + 0] = src_row[src_idx + 0];
            dst_row[dst_idx + 1] = src_row[src_idx + 1];
            dst_row[dst_idx + 2] = src_row[src_idx + 2];
        }
    }
    return 1;
}
