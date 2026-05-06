// ./src/pipeline/canvas_geometry.c
#include "pipeline/canvas_internal.h"

void img_canvas_compute_geometry(img_canvas_t *canvas, const img_job_t *job, uint32_t pw,
                                 uint32_t ph) {
    uint32_t photo_w = img_cm_to_px(job->photo_w_cm, job->dpi);
    uint32_t photo_h = img_cm_to_px(job->photo_h_cm, job->dpi);

    uint32_t pad2 = job->padding * 2;
    uint32_t h_gaps = (job->cols > 1) ? (job->cols - 1) * job->gap : 0;
    uint32_t v_gaps = (job->rows > 1) ? (job->rows - 1) * job->gap : 0;

    uint32_t avail_w = (pw > pad2 + h_gaps) ? pw - pad2 - h_gaps : 1;
    uint32_t avail_h = (ph > pad2 + v_gaps) ? ph - pad2 - v_gaps : 1;

    uint32_t max_pw = avail_w / job->cols;
    uint32_t max_ph = avail_h / job->rows;

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

    canvas->photo_w_px = photo_w;
    canvas->photo_h_px = photo_h;

    uint32_t grid_w = job->cols * photo_w + h_gaps;
    canvas->start_x = (pw > grid_w) ? (pw - grid_w) / 2 : job->padding;
    canvas->start_y = job->padding;
}
