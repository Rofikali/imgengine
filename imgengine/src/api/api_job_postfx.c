// ./src/api/api_job_postfx.c
#include "api/api_job_internal.h"
#include "runtime/job_exec.h"

img_result_t img_apply_job_postfx(
    img_canvas_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job)
{
    return img_runtime_apply_job_postfx(canvas, layout, job);
}
