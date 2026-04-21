#include "runtime/benchmark_hot.h"
#include "runtime/job_exec.h"

#include "api/api_process_raw_internal.h"
#include "api/api_process_fast_internal.h"
#include "cold/validation.h"
#include "core/opcodes.h"
#include "hot/pipeline_exec.h"

#include <stdlib.h>
#include <string.h>

img_result_t img_runtime_hot_bench_init(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_hot_bench_state_t *state)
{
    if (!engine || !input || !state)
        return IMG_ERR_SECURITY;

    memset(state, 0, sizeof(*state));

    img_result_t r = decode_image_secure(engine, input, input_size, &state->decoded);
    if (r != IMG_SUCCESS)
        return r;

    state->image_bytes = (size_t)state->decoded.stride * state->decoded.height;
    if (state->image_bytes == 0)
    {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_FORMAT;
    }

    state->scratch_bytes = (state->image_bytes + 63u) & ~((size_t)63u);
    state->scratch = aligned_alloc(64, state->scratch_bytes);
    if (!state->scratch)
    {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_NOMEM;
    }

    state->pipe.count = 1;
    state->pipe.ops[0].op_code = OP_GRAYSCALE;
    state->pipe.ops[0].params = NULL;

    if (!img_validate_pipeline_safety(&state->pipe) ||
        img_pipeline_compile(&state->pipe, &state->compiled) != 0)
    {
        img_runtime_hot_bench_destroy(engine, state);
        return IMG_ERR_FORMAT;
    }

    img_api_make_ctx(engine, &state->ctx);
    return IMG_SUCCESS;
}

img_result_t img_runtime_hot_bench_step(
    img_hot_bench_state_t *state)
{
    if (!state || !state->scratch || !state->decoded.data || state->image_bytes == 0)
        return IMG_ERR_SECURITY;

    img_buffer_t work = state->decoded;
    memcpy(state->scratch, state->decoded.data, state->image_bytes);
    work.data = state->scratch;

    img_pipeline_execute_compiled_hot(&state->ctx, &state->compiled, &work);
    return IMG_SUCCESS;
}

void img_runtime_hot_bench_destroy(
    img_engine_t *engine,
    img_hot_bench_state_t *state)
{
    if (!state)
        return;

    free(state->scratch);
    state->scratch = NULL;

    if (engine && state->decoded.data)
        img_runtime_release_raw_buffer(engine, &state->decoded);

    memset(state, 0, sizeof(*state));
}
