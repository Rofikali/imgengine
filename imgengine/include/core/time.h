
// core/time.h 

#ifndef IMGENGINE_TIME_H
#define IMGENGINE_TIME_H

#include <stdint.h>
#include <time.h>

/*
 * 🔥 MONOTONIC CLOCK (ns)
 * kernel-style: single source of truth
 */
static inline uint64_t img_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

#endif