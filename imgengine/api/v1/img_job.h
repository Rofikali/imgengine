// ./api/v1/img_job.h

#ifndef IMGENGINE_API_JOB_H
#define IMGENGINE_API_JOB_H

#include <stdint.h>
#include "api/v1/img_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * img_scale_mode_t
     *
     * IMG_FIT:  scale to fit within photo dimensions, letterbox
     * IMG_FILL: scale to fill photo dimensions, center crop
     */
    typedef enum
    {
        IMG_FIT = 0,
        IMG_FILL = 1,
    } img_scale_mode_t;

    /*
     * img_job_t
     *
     * Describes a full print layout job.
     * ABI-STABLE: add fields only at end, never reorder.
     * Versioned via abi_version field.
     *
     * Units:
     *   photo_w_cm, photo_h_cm: centimeters
     *   dpi:                    dots per inch
     *   all _px fields:         pixels at the specified DPI
     */
    typedef struct img_job
    {
        uint32_t abi_version; /* must be IMG_JOB_ABI_VERSION */

        /* photo dimensions */
        float photo_w_cm;
        float photo_h_cm;
        uint32_t dpi;

        /* grid layout */
        uint32_t cols;
        uint32_t rows;
        uint32_t gap;     /* gap between photos (px) */
        uint32_t padding; /* page margin (px) */

        /* photo decoration */
        uint32_t border_px;

        /* print registration */
        uint32_t bleed_px;
        uint32_t crop_mark_px;
        uint32_t crop_thickness;
        uint32_t crop_offset_px;

        /* fill/fit */
        img_scale_mode_t mode;

        /* background color (RGB) */
        uint8_t bg_r;
        uint8_t bg_g;
        uint8_t bg_b;
        uint8_t _pad;

    } img_job_t;

#define IMG_JOB_ABI_VERSION 1

    /*
     * img_job_defaults()
     *
     * Initialize job with production defaults.
     * Always call this before setting fields — future ABI versions
     * may add fields; defaults ensure zero-init of unknown fields.
     */
    static inline void img_job_defaults(img_job_t *job)
    {
        if (!job)
            return;

        *job = (img_job_t){
            .abi_version = IMG_JOB_ABI_VERSION,
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

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_JOB_H */