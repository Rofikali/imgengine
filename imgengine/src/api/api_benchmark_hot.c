#include "api/api_benchmark_internal.h"
#include "api/api_process_fast_internal.h"
#include "runtime/benchmark_hot.h"
#include "runtime/job_exec.h"

img_result_t img_api_hot_bench_init(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_hot_bench_state_t *state)
{
    return img_api_hot_bench_init_with_template(
        engine,
        input,
        input_size,
        IMG_JOB_TEMPLATE_CUSTOM,
        state);
}

img_result_t img_api_hot_bench_init_with_template(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_job_template_t preset_template,
    img_hot_bench_state_t *state)
{
    img_buffer_t decoded = {0};
    img_result_t r = decode_image_secure(engine, input, input_size, &decoded);
    if (r != IMG_SUCCESS)
        return r;

    r = img_runtime_hot_bench_init(engine, &decoded, preset_template, state);
    if (r != IMG_SUCCESS)
        img_runtime_release_raw_buffer(engine, &decoded);

    return r;
}

img_result_t img_api_hot_bench_step(
    img_hot_bench_state_t *state)
{
    return img_runtime_hot_bench_step(state);
}

void img_api_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state)
{
    img_runtime_hot_bench_destroy(engine, state);
}
