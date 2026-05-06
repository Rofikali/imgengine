// ./src/runtime/job_postfx.c
#include "runtime/job_exec.h"

#include "pipeline/border.h"

img_result_t img_apply_bleed(img_buffer_t *canvas, const img_layout_t *layout, uint32_t bleed_px);

img_result_t img_draw_crop_marks(img_buffer_t *canvas, const img_layout_t *layout,
                                 const img_job_t *job);

img_result_t img_runtime_apply_job_postfx(img_canvas_t *canvas, const img_layout_t *layout,
                                          const img_job_t *job) {
    if (!canvas || !layout || !job)
        return IMG_ERR_SECURITY;

    img_result_t r = IMG_SUCCESS;

    if (job->border_px > 0) {
        r = img_draw_borders(&canvas->buf, layout, job->border_px);
        if (r != IMG_SUCCESS)
            return r;
    }

    r = img_apply_bleed(&canvas->buf, layout, job->bleed_px);
    if (r != IMG_SUCCESS)
        return r;

    return img_draw_crop_marks(&canvas->buf, layout, job);
}
