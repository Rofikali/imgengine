#include "observability/tracing/tracing.h"
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
    if (!s) return;
    s->end = img_now_ns();
}