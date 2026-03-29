// src/plugins/bleed_plugin.c

#include "imgengine/layout.h"
#include "imgengine/plugins/plugin.h"
#include <string.h>

static void apply_bleed_safe(img_t *img, int x, int y, int w, int h, int bleed) {
    int ch = 3;
    size_t row_bytes = w * ch;

    // TOP: Copy the first row of the photo upwards
    for (int i = 1; i <= bleed; i++) {
        int target_y = y - i;
        if (target_y < 0)
            break;
        memcpy(&img->data[(target_y * img->width + x) * ch], &img->data[(y * img->width + x) * ch],
               row_bytes);
    }

    // BOTTOM: Copy the last row of the photo downwards
    for (int i = 1; i <= bleed; i++) {
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
    for (int row = -bleed; row < h + bleed; row++) {
        int target_y = y + row;

        // 1. Safety Check: Don't write outside the canvas
        if (target_y < 0 || target_y >= img->height)
            continue;

        int row_offset = target_y * img->width * ch;

        // --- LEFT BLEED ---
        // Source is the first pixel of the photo in this row
        int src_x_idx = row_offset + (x * ch);
        for (int b = 1; b <= bleed; b++) {
            int dst_x = x - b;
            if (dst_x >= 0) {
                int dst_idx = row_offset + (dst_x * ch);
                // Copy 3 bytes (RGB) at once
                *(uint16_t *)(&img->data[dst_idx]) =
                    *(uint16_t *)(&img->data[src_x_idx]);          // Copy first 2 bytes
                img->data[dst_idx + 2] = img->data[src_x_idx + 2]; // Copy 3rd byte
            }
        }

        // --- RIGHT BLEED ---
        // Source is the last pixel of the photo in this row
        int src_r_idx = row_offset + ((x + w - 1) * ch);
        for (int b = 1; b <= bleed; b++) {
            int dst_x = x + w - 1 + b;
            if (dst_x < img->width) {
                int dst_idx = row_offset + (dst_x * ch);
                // Using a 3-byte struct or casting is faster than a loop
                img->data[dst_idx] = img->data[src_r_idx];
                img->data[dst_idx + 1] = img->data[src_r_idx + 1];
                img->data[dst_idx + 2] = img->data[src_r_idx + 2];
            }
        }
    }
}

static int should_run_bleed(const img_job_t *job) {
    return job->bleed_px > 0;
}

static int bleed_process(img_ctx_t *ctx, img_t *canvas, const img_job_t *job) {
    img_layout_info_t *layout = layout_get(ctx);
    if (!layout)
        return 1;

    for (int i = 0; i < layout->count; i++) {
        apply_bleed_safe(canvas, layout->cells[i].x, layout->cells[i].y, layout->cells[i].w,
                         layout->cells[i].h, job->bleed_px);
    }
    return 1;
}

static img_plugin_t bleed_plugin = {
    .name = "bleed_engine", .should_run = should_run_bleed, .process = bleed_process};

img_plugin_t *get_bleed_plugin() {
    return &bleed_plugin;
}
