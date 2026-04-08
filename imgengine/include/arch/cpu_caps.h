#ifndef IMGENGINE_CPU_CAPS_H
#define IMGENGINE_CPU_CAPS_H

#include <stdint.h>

typedef struct
{
    uint8_t avx2;
    uint8_t avx512;
    uint8_t sse4;
    uint8_t neon;

    uint32_t logical_cores;

} cpu_caps_t;

cpu_caps_t img_cpu_detect_caps(void);

#endif