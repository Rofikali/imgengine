// src/api/api.c

#include "imgengine/api.h"
#include "imgengine/error.h"
#include "imgengine/image.h"
#include "imgengine/io.h"
#include "imgengine/crop.h"
#include "imgengine/resize.h"
#include "imgengine/border.h"
#include "imgengine/layout.h"

#include <stdio.h>

// 🔥 PRECHECK FUNCTION
static int layout_fits(int canvas_w, int canvas_h,
                       int pw, int ph,
                       int cols, int rows,
                       int gap, int padding)
{
    int total_w = cols * pw + (cols - 1) * gap + 2 * padding;
    int total_h = rows * ph + (rows - 1) * gap + 2 * padding;

    return (total_w <= canvas_w) && (total_h <= canvas_h);
}

int imgengine_run(img_ctx_t *ctx,
                  const char *input,
                  const char *output,
                  const img_job_t *job)
{
    if (!input || !output || !job)
        return IMG_ERR_INVALID;

    if (job->dpi <= 0 || job->photo_w_cm <= 0 || job->photo_h_cm <= 0)
        return IMG_ERR_INVALID;

    printf("Loading image: %s\n", input);

    img_t src;
    if (!img_load(input, &src))
        return IMG_ERR_IO;

    int pw = (job->photo_w_cm / 2.54f) * job->dpi;
    int ph = (job->photo_h_cm / 2.54f) * job->dpi;

    img_t cropped;
    if (!img_center_crop(&src, &cropped, &ctx->pool, pw, ph))
        return IMG_ERR_ALLOC;

    img_t resized;
    if (!img_resize(&cropped, &resized, &ctx->pool, pw, ph))
        return IMG_ERR_ALLOC;

    img_t bordered;
    if (!img_add_border(&resized, &bordered, &ctx->pool, job->border_px))
        return IMG_ERR_ALLOC;

    img_t canvas;
    if (!img_create(&canvas, &ctx->pool, 2480, 3508, 3))
        return IMG_ERR_ALLOC;

    // Fill white background
    for (int i = 0; i < 2480 * 3508 * 3; i++)
        canvas.data[i] = 255;

    int cols = job->cols;
    int rows = job->rows;

    printf("Manual layout cols=%d rows=%d\n", cols, rows);

    // 🔥 STRICT PRECHECK (NO AUTO FIX)
    // if (!layout_fits(canvas.width, canvas.height,
    //                  bordered.width, bordered.height,
    //                  cols, rows,
    //                  job->gap, job->padding))
    // {
    //     printf("❌ Layout does NOT fit on canvas\n");
    //     printf("👉 Reduce cols/rows, gap, padding or photo size\n");
    //     return IMG_ERR_INVALID;
    // }
    int fits = layout_fits(canvas.width, canvas.height,
                           bordered.width, bordered.height,
                           cols, rows,
                           job->gap, job->padding);

    if (!fits)
    {
        printf("⚠ Layout too large → auto scaling applied (fit mode)\n");
        // printf("Scale factor: %.2f\n", scale);
    }

    if (!layout_grid(&canvas, &bordered, cols, rows,
                     job->gap, job->padding, ctx))
        return IMG_ERR_INVALID;

    if (!img_save_png(output, &canvas))
        return IMG_ERR_IO;

    return IMG_OK;
}