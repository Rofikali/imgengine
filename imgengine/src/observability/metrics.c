
// ./src/observability/metrics.c

#include "observability/metrics.h"
#include <stdio.h>

img_metrics_t g_metrics;

void img_metrics_init(void)
{
    for (int i = 0; i < IMG_MAX_CORES; i++)
    {
        g_metrics.cores[i].min_latency = UINT64_MAX;
    }
}

void img_metrics_inc(uint32_t core_id)
{
    __atomic_fetch_add(
        &g_metrics.cores[core_id].total_requests,
        1,
        __ATOMIC_RELAXED);
}

void img_metrics_latency(uint32_t core_id, uint64_t cycles)
{
    img_metrics_core_t *c = &g_metrics.cores[core_id];

    __atomic_fetch_add(&c->total_cycles, cycles, __ATOMIC_RELAXED);

    if (cycles > c->max_latency)
        c->max_latency = cycles;

    if (cycles < c->min_latency)
        c->min_latency = cycles;
}

void img_metrics_export_prometheus(char *buf, size_t size)
{
    uint64_t total_req = 0;
    uint64_t total_cycles = 0;
    uint64_t max = 0;

    for (int i = 0; i < IMG_MAX_CORES; i++)
    {
        total_req += g_metrics.cores[i].total_requests;
        total_cycles += g_metrics.cores[i].total_cycles;

        if (g_metrics.cores[i].max_latency > max)
            max = g_metrics.cores[i].max_latency;
    }

    uint64_t avg = total_req ? total_cycles / total_req : 0;

    snprintf(buf, size,
             "img_requests_total %lu\n"
             "img_latency_avg %lu\n"
             "img_latency_max %lu\n",
             total_req, avg, max);
}