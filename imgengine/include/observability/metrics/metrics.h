#ifndef IMGENGINE_METRICS_H
#define IMGENGINE_METRICS_H

#include <stdint.h>

#define IMG_MAX_CORES 64

typedef struct
{
    uint64_t req;
    uint64_t cycles;
    uint64_t max;
    uint64_t min;

} img_metrics_core_t;

typedef struct
{
    img_metrics_core_t cores[IMG_MAX_CORES];

    uint64_t drops;

} img_metrics_t;

extern img_metrics_t g_metrics;

void img_metrics_init(void);
void img_metrics_record(uint32_t core, uint64_t cycles);

void img_metrics_export_prometheus(char *buf, uint64_t size);

#endif