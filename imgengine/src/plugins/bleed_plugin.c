// // // src/plugins/bleed_plugin.c

// #include "imgengine/plugins/plugin.h"
// #include "imgengine/layout.h"

// // 🔥 extend edges (real bleed)
// static void apply_bleed(img_t *img, int x, int y, int w, int h, int bleed)
// {
//     int channels = 3;

//     // TOP
//     for (int by = 1; by <= bleed; by++)
//     {
//         for (int i = 0; i < w; i++)
//         {
//             int src = ((y)*img->width + (x + i)) * channels;
//             int dst = ((y - by) * img->width + (x + i)) * channels;

//             for (int c = 0; c < channels; c++)
//                 img->data[dst + c] = img->data[src + c];
//         }
//     }

//     // BOTTOM
//     for (int by = 1; by <= bleed; by++)
//     {
//         for (int i = 0; i < w; i++)
//         {
//             int src = ((y + h - 1) * img->width + (x + i)) * channels;
//             int dst = ((y + h - 1 + by) * img->width + (x + i)) * channels;

//             for (int c = 0; c < channels; c++)
//                 img->data[dst + c] = img->data[src + c];
//         }
//     }

//     // LEFT + RIGHT
//     for (int j = -bleed; j < h + bleed; j++)
//     {
//         for (int bx = 1; bx <= bleed; bx++)
//         {
//             int yy = y + j;

//             if (yy < 0 || yy >= img->height)
//                 continue;

//             // LEFT
//             int srcL = (yy * img->width + x) * channels;
//             int dstL = (yy * img->width + (x - bx)) * channels;

//             // RIGHT
//             int srcR = (yy * img->width + (x + w - 1)) * channels;
//             int dstR = (yy * img->width + (x + w - 1 + bx)) * channels;

//             if (x - bx >= 0)
//                 for (int c = 0; c < channels; c++)
//                     img->data[dstL + c] = img->data[srcL + c];

//             if (x + w - 1 + bx < img->width)
//                 for (int c = 0; c < channels; c++)
//                     img->data[dstR + c] = img->data[srcR + c];
//         }
//     }
// }

// static int bleed_process(img_ctx_t *ctx,
//                          img_t *canvas,
//                          const img_job_t *job)
// {
//     if (job->bleed_px <= 0)
//         return 1;

//     img_layout_info_t *layout = layout_get(ctx);

//     for (int i = 0; i < layout->count; i++)
//     {
//         img_cell_t c = layout->cells[i];

//         apply_bleed(canvas,
//                     c.x, c.y,
//                     c.w, c.h,
//                     job->bleed_px);
//     }

//     return 1;
// }

// static img_plugin_t plugin = {
//     .name = "bleed",
//     .process = bleed_process};

// img_plugin_t *get_bleed_plugin()
// {
//     return &plugin;
// }


// src/plugins/bleed_plugin.c
#include "imgengine/plugins/plugin.h"
#include "imgengine/layout.h"
#include <string.h>

static void apply_bleed_safe(img_t *img, int x, int y, int w, int h, int bleed)
{
    int ch = 3;
    size_t row_bytes = w * ch;

    // TOP: Copy the first row of the photo upwards
    for (int i = 1; i <= bleed; i++)
    {
        int target_y = y - i;
        if (target_y < 0)
            break;
        memcpy(&img->data[(target_y * img->width + x) * ch],
               &img->data[(y * img->width + x) * ch], row_bytes);
    }

    // BOTTOM: Copy the last row of the photo downwards
    for (int i = 1; i <= bleed; i++)
    {
        int target_y = y + h - 1 + i;
        if (target_y >= img->height)
            break;
        memcpy(&img->data[(target_y * img->width + x) * ch],
               &img->data[((y + h - 1) * img->width + x) * ch], row_bytes);
    }

    // LEFT/RIGHT: Needs pixel-by-pixel for the vertical strip
    // (Optimization: Use a small buffer and memcpy for speed)
    // LEFT + RIGHT Vertical Strips
    // We process row-by-row to keep the CPU cache "warm"
    for (int row = -bleed; row < h + bleed; row++)
    {
        int target_y = y + row;

        // 1. Safety Check: Don't write outside the canvas
        if (target_y < 0 || target_y >= img->height)
            continue;

        int row_offset = target_y * img->width * ch;

        // --- LEFT BLEED ---
        // Source is the first pixel of the photo in this row
        int src_x_idx = row_offset + (x * ch);
        for (int b = 1; b <= bleed; b++)
        {
            int dst_x = x - b;
            if (dst_x >= 0)
            {
                int dst_idx = row_offset + (dst_x * ch);
                // Copy 3 bytes (RGB) at once
                *(uint16_t *)(&img->data[dst_idx]) = *(uint16_t *)(&img->data[src_x_idx]); // Copy first 2 bytes
                img->data[dst_idx + 2] = img->data[src_x_idx + 2];                         // Copy 3rd byte
            }
        }

        // --- RIGHT BLEED ---
        // Source is the last pixel of the photo in this row
        int src_r_idx = row_offset + ((x + w - 1) * ch);
        for (int b = 1; b <= bleed; b++)
        {
            int dst_x = x + w - 1 + b;
            if (dst_x < img->width)
            {
                int dst_idx = row_offset + (dst_x * ch);
                // Using a 3-byte struct or casting is faster than a loop
                img->data[dst_idx] = img->data[src_r_idx];
                img->data[dst_idx + 1] = img->data[src_r_idx + 1];
                img->data[dst_idx + 2] = img->data[src_r_idx + 2];
            }
        }
    }
}

static int should_run_bleed(const img_job_t *job)
{
    return job->bleed_px > 0;
}

static int bleed_process(img_ctx_t *ctx, img_t *canvas, const img_job_t *job)
{
    img_layout_info_t *layout = layout_get(ctx);
    if (!layout)
        return 1;

    for (int i = 0; i < layout->count; i++)
    {
        apply_bleed_safe(canvas, layout->cells[i].x, layout->cells[i].y,
                         layout->cells[i].w, layout->cells[i].h, job->bleed_px);
    }
    return 1;
}

static img_plugin_t bleed_plugin = {
    .name = "bleed_engine",
    .should_run = should_run_bleed,
    .process = bleed_process};

img_plugin_t *get_bleed_plugin() { return &bleed_plugin; }
