// // src/plugins/crop_plugin.c

// #include "imgengine/plugins/plugin.h"
// #include "imgengine/layout.h"

// static void draw_line_h(img_t *img, int x, int y, int len, int th)
// {
//     for (int t = 0; t < th; t++)
//         for (int i = 0; i < len; i++)
//         {
//             int xx = x + i;
//             int yy = y + t;

//             if (xx < 0 || xx >= img->width || yy < 0 || yy >= img->height)
//                 continue;

//             int idx = (yy * img->width + xx) * 3;
//             img->data[idx + 0] = 0;
//             img->data[idx + 1] = 0;
//             img->data[idx + 2] = 0;
//         }
// }

// static void draw_line_v(img_t *img, int x, int y, int len, int th)
// {
//     for (int t = 0; t < th; t++)
//         for (int i = 0; i < len; i++)
//         {
//             int xx = x + t;
//             int yy = y + i;

//             if (xx < 0 || xx >= img->width || yy < 0 || yy >= img->height)
//                 continue;

//             int idx = (yy * img->width + xx) * 3;
//             img->data[idx + 0] = 0;
//             img->data[idx + 1] = 0;
//             img->data[idx + 2] = 0;
//         }
// }

// static int crop_process(img_ctx_t *ctx,
//                         img_t *canvas,
//                         const img_job_t *job)
// {
//     if (job->crop_mark_px <= 0)
//         return 1;

//     img_layout_info_t *layout = layout_get(ctx);

//     int bleed = job->bleed_px;
//     int offset = job->crop_offset_px;

//     for (int i = 0; i < layout->count; i++)
//     {
//         img_cell_t c = layout->cells[i];

//         int len = job->crop_mark_px;
//         int th = job->crop_thickness;

//         // 🔥 final offset = bleed + gap
//         int d = bleed + offset;

//         // ===== TOP LEFT =====
//         draw_line_h(canvas, c.x - d - len, c.y - d, len, th);
//         draw_line_v(canvas, c.x - d, c.y - d - len, len, th);

//         // ===== TOP RIGHT =====
//         draw_line_h(canvas, c.x + c.w + d, c.y - d, len, th);
//         draw_line_v(canvas, c.x + c.w + d, c.y - d - len, len, th);

//         // ===== BOTTOM LEFT =====
//         draw_line_h(canvas, c.x - d - len, c.y + c.h + d, len, th);
//         draw_line_v(canvas, c.x - d, c.y + c.h + d, len, th);

//         // ===== BOTTOM RIGHT =====
//         draw_line_h(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
//         draw_line_v(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
//     }

//     return 1;
// }

// static img_plugin_t plugin = {
//     .name = "crop_marks_iso",
//     .process = crop_process};

// img_plugin_t *get_crop_plugin()
// {
//     return &plugin;
// }

// ******************************************
// #include "imgengine/plugins/plugin.h"
// ********************************************

// src/plugins/crop_plugin.c

#include "imgengine/layout.h"
#include "imgengine/plugins/plugin.h"

/* Senior Move: Paranoia check. Prevent any out-of-bounds writes */
#define SET_PIXEL_SAFE(img, x, y)                                                                  \
    if (x >= 0 && x < (img)->width && y >= 0 && y < (img)->height) {                               \
        int idx = ((y) * (img)->width + (x)) * 3;                                                  \
        (img)->data[idx] = (img)->data[idx + 1] = (img)->data[idx + 2] = 0;                        \
    }

static void draw_line_h_safe(img_t *img, int x, int y, int len, int th) {
    for (int t = 0; t < th; t++)
        for (int i = 0; i < len; i++)
            SET_PIXEL_SAFE(img, x + i, y + t);
}

static void draw_line_v_safe(img_t *img, int x, int y, int len, int th) {
    for (int t = 0; t < th; t++)
        for (int i = 0; i < len; i++)
            SET_PIXEL_SAFE(img, x + t, y + i);
}

static int should_run_crop(const img_job_t *job) {
    return job->crop_mark_px > 0;
}

static int crop_process(img_ctx_t *ctx, img_t *canvas, const img_job_t *job) {
    img_layout_info_t *layout = layout_get(ctx);
    if (!layout)
        return 1;

    /* 6-Year Engineer Tip: We align marks to the PHOTO edge, not the bleed edge.
       This ensures the lab knows exactly where to cut. */
    int d = job->crop_offset_px;
    int len = job->crop_mark_px;
    int th = job->crop_thickness;

    for (int i = 0; i < layout->count; i++) {
        img_cell_t c = layout->cells[i];

        // TOP LEFT (Horizontal draws left, Vertical draws up)
        draw_line_h_safe(canvas, c.x - d - len, c.y - d, len, th);
        draw_line_v_safe(canvas, c.x - d, c.y - d - len, len, th);

        // TOP RIGHT (Horizontal draws right, Vertical draws up)
        draw_line_h_safe(canvas, c.x + c.w + d, c.y - d, len, th);
        draw_line_v_safe(canvas, c.x + c.w + d, c.y - d - len, len, th);

        // BOTTOM LEFT (Horizontal draws left, Vertical draws down)
        draw_line_h_safe(canvas, c.x - d - len, c.y + c.h + d, len, th);
        draw_line_v_safe(canvas, c.x - d, c.y + c.h + d, len, th);

        // BOTTOM RIGHT (Horizontal draws right, Vertical draws down)
        draw_line_h_safe(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
        draw_line_v_safe(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
    }

    // for (int i = 0; i < layout->count; i++) {
    //     img_cell_t c = layout->cells[i];

    //     // TOP LEFT
    //     draw_line_h_safe(canvas, c.x - d - len, c.y - d, len, th);
    //     draw_line_v_safe(canvas, c.x - d, c.y - d - len, len, th);

    //     // TOP RIGHT
    //     draw_line_h_safe(canvas, c.x + c.w + d, c.y - d, len, th);
    //     draw_line_v_safe(canvas, c.x + c.w + d, c.y - d - len, len, th);

    //     // BOTTOM LEFT
    //     draw_line_h_safe(canvas, c.x - d - len, c.y + c.h + d, len, th);
    //     draw_line_v_safe(canvas, c.x - d, c.y + c.h + d, len, th);

    //     // BOTTOM RIGHT
    //     draw_line_h_safe(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
    //     draw_line_v_safe(canvas, c.x + c.w + d, c.y + c.h + d, len, th);
    // }

    return 1;
}

static img_plugin_t crop_plugin = {
    .name = "crop_marks_iso", .should_run = should_run_crop, .process = crop_process};

img_plugin_t *get_crop_plugin() {
    return &crop_plugin;
}
