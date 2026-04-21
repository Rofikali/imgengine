#ifndef IMGENGINE_RUNTIME_BENCHMARK_H
#define IMGENGINE_RUNTIME_BENCHMARK_H

#include "api/api_internal.h"
#include "core/buffer.h"
#include "pipeline/pipeline_compiled.h"
#include "pipeline/pipeline_types.h"

typedef struct
{
    img_buffer_t decoded;
    img_pipeline_desc_t pipe;
    img_pipeline_compiled_t compiled;
    img_ctx_t ctx;
    uint8_t *scratch;
    size_t image_bytes;
    size_t scratch_bytes;
} img_hot_bench_state_t;

img_result_t img_runtime_hot_bench_init(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_hot_bench_state_t *state);

img_result_t img_runtime_hot_bench_step(
    img_hot_bench_state_t *state);

void img_runtime_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state);

#endif /* IMGENGINE_RUNTIME_BENCHMARK_H */
