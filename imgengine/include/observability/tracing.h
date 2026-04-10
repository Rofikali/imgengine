
/* observability/tracing.h */

#ifndef IMGENGINE_TRACING_H
#define IMGENGINE_TRACING_H

#include <stdint.h>

typedef struct
{
    const char *name;
    uint64_t start;
    uint64_t end;

} img_span_t;

img_span_t img_trace_start(const char *name);
void img_trace_end(img_span_t *span);

#endif