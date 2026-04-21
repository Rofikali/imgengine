#ifndef IMGENGINE_API_BENCHMARK_INTERNAL_H
#define IMGENGINE_API_BENCHMARK_INTERNAL_H

#include "runtime/benchmark_hot.h"

img_result_t img_api_hot_bench_init(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_hot_bench_state_t *state);

img_result_t img_api_hot_bench_step(
    img_hot_bench_state_t *state);

void img_api_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state);

#endif /* IMGENGINE_API_BENCHMARK_INTERNAL_H */
