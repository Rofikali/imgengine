// ./include/observability/tracepoints.h

// observability/tracepoints.h

#ifndef IMGENGINE_TRACEPOINTS_H
#define IMGENGINE_TRACEPOINTS_H

#include <stdint.h>

/*
 * 🔥 STATIC TRACEPOINT (ZERO OVERHEAD WHEN DISABLED)
 */
#ifdef IMG_TRACE_ENABLED

#define IMG_TRACE(name, a0, a1, a2) \
    img_trace_emit(name, a0, a1, a2)

#else

#define IMG_TRACE(name, a0, a1, a2) \
    do                              \
    {                               \
    } while (0)

#endif

void img_trace_emit(const char *name,
                    uint64_t a0,
                    uint64_t a1,
                    uint64_t a2);

#endif