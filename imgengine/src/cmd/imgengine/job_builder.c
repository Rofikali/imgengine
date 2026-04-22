// ./src/cmd/imgengine/job_builder.c

#include "cmd/imgengine/job_builder.h"

int img_build_job(const img_cli_options_t *opts, img_job_t *job)
{
    if (!opts || !job)
        return -1;

    img_job_defaults(job);
    if (opts->has_preset)
        img_job_apply_template(job, opts->preset_template);

    /* layout */
    if (opts->has_cols)
        job->cols = opts->cols;
    if (opts->has_rows)
        job->rows = opts->rows;
    if (opts->has_gap)
        job->gap = opts->gap;
    if (opts->has_padding)
        job->padding = opts->padding;

    /* photo */
    if (opts->has_photo_w_cm)
        job->photo_w_cm = opts->photo_w_cm;
    if (opts->has_photo_h_cm)
        job->photo_h_cm = opts->photo_h_cm;
    if (opts->has_dpi)
        job->dpi = opts->dpi;
    if (opts->has_border)
        job->border_px = opts->border;

    /* print */
    if (opts->has_bleed)
        job->bleed_px = opts->bleed;
    if (opts->has_crop_mark)
        job->crop_mark_px = opts->crop_mark;
    if (opts->has_crop_thickness)
        job->crop_thickness = opts->crop_thickness;
    if (opts->has_crop_offset)
        job->crop_offset_px = opts->crop_offset;

    if (opts->mode_set)
        job->mode = opts->mode;

    return 0;
}
