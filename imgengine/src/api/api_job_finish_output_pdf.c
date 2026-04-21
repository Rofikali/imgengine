// ./src/api/api_job_finish_output_pdf.c
#include "api/api_job_finish_output_internal.h"
#include "api/io_vtable.h"

#include <stdio.h>

img_result_t img_api_finish_job_output_pdf(
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job,
    const char *output_path)
{
    if (!g_io_vtable.encode_pdf)
        return IMG_ERR_INTERNAL;

    img_result_t r = img_apply_job_postfx(canvas, layout, job);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] output postfx failed: %s\n", img_result_name(r));
        return r;
    }

    r = g_io_vtable.encode_pdf(&canvas->buf, output_path, job->dpi);
    if (r != IMG_SUCCESS)
        fprintf(stderr, "[JOB] pdf encode failed: %s\n", img_result_name(r));
    return r;
}
