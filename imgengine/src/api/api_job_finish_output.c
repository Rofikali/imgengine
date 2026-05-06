// ./src/api/api_job_finish_output.c
#include "api/api_job_finish_output_internal.h"

int is_pdf_output(const char *path);

img_result_t img_api_finish_job_output(img_ctx_t *ctx, img_canvas_t *canvas,
                                       const img_layout_t *layout, const img_job_t *job,
                                       const char *output_path) {
    if (!ctx || !canvas || !layout || !job || !output_path)
        return IMG_ERR_SECURITY;

    if (is_pdf_output(output_path))
        return img_api_finish_job_output_pdf(canvas, layout, job, output_path);

    return img_api_finish_job_output_file(ctx, canvas, layout, job, output_path);
}
