// ./include/observability/profiler.h

// observability/metrics.h

#ifndef IMGENGINE_PROFILER_H
#define IMGENGINE_PROFILER_H

#include <stdint.h>

static inline uint64_t img_profiler_now(void) {
#if defined(__x86_64__)
    return __builtin_ia32_rdtsc();
#else
    return 0;
#endif
}

#endif