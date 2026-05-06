// ./src/pipeline/canvas.c

#define _GNU_SOURCE

#include "pipeline/canvas_internal.h"

img_result_t img_canvas_init(img_canvas_t *canvas, img_slab_pool_t *pool, const img_job_t *job) {
    if (!canvas || !pool || !job)
        return IMG_ERR_SECURITY;

    uint32_t pw = img_cm_to_px(IMG_A4_W_CM, job->dpi);
    uint32_t ph = img_cm_to_px(IMG_A4_H_CM, job->dpi);

    canvas->page_w_px = pw;
    canvas->page_h_px = ph;
    img_canvas_compute_geometry(canvas, job, pw, ph);

    img_result_t r = img_canvas_alloc_buffer(canvas, pool, pw, ph);
    if (r != IMG_SUCCESS)
        return r;

    const uint32_t bg_signature =
        ((uint32_t)job->bg_r << 16) | ((uint32_t)job->bg_g << 8) | (uint32_t)job->bg_b;

    if (canvas->initialized && canvas->bg_signature == bg_signature)
        return IMG_SUCCESS;

    img_canvas_fill_background(canvas, job);
    return IMG_SUCCESS;
}
