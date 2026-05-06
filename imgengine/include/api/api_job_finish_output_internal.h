// ./include/api/api_job_finish_output_internal.h
#ifndef IMGENGINE_API_JOB_FINISH_OUTPUT_INTERNAL_H
#define IMGENGINE_API_JOB_FINISH_OUTPUT_INTERNAL_H

#include "api/api_job_internal.h"

img_result_t img_api_finish_job_output_pdf(img_canvas_t *canvas, const img_layout_t *layout,
                                           const img_job_t *job, const char *output_path);

img_result_t img_api_finish_job_output_file(img_ctx_t *ctx, img_canvas_t *canvas,
                                            const img_layout_t *layout, const img_job_t *job,
                                            const char *output_path);

#endif /* IMGENGINE_API_JOB_FINISH_OUTPUT_INTERNAL_H */
