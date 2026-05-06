// ./include/pipeline/trace.h

// include/pipeline/trace.h  (NEW FILE)
// Observability hooks for the pipeline hot path.
// Compiles to zero overhead when IMGENGINE_TRACE=OFF.

#ifndef IMGENGINE_PIPELINE_TRACE_H
#define IMGENGINE_PIPELINE_TRACE_H

#include <stdint.h>

/*
 * Pipeline tracing macros.
 *
 * These replace direct includes of observability/ headers in pipeline/.
 * When IMGENGINE_TRACE is not defined: all macros expand to nothing.
 * When IMGENGINE_TRACE is defined: hooks into the observability layer.
 *
 * This breaks the pipeline/ → observability/ dependency:
 *   pipeline/ includes pipeline/trace.h  (same layer — no violation)
 *   observability/ implements the hooks   (registered at startup)
 *
 * Linux kernel analogy: TRACE_EVENT() macros in kernel/trace/
 */

#ifdef IMGENGINE_TRACE

#include "observability/profiler.h"
#include "observability/binlog_fast.h"
#include "observability/tracepoints.h"
#include "observability/logger.h"

#define PIPELINE_PROFILER_NOW() img_profiler_now()
#define PIPELINE_LOG_LATENCY(cyc, cnt, ext) IMG_LOG_LATENCY(cyc, cnt, ext)
#define PIPELINE_TRACE(name, a0, a1, a2) IMG_TRACE(name, a0, a1, a2)

#else /* IMGENGINE_TRACE not defined */

#define PIPELINE_PROFILER_NOW() ((uint64_t)0)
#define PIPELINE_LOG_LATENCY(cyc, cnt, ext) ((void)0)
#define PIPELINE_TRACE(name, a0, a1, a2) ((void)0)

#endif /* IMGENGINE_TRACE */

#endif /* IMGENGINE_PIPELINE_TRACE_H */
