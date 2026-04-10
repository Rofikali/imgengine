// api/v1/img_compat.h

#ifndef IMGENGINE_COMPAT_H
#define IMGENGINE_COMPAT_H

#include "img_error.h"
#include "img_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct img_ctx img_ctx_t;

/* 🔥 YOUR EXISTING JOB STRUCT (UNCHANGED) */
typedef struct img_job {
    float photo_w_cm;
    float photo_h_cm;
    int dpi;
    int bleed_px;
    int crop_mark_px;
    int crop_thickness;
    int crop_offset_px;
    int cols;
    int rows;
    int gap;
    int border_px;
    int padding;
    int mode;
    unsigned char bg_r;
    unsigned char bg_g;
    unsigned char bg_b;
} img_job_t;

/* 🔥 SAME API (NO BREAKAGE) */
int imgengine_create(img_ctx_t **out_ctx, size_t pool_size);
void imgengine_destroy(img_ctx_t *ctx);

int imgengine_run(img_ctx_t *ctx, const char *input, const char *output, const img_job_t *job);

#ifdef __cplusplus
}
#endif

#endif