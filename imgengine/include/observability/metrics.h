// ./include/observability/metrics.h

// observability/metrics.h

#ifndef IMGENGINE_METRICS_H
#define IMGENGINE_METRICS_H

#include <stdint.h>
#include <stddef.h>

#define IMG_MAX_CORES 64

typedef struct __attribute__((aligned(64)))
{
    uint64_t total_requests;
    uint64_t total_cycles;
    uint64_t max_latency;
    uint64_t min_latency;

} img_metrics_core_t;

typedef struct
{
    img_metrics_core_t cores[IMG_MAX_CORES];

    uint64_t backpressure_drops;

} img_metrics_t;

extern img_metrics_t g_metrics;

void img_metrics_init(void);

void img_metrics_inc(uint32_t core_id);
void img_metrics_latency(uint32_t core_id, uint64_t cycles);

void img_metrics_export_prometheus(char *buf, size_t size);

#endif