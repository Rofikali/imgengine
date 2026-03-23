// src/layout/grid_layout.c

#include "imgengine/image.h"
#include "imgengine/resize.h"
#include "imgengine/context.h"

extern void img_blit_avx2(img_t *, const img_t *, int, int);

int layout_grid(img_t *canvas,
                const img_t *photo,
                int cols, int rows,
                int gap, int padding,
                img_ctx_t *ctx)
{
    int pw = photo->width;
    int ph = photo->height;

    int usable_w = canvas->width - 2 * padding;
    int usable_h = canvas->height - 2 * padding;

    // =========================
    // SCALE (FIT TO PAGE - NEVER BREAK)
    // =========================
    float scale_x = (float)usable_w /
                    (cols * pw + (cols - 1) * gap);

    float scale_y = (float)usable_h /
                    (rows * ph + (rows - 1) * gap);

    float scale = scale_x < scale_y ? scale_x : scale_y;

    if (scale > 1.0f)
        scale = 1.0f; // no upscale

    int final_pw = (int)(pw * scale);
    int final_ph = (int)(ph * scale);

    // =========================
    // PRE-RESIZE ONCE (ULTRA IMPORTANT)
    // =========================
    img_t scaled;
    if (!img_resize(photo, &scaled, &ctx->pool, final_pw, final_ph))
        return 0;

    // =========================
    // GRID TOTAL SIZE
    // =========================
    int total_w = cols * final_pw + (cols - 1) * gap;
    // int total_h = rows * final_ph + (rows - 1) * gap;

    // =========================
    // ⭐ PRO HYBRID POSITIONING
    // =========================
    int start_x = (canvas->width - total_w) / 2; // center horizontally
    int start_y = padding;                       // top aligned

    // Safety clamp (never go negative)
    if (start_x < padding)
        start_x = padding;

    // =========================
    // GRID PLACEMENT (SIMD READY)
    // =========================
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int x = start_x + c * (final_pw + gap);
            int y = start_y + r * (final_ph + gap);

            img_blit_avx2(canvas, &scaled, x, y);
        }
    }

    return 1;
}

// #include "imgengine/image.h"
// #include "imgengine/context.h"
// #include "imgengine/resize.h"

// #include <omp.h>

// // 🔥 use AVX2 blitter
// extern void img_blit_avx2(img_t *, const img_t *, int, int);

// int layout_grid(img_t *canvas,
//                 const img_t *photo,
//                 int cols, int rows,
//                 int gap, int padding,
//                 img_ctx_t *ctx)
// {
//     int pw = photo->width;
//     int ph = photo->height;

//     int usable_w = canvas->width - 2 * padding;
//     int usable_h = canvas->height - 2 * padding;

//     // =========================
//     // FIT WITHOUT BREAKING
//     // =========================
//     float scale_x = (float)usable_w /
//                     (cols * pw + (cols - 1) * gap);

//     float scale_y = (float)usable_h /
//                     (rows * ph + (rows - 1) * gap);

//     float scale = scale_x < scale_y ? scale_x : scale_y;

//     if (scale > 1.0f)
//         scale = 1.0f;

//     int final_pw = pw * scale;
//     int final_ph = ph * scale;

//     // =========================
//     // PRE-RESIZE ONCE
//     // =========================
//     img_t scaled;
//     if (!img_resize(photo, &scaled, &ctx->pool, final_pw, final_ph))
//         return 0;

//     // =========================
//     // CENTER GRID
//     // =========================
//     int total_w = cols * final_pw + (cols - 1) * gap;
//     int total_h = rows * final_ph + (rows - 1) * gap;

//     int start_x = (canvas->width - total_w) / 2;
//     int start_y = (canvas->height - total_h) / 2;

//     // =========================
//     // MULTI-THREAD
//     // =========================
// #pragma omp parallel for schedule(static)
//     for (int r = 0; r < rows; r++)
//     {
//         for (int c = 0; c < cols; c++)
//         {
//             int x = start_x + c * (final_pw + gap);
//             int y = start_y + r * (final_ph + gap);

//             img_blit_avx2(canvas, &scaled, x, y);
//         }
//     }

//     return 1;
// }