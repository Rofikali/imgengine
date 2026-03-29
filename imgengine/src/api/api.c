// // src/api/api.c

// #include "imgengine/api.h"
// #include "imgengine/error.h"
// #include "imgengine/image.h"
// #include "imgengine/io.h"
// #include "imgengine/crop.h"
// #include "imgengine/resize.h"
// #include "imgengine/border.h"
// #include "imgengine/layout.h"
// #include "imgengine/plugins/plugin_runner.h"
// #include "imgengine/plugins/plugin_registry.h"

// // #include "imgengine/plugin/register_all.h"

// #include <stdio.h>
// #include <string.h>

// // 🔥 PRECHECK FUNCTION
// static int layout_fits(int canvas_w, int canvas_h,
//                        int pw, int ph,
//                        int cols, int rows,
//                        int gap, int padding)
// {
//     int total_w = cols * pw + (cols - 1) * gap + 2 * padding;
//     int total_h = rows * ph + (rows - 1) * gap + 2 * padding;

//     return (total_w <= canvas_w) && (total_h <= canvas_h);
// }

// int imgengine_run(img_ctx_t *ctx,
//                   const char *input,
//                   const char *output,
//                   const img_job_t *job)
// {
//     if (!input || !output || !job)
//         return IMG_ERR_INVALID;

//     if (job->dpi <= 0 || job->photo_w_cm <= 0 || job->photo_h_cm <= 0)
//         return IMG_ERR_INVALID;

//     // printf("Loading image: %s\n", input);

//     img_t src;
//     if (!img_load(input, &src))
//         return IMG_ERR_IO;

//     int pw = (job->photo_w_cm / 2.54f) * job->dpi;
//     int ph = (job->photo_h_cm / 2.54f) * job->dpi;

//     img_t cropped;
//     if (!img_center_crop(&src, &cropped, &ctx->pool, pw, ph))
//         return IMG_ERR_ALLOC;

//     img_t resized;
//     if (!img_resize(&cropped, &resized, &ctx->pool, pw, ph))
//         return IMG_ERR_ALLOC;

//     img_t bordered;
//     if (!img_add_border(&resized, &bordered, &ctx->pool, job->border_px))
//         return IMG_ERR_ALLOC;

//     img_t canvas;
//     if (!img_create(&canvas, &ctx->pool, 2480, 3508, 3))
//         return IMG_ERR_ALLOC;

//     // Fill white background
//     for (int i = 0; i < 2480 * 3508 * 3; i++)
//         canvas.data[i] = 255;

//     int cols = job->cols;
//     int rows = job->rows;

//     printf("Manual layout cols=%d rows=%d\n", cols, rows);

//     int fits = layout_fits(canvas.width, canvas.height,
//                            bordered.width, bordered.height,
//                            cols, rows,
//                            job->gap, job->padding);

//     if (!fits)
//     {
//         printf("⚠ Layout too large → auto scaling applied (fit mode)\n");
//         // printf("Scale factor: %.2f\n", scale);
//     }

//     register_all_plugins();

//     if (!layout_grid(&canvas, &bordered, job, ctx))
//         return IMG_ERR_INVALID;

//     if (!run_plugins(ctx, &canvas, job))
//         return IMG_ERR_INVALID;

//     // detect output format
//     const char *ext = strrchr(output, '.');

//     if (ext && strcmp(ext, ".pdf") == 0)
//     {
//         if (!img_save_pdf(output, &canvas))
//             return IMG_ERR_IO;
//     }
//     else
//     {
//         if (!img_save_png(output, &canvas))
//             return IMG_ERR_IO;
//     }

//     // if (!img_save_png(output, &canvas))
//     //     return IMG_ERR_IO;

//     return IMG_OK;
// }


// src/api/api.c

#include "imgengine/api.h"
#include "imgengine/context.h"
#include "imgengine/image.h"
#include "imgengine/io.h"
#include "imgengine/crop.h"
#include "imgengine/resize.h"
#include "imgengine/border.h"
#include "imgengine/layout.h"
#include "imgengine/plugins/plugin_runner.h"
#include "imgengine/plugins/plugin_registry.h"
#include <stdio.h>
#include <string.h>
#include <strings.h> // for strcasecmp on Linux
#include <stdlib.h>

/* --- Lifecycle Implementation --- */

// img_status_t imgengine_create(img_ctx_t **out_ctx, size_t pool_size)
// {
//     if (!out_ctx)
//         return IMG_ERR_INVALID;

//     img_ctx_t *ctx = (img_ctx_t *)malloc(sizeof(img_ctx_t));
//     if (!ctx)
//         return IMG_ERR_ALLOC;

//     mp_init(&ctx->pool, pool_size);
//     if (!ctx->pool.buffer)
//     {
//         free(ctx);
//         return IMG_ERR_ALLOC;
//     }

//     ctx->layout = NULL; // Initialized by layout_grid later

//     // Register plugins once per context for thread-safety
//     register_all_plugins();

//     *out_ctx = ctx;
//     return IMG_OK;
// }

/* --- Lifecycle Implementation --- */

img_status_t imgengine_create(img_ctx_t **out_ctx, size_t pool_size)
{
    if (!out_ctx)
        return IMG_ERR_INVALID;

    img_ctx_t *ctx = (img_ctx_t *)malloc(sizeof(img_ctx_t));
    if (!ctx)
        return IMG_ERR_ALLOC;

    // Initialize memory pool
    mp_init(&ctx->pool, pool_size);
    if (!ctx->pool.buffer)
    {
        free(ctx);
        return IMG_ERR_ALLOC;
    }

    ctx->layout = NULL; 
    ctx->plugin_count = 0; // Kernel-grade: explicitly zero out counters

    // ✅ FIXED: Pass 'ctx' to register_all_plugins
    register_all_plugins(ctx);

    *out_ctx = ctx;
    return IMG_OK;
}


void imgengine_destroy(img_ctx_t *ctx)
{
    if (ctx)
    {
        mp_destroy(&ctx->pool);
        free(ctx);
    }
}

/* --- Core Logic Implementation --- */

img_status_t imgengine_run(img_ctx_t *ctx, const char *input, const char *output, const img_job_t *job)
{
    if (!ctx || !input || !output || !job)
        return IMG_ERR_INVALID;

    // Senior Move: Reset pool at start of every run to reuse memory
    mp_reset(&ctx->pool);

    img_t src, cropped, resized, bordered, canvas;

    // 1. Load Image
    if (!img_load(input, &src))
        return IMG_ERR_IO;

    // 2. Physical to Pixel Calculation
    int pw = (int)((job->photo_w_cm / 2.54f) * job->dpi);
    int ph = (int)((job->photo_h_cm / 2.54f) * job->dpi);

    // 3. Processing Pipeline (Memory Pool Protected)
    if (!img_center_crop(&src, &cropped, &ctx->pool, pw, ph))
        return IMG_ERR_ALLOC;
    if (!img_resize(&cropped, &resized, &ctx->pool, pw, ph))
        return IMG_ERR_ALLOC;
    if (!img_add_border(&resized, &bordered, &ctx->pool, job->border_px))
        return IMG_ERR_ALLOC;

    // 4. Create A4 Canvas (Standard 300DPI: 2480x3508)
    if (!img_create(&canvas, &ctx->pool, 2480, 3508, 3))
        return IMG_ERR_ALLOC;
    memset(canvas.data, 255, canvas.width * canvas.height * 3); // White Background

    // 5. Layout Engine
    if (!layout_grid(&canvas, &bordered, job, ctx))
        return IMG_ERR_INVALID;

    // 6. Plugin Execution (Bleed, Crop Marks)
    if (!run_plugins(ctx, &canvas, job))
        return IMG_ERR_INVALID;

    // 7. Smart Save (Auto-detect PDF/PNG)
    const char *ext = strrchr(output, '.');
    if (ext && (strcasecmp(ext, ".pdf") == 0))
    {
        if (!img_save_pdf(output, &canvas))
            return IMG_ERR_IO;
    }
    else
    {
        if (!img_save_png(output, &canvas))
            return IMG_ERR_IO;
    }

    return IMG_OK;
}
