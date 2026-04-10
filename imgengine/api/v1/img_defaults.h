// api/v1/img_defaults.h

#ifndef IMGENGINE_DEFAULTS_H
#define IMGENGINE_DEFAULTS_H

#include "img_compat.h"

static inline img_job_t img_job_default(void) {
    img_job_t j = {.photo_w_cm = 3.5f,
                   .photo_h_cm = 4.5f,
                   .dpi = 300,
                   .cols = 6,
                   .rows = 6,
                   .gap = 10,
                   .border_px = 10,
                   .padding = 5,
                   .crop_mark_px = 20,
                   .crop_thickness = 2,
                   .bleed_px = 5,
                   .crop_offset_px = 5,
                   .mode = 1, // IMG_FILL
                   .bg_r = 255,
                   .bg_g = 255,
                   .bg_b = 255};
    return j;
}

#endif