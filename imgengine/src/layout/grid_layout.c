// // src/layout/grid_layout.c

// #include "imgengine/layout.h"
// #include "imgengine/context.h"
// #include "imgengine/api.h" // 🔥 REQUIRED (fixes img_job)
// #include "imgengine/resize.h"
// #include "imgengine/memory_pool.h" // 🔥 REQUIRED

// extern void img_blit_avx2(img_t *, const img_t *, int, int);

// int layout_grid(img_t *canvas,
//                 const img_t *photo,
//                 const img_job_t *job,
//                 struct img_ctx *ctx)
// {
//     int cols = job->cols;
//     int rows = job->rows;
//     int gap = job->gap;
//     int padding = job->padding;

//     int pw = photo->width;
//     int ph = photo->height;

//     int usable_w = canvas->width - 2 * padding;
//     int usable_h = canvas->height - 2 * padding;

//     float scale_x = (float)usable_w / (cols * pw + (cols - 1) * gap);
//     float scale_y = (float)usable_h / (rows * ph + (rows - 1) * gap);

//     float scale = scale_x < scale_y ? scale_x : scale_y;
//     if (scale > 1.0f)
//         scale = 1.0f;

//     int final_pw = pw * scale;
//     int final_ph = ph * scale;

//     img_t scaled;
//     if (!img_resize(photo, &scaled, &ctx->pool, final_pw, final_ph))
//         return 0;

//     int total_w = cols * final_pw + (cols - 1) * gap;

//     int start_x = (canvas->width - total_w) / 2;
//     int start_y = padding;

//     int total_cells = cols * rows;

//     img_cell_t *cells =
//         (img_cell_t *)mp_alloc(&ctx->pool, sizeof(img_cell_t) * total_cells);

//     if (!cells)
//         return 0;

//     int idx = 0;

//     for (int r = 0; r < rows; r++)
//     {
//         for (int c = 0; c < cols; c++)
//         {
//             int x = start_x + c * (final_pw + gap);
//             int y = start_y + r * (final_ph + gap);

//             img_blit_avx2(canvas, &scaled, x, y);

//             cells[idx].x = x;
//             cells[idx].y = y;
//             cells[idx].w = final_pw;
//             cells[idx].h = final_ph;
//             idx++;
//         }
//     }

//     layout_store(ctx, cells, total_cells);

//     return 1;
// }

#include "imgengine/layout.h"
#include "imgengine/context.h"
#include "imgengine/api.h"
#include "imgengine/resize.h"
#include <math.h>

extern void img_blit_avx2(img_t *, const img_t *, int, int);

int layout_grid(img_t *canvas, const img_t *photo, const img_job_t *job, img_ctx_t *ctx)
{
    // 1. Calculate usable area
    int usable_w = canvas->width - (2 * job->padding);
    int usable_h = canvas->height - (2 * job->padding);

    // 2. Determine Scale (Senior Move: prevent floating point drift with floor)
    float needed_w = (float)(job->cols * photo->width + (job->cols - 1) * job->gap);
    float needed_h = (float)(job->rows * photo->height + (job->rows - 1) * job->gap);

    float scale_x = (float)usable_w / needed_w;
    float scale_y = (float)usable_h / needed_h;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale > 1.0f) scale = 1.0f;

    int final_pw = (int)(photo->width * scale);
    int final_ph = (int)(photo->height * scale);

    // 3. Resize once into the pool (High-speed reuse)
    img_t scaled;
    if (!img_resize(photo, &scaled, &ctx->pool, final_pw, final_ph))
        return 0;

    // 4. Center the entire grid on the canvas
    int grid_w = job->cols * final_pw + (job->cols - 1) * job->gap;
    int start_x = (canvas->width - grid_w) / 2;
    int start_y = job->padding; // Top-heavy layout is standard for print

    // 5. Memory-Safe Cell Allocation
    int total_cells = job->cols * job->rows;
    img_cell_t *cells = (img_cell_t *)mp_alloc(&ctx->pool, sizeof(img_cell_t) * total_cells);
    if (!cells) return 0;

    // 6. Blit & Store (The Hot Path)
    for (int r = 0; r < job->rows; r++) {
        for (int c = 0; c < job->cols; c++) {
            int x = start_x + c * (final_pw + job->gap);
            int y = start_y + r * (final_ph + job->gap);

            // AVX2 Power
            img_blit_avx2(canvas, &scaled, x, y);

            int idx = r * job->cols + c;
            cells[idx].x = x;
            cells[idx].y = y;
            cells[idx].w = final_pw;
            cells[idx].h = final_ph;
        }
    }

    // 7. Persist to Context for Plugins
    layout_store(ctx, cells, total_cells);

    return 1;
}
