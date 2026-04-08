// hot/pipeline_threaded.h

#ifndef IMGENGINE_PIPELINE_THREADED_H
#define IMGENGINE_PIPELINE_THREADED_H

#include "core/context_internal.h"
#include "pipeline/pipeline_types.h"
#include "core/buffer.h"

/*
 * 🔥 DIRECT-THREADED PROGRAM (NO LABEL STORAGE)
 */
typedef struct
{
    uint32_t *opcodes;
    void **params;
    uint32_t count;

} img_threaded_program_t;

/*
 * 🔥 BUILD
 */
void img_pipeline_build_threaded(
    const img_pipeline_desc_t *in,
    img_threaded_program_t *out);

/*
 * 🔥 EXECUTE (DIRECT THREADED)
 */
void img_pipeline_execute_threaded(
    img_ctx_t *ctx,
    const img_threaded_program_t *prog,
    img_buffer_t *buf);

#endif
