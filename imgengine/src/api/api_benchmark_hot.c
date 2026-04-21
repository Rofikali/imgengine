#include "api/api_benchmark_internal.h"
#include "runtime/benchmark_hot.h"

img_result_t img_api_hot_bench_init(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_hot_bench_state_t *state)
{
    return img_runtime_hot_bench_init(engine, input, input_size, state);
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
