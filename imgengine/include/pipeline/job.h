// ./include/pipeline/job.h
// include/pipeline/job.h  (NEW FILE)

// ================================================================
// STEP 3 OF 6: Move img_job_t to pipeline/
// Fixes: pipeline depending on api for img_job_t (6 violations)
// img_job_t is a pipeline concept, not an API concept.
// The API layer re-exports it — that's the correct direction.
// ================================================================

#ifndef IMGENGINE_PIPELINE_JOB_H
#define IMGENGINE_PIPELINE_JOB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * img_scale_mode_t and img_job_t — print layout job descriptor.
 *
 * Moved from api/v1/img_job.h to pipeline/job.h because:
 *   - pipeline/canvas.h needs img_job_t
 *   - pipeline/layout.h needs img_job_t
 *   - pipeline/ (index 4) must not depend on api/ (index 9)
 *
 * api/v1/img_job.h now re-exports this for public API consumers.
 */

typedef enum {
    IMG_FIT = 0,
    IMG_FILL = 1,
} img_scale_mode_t;

typedef enum {
    IMG_JOB_TEMPLATE_CUSTOM = 0,
    IMG_JOB_TEMPLATE_PASSPORT_45X35 = 1,
    IMG_JOB_TEMPLATE_PASSPORT_38X35 = 2,
    IMG_JOB_TEMPLATE_PRINTREADY_6X6 = 3,
} img_job_template_t;

typedef struct img_job {
    uint32_t abi_version;
    uint32_t template_id;

    float photo_w_cm;
    float photo_h_cm;
    uint32_t dpi;

    uint32_t cols;
    uint32_t rows;
    uint32_t gap;
    uint32_t padding;

    uint32_t border_px;
    uint32_t bleed_px;
    uint32_t crop_mark_px;
    uint32_t crop_thickness;
    uint32_t crop_offset_px;

    img_scale_mode_t mode;

    uint8_t bg_r;
    uint8_t bg_g;
    uint8_t bg_b;
    uint8_t _pad;

} img_job_t;

#define IMG_JOB_ABI_VERSION 2

static inline void img_job_defaults(img_job_t *job) {
    if (!job)
        return;
    *job = (img_job_t){
        .abi_version = IMG_JOB_ABI_VERSION,
        .template_id = IMG_JOB_TEMPLATE_CUSTOM,
        .photo_w_cm = 4.5f,
        .photo_h_cm = 3.5f,
        .dpi = 300,
        .cols = 2,
        .rows = 3,
        .gap = 15,
        .padding = 20,
        .border_px = 2,
        .bleed_px = 10,
        .crop_mark_px = 20,
        .crop_thickness = 2,
        .crop_offset_px = 8,
        .mode = IMG_FILL,
        .bg_r = 255,
        .bg_g = 255,
        .bg_b = 255,
    };
}

static inline void img_job_apply_template(img_job_t *job, img_job_template_t template_id) {
    if (!job)
        return;

    switch (template_id) {
    case IMG_JOB_TEMPLATE_PASSPORT_45X35:
        job->template_id = template_id;
        job->photo_w_cm = 4.5f;
        job->photo_h_cm = 3.5f;
        job->cols = 6;
        job->rows = 6;
        job->gap = 15;
        job->padding = 20;
        job->border_px = 2;
        job->bleed_px = 1;
        job->crop_mark_px = 5;
        job->crop_thickness = 10;
        job->crop_offset_px = 5;
        job->mode = IMG_FILL;
        job->bg_r = 255;
        job->bg_g = 255;
        job->bg_b = 255;
        break;
    case IMG_JOB_TEMPLATE_PASSPORT_38X35:
        job->template_id = template_id;
        job->photo_w_cm = 3.8f;
        job->photo_h_cm = 3.5f;
        job->cols = 6;
        job->rows = 6;
        job->gap = 15;
        job->padding = 20;
        job->border_px = 2;
        job->bleed_px = 1;
        job->crop_mark_px = 5;
        job->crop_thickness = 10;
        job->crop_offset_px = 5;
        job->mode = IMG_FILL;
        job->bg_r = 255;
        job->bg_g = 255;
        job->bg_b = 255;
        break;
    case IMG_JOB_TEMPLATE_PRINTREADY_6X6:
        job->template_id = template_id;
        job->cols = 6;
        job->rows = 6;
        job->gap = 15;
        job->padding = 20;
        job->border_px = 2;
        job->bleed_px = 1;
        job->crop_mark_px = 5;
        job->crop_thickness = 10;
        job->crop_offset_px = 5;
        job->mode = IMG_FILL;
        job->bg_r = 255;
        job->bg_g = 255;
        job->bg_b = 255;
        break;
    case IMG_JOB_TEMPLATE_CUSTOM:
    default:
        job->template_id = IMG_JOB_TEMPLATE_CUSTOM;
        break;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_PIPELINE_JOB_H */
