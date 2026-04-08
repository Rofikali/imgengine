/* observability/tracing.c */

#define _GNU_SOURCE

#include "observability/tracing.h"
#include "observability/tracing.h"
#include "observability/binlog_fast.h"
#include "core/time.h"

img_span_t img_trace_start(const char *name)
{
    img_span_t s;
    s.name = name;
    s.start = img_now_ns();
    s.end = 0;
    return s;
}

void img_trace_end(img_span_t *s)
{
    if (!s)
        return;

    s->end = img_now_ns();

    // future: push to lock-free ring buffer exporter
}

void img_trace_emit(const char *name,
                    uint64_t a0,
                    uint64_t a1,
                    uint64_t a2)
{
    (void)name;

    // 🔥 bridge to binlog (can later swap to eBPF)
    IMG_LOG(IMG_EVENT_BATCH, a0, a1, a2);
}