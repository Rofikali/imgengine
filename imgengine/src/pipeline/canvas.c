// ./src/pipeline/canvas.c

#define _GNU_SOURCE

#include "pipeline/canvas.h"
#include "core/result.h"
#include <string.h>
// #include "pipeline/job.h"
#include "core/result.h"

img_result_t img_canvas_init(
    img_canvas_t *canvas,
    img_slab_pool_t *pool,
    const img_job_t *job)
{
    if (!canvas || !pool || !job)
        return IMG_ERR_SECURITY;

    uint32_t pw = img_cm_to_px(IMG_A4_W_CM, job->dpi);
    uint32_t ph = img_cm_to_px(IMG_A4_H_CM, job->dpi);

    canvas->page_w_px = pw;
    canvas->page_h_px = ph;

    uint32_t photo_w = img_cm_to_px(job->photo_w_cm, job->dpi);
    uint32_t photo_h = img_cm_to_px(job->photo_h_cm, job->dpi);

    /*
     * Available space after removing padding and gaps between photos.
     *
     *   avail_w = page_w - 2*padding - (cols-1)*gap
     *   max_pw  = avail_w / cols
     *
     * This correctly separates padding (outer margin) from
     * gap (space BETWEEN photos). Both produce white space.
     * Photos are sized to fit within avail_w / cols.
     */
    uint32_t pad2 = job->padding * 2;
    uint32_t h_gaps = (job->cols > 1) ? (job->cols - 1) * job->gap : 0;
    uint32_t v_gaps = (job->rows > 1) ? (job->rows - 1) * job->gap : 0;

    uint32_t avail_w = (pw > pad2 + h_gaps) ? pw - pad2 - h_gaps : 1;
    uint32_t avail_h = (ph > pad2 + v_gaps) ? ph - pad2 - v_gaps : 1;

    uint32_t max_pw = avail_w / job->cols;
    uint32_t max_ph = avail_h / job->rows;

    /* Scale down proportionally if photos don't fit */
    if (photo_w > max_pw || photo_h > max_ph)
    {
        float sx = (max_pw > 0) ? (float)max_pw / (float)photo_w : 1.0f;
        float sy = (max_ph > 0) ? (float)max_ph / (float)photo_h : 1.0f;
        float s = (sx < sy) ? sx : sy;
        photo_w = (uint32_t)((float)photo_w * s);
        photo_h = (uint32_t)((float)photo_h * s);
    }

    if (photo_w == 0)
        photo_w = 1;
    if (photo_h == 0)
        photo_h = 1;

    canvas->photo_w_px = photo_w;
    canvas->photo_h_px = photo_h;

    /*
     * Grid origin: horizontally centered, top-aligned.
     *
     * grid_w = cols * photo_w + (cols-1) * gap
     * start_x = (page_w - grid_w) / 2
     * start_y = padding
     *
     * White space sources:
     *   - Left/right of grid: (page_w - grid_w) / 2
     *   - Top of grid: padding
     *   - Between photos: gap (background color shows through)
     */
    uint32_t grid_w = job->cols * photo_w + h_gaps;

    canvas->start_x = (pw > grid_w) ? (pw - grid_w) / 2 : job->padding;
    canvas->start_y = job->padding;

    size_t required = (size_t)pw * ph * 3;
    size_t block = img_slab_block_size(pool);

    if (required > block)
        return IMG_ERR_NOMEM;

    uint8_t *mem = img_slab_alloc(pool);
    if (!mem)
        return IMG_ERR_NOMEM;

    /* White background — gap pixels are never blitted over */
    for (size_t i = 0; i < required; i += 3)
    {
        mem[i] = job->bg_r;
        mem[i + 1] = job->bg_g;
        mem[i + 2] = job->bg_b;
    }

    canvas->buf.data = mem;
    canvas->buf.width = pw;
    canvas->buf.height = ph;
    canvas->buf.channels = 3;
    canvas->buf.stride = pw * 3;

    return IMG_SUCCESS;
}

void img_canvas_free(img_canvas_t *canvas, img_slab_pool_t *pool)
{
    if (canvas && canvas->buf.data && pool)
    {
        img_slab_free(pool, canvas->buf.data);
        canvas->buf.data = NULL;
    }
}
