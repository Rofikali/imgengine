
// ./src/pipeline/batch_exec.c

#include "pipeline/batch_exec.h"
#include "pipeline/pipeline_fused.h"
#include "observability/profiler.h"
#include "observability/logger.h"
#include "core/buffer.h"
#include <stddef.h>

#include "observability/binlog_fast.h"
#include "observability/tracepoints.h"

#include "api/v1/img_buffer_utils.h"

void img_batch_execute(
    img_ctx_t *ctx,
    img_batch_t *batch,
    void *pipeline)
{
    if (__builtin_expect(!batch || batch->count == 0, 0))
        return;

    /*
     * 🔥 Resolve fused kernel ONCE
     */
    img_fused_kernel_fn fn = img_get_fused_kernel();

    if (__builtin_expect(fn == NULL, 0))
        return;

    /*
     * 🔥 PROFILING START
     */
    uint64_t start = img_profiler_now();

    img_buffer_t **buffers;

    /*
     * 🔥 SINGLE CALL → FULL BATCH
     * (NO per-image loop — kernel handles it)
     */
    fn(ctx, batch, pipeline);

    /*
     * 🔥 PROFILING END
     */
    uint64_t end = img_profiler_now();
    uint64_t cycles = end - start;

    /*
     * 🔥 ZERO-OVERHEAD LOGGING
     */
    IMG_LOG_LATENCY(cycles, batch->count, 0);

    /*
     * 🔥 TRACEPOINT (perf/eBPF ready)
     */
    IMG_TRACE("batch_exec", cycles, batch->count, 0);
}
