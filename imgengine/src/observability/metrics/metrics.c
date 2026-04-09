#include "observability/metrics/metrics.h"
#include <stdio.h>

img_metrics_t g_metrics;

void img_metrics_init(void)
{
    for (int i = 0; i < IMG_MAX_CORES; i++)
        g_metrics.cores[i].min = UINT64_MAX;
}

void img_metrics_record(uint32_t core, uint64_t cycles)
{
    img_metrics_core_t *c = &g_metrics.cores[core];

    __atomic_fetch_add(&c->req, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&c->cycles, cycles, __ATOMIC_RELAXED);

    if (cycles > c->max) c->max = cycles;
    if (cycles < c->min) c->min = cycles;
}

void img_metrics_export_prometheus(char *buf, uint64_t size)
{
    uint64_t req = 0, cycles = 0, max = 0;

    for (int i = 0; i < IMG_MAX_CORES; i++)
    {
        req += g_metrics.cores[i].req;
        cycles += g_metrics.cores[i].cycles;

        if (g_metrics.cores[i].max > max)
            max = g_metrics.cores[i].max;
    }

    uint64_t avg = req ? cycles / req : 0;

    snprintf(buf, size,
        "img_requests_total %lu\n"
        "img_latency_avg %lu\n"
        "img_latency_max %lu\n",
        req, avg, max);
}