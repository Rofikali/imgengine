// ./include/core/time.h
// include/core/time.h
//
// DO NOT define _GNU_SOURCE here.
// Feature test macros must be defined BEFORE any #include in the
// translation unit (.c file). Defining them in headers is too late
// if any system header has already been included transitively.
//
// Every .c file that includes this header must have:
//   #define _GNU_SOURCE    <- first line, before all includes
//
// This is the Linux kernel rule for POSIX/GNU extensions.

#ifndef IMGENGINE_TIME_H
#define IMGENGINE_TIME_H

#include <stdint.h>
#include <time.h>

/*
 * img_now_ns()
 *
 * Monotonic nanosecond clock -- never goes backward.
 * Uses CLOCK_MONOTONIC (vDSO-accelerated on Linux, ~20ns cost).
 *
 * Requires: _GNU_SOURCE defined before any include in the caller.
 * Used for: span timing, latency recording, log timestamps.
 * Not for: benchmarking tight loops -- use img_profiler_now() (RDTSC).
 */
static inline uint64_t img_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/*
 * img_now_ns_raw()
 *
 * Raw monotonic clock -- immune to NTP frequency corrections.
 * Most stable for benchmarking over long runs.
 * Falls back to CLOCK_MONOTONIC if CLOCK_MONOTONIC_RAW unavailable.
 */
static inline uint64_t img_now_ns_raw(void) {
    struct timespec ts;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#endif /* IMGENGINE_TIME_H */