// hot/pipeline_threaded.c

#define _GNU_SOURCE
#include "hot/pipeline_threaded.h"
#include "pipeline/pipeline_types.h"
#include "core/opcodes.h"
#include "pipeline/jump_table.h"
#include "core/context_internal.h"
#include <stdlib.h>

#include <stdint.h>

#include "hot/pipeline_threaded.h"
#include "pipeline/jump_table.h"

void img_pipeline_execute_threaded(
    img_ctx_t *ctx,
    const img_threaded_program_t *prog,
    img_buffer_t *buf)
{
    for (uint32_t i = 0; i < prog->count; i++)
    {
        uint32_t op = prog->opcodes[i];
        void *params = prog->params[i];

        img_kernel_fn fn = g_jump_table[op];

        if (fn)
            fn(ctx, buf, params);
    }
}

void img_pipeline_build_threaded(
    const img_pipeline_desc_t *in,
    img_threaded_program_t *out)
{
    out->count = in->count;

    out->opcodes = malloc(sizeof(uint32_t) * in->count);
    out->params = malloc(sizeof(void *) * in->count);

    for (uint32_t i = 0; i < in->count; i++)
    {
        out->opcodes[i] = in->ops[i].op_code;
        out->params[i] = in->ops[i].params;
    }
}
