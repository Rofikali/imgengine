// ./src/pipeline/canvas_geometry.c
#include "pipeline/canvas_internal.h"

img_result_t img_canvas_compute_geometry(img_canvas_t *canvas, const img_job_t *job, uint32_t pw,
                                         uint32_t ph) {
    if (!canvas || !job || !pw || !ph || !job->cols || !job->rows)
        return IMG_ERR_SECURITY;

    uint64_t pad2 = (uint64_t)job->padding * 2u;
    uint64_t h_gaps = (job->cols > 1u) ? (uint64_t)(job->cols - 1u) * job->gap : 0u;
    uint64_t v_gaps = (job->rows > 1u) ? (uint64_t)(job->rows - 1u) * job->gap : 0u;

    if (pad2 + h_gaps >= pw || pad2 + v_gaps >= ph)
        return IMG_ERR_SECURITY;

    uint32_t avail_w = (uint32_t)((uint64_t)pw - pad2 - h_gaps);
    uint32_t avail_h = (uint32_t)((uint64_t)ph - pad2 - v_gaps);
    uint32_t max_pw = avail_w / job->cols;
    uint32_t max_ph = avail_h / job->rows;

    if (!max_pw || !max_ph)
        return IMG_ERR_SECURITY;

    uint32_t photo_w = img_cm_to_px(job->photo_w_cm, job->dpi);
    uint32_t photo_h = img_cm_to_px(job->photo_h_cm, job->dpi);

    if (photo_w > max_pw || photo_h > max_ph) {
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

    if (photo_w > max_pw)
        photo_w = max_pw;
    if (photo_h > max_ph)
        photo_h = max_ph;

    canvas->photo_w_px = photo_w;
    canvas->photo_h_px = photo_h;

    uint64_t grid_w = (uint64_t)job->cols * photo_w + h_gaps;
    canvas->start_x = (uint32_t)(((uint64_t)pw - grid_w) / 2u);
    canvas->start_y = job->padding;
    return IMG_SUCCESS;
}
