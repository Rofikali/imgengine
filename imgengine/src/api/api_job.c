// ./src/api/api_job.c
#include "api/v1/img_api.h"
#include "api/api_job_internal.h"

img_result_t img_api_run_job(img_engine_t *engine, const char *input_path, const char *output_path,
                             const img_job_t *job) {
    return img_api_run_job_impl(engine, input_path, output_path, job);
}

img_result_t img_api_run_job_raw(img_engine_t *engine, const char *input_path, const img_job_t *job,
                                 uint8_t **output, size_t *output_size) {
    return img_api_run_job_raw_impl(engine, input_path, job, output, output_size);
}

img_result_t img_api_run_job_rgb24(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                   uint32_t width, uint32_t height, uint32_t stride,
                                   const char *output_path, const img_job_t *job) {
    return img_api_run_job_rgb24_impl(engine, input, input_size, width, height, stride, output_path,
                                      job);
}

img_result_t img_api_run_job_rgb24_raw(img_engine_t *engine, const uint8_t *input,
                                       size_t input_size, uint32_t width, uint32_t height,
                                       uint32_t stride, const img_job_t *job, uint8_t **output,
                                       size_t *output_size) {
    return img_api_run_job_rgb24_raw_impl(engine, input, input_size, width, height, stride, job,
                                          output, output_size);
}
