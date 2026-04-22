#ifndef IMGENGINE_RUNTIME_BENCHMARK_H
#define IMGENGINE_RUNTIME_BENCHMARK_H

#include <stddef.h>

#include "core/context_internal.h"
#include "core/buffer.h"
#include "core/result.h"
#include "memory/arena.h"
#include "pipeline/canvas.h"
#include "pipeline/render_cache.h"
#include "pipeline/job.h"
#include "pipeline/layout.h"

typedef struct
{
    img_engine_t *engine;
    img_buffer_t decoded;
    img_job_t job;
    img_ctx_t ctx;
    img_canvas_t canvas;
    img_layout_t layout;
    img_render_cache_t render_cache;
    img_arena_t *arena;
} img_hot_bench_state_t;

img_result_t img_runtime_hot_bench_init(
    img_engine_t *engine,
    const img_buffer_t *decoded,
    img_job_template_t preset_template,
    img_hot_bench_state_t *state);

img_result_t img_runtime_hot_bench_step(
    img_hot_bench_state_t *state);

void img_runtime_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state);

#endif /* IMGENGINE_RUNTIME_BENCHMARK_H */
