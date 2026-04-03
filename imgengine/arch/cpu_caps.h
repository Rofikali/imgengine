/* include/arch/cpu_caps.h */

#ifndef IMGENGINE_CPU_CAPS_H
#define IMGENGINE_CPU_CAPS_H

#include <stdint.h>

typedef uint32_t cpu_caps_t;

// ================= BIT FLAGS =================
#define CPU_CAP_AVX2 (1 << 0)
#define CPU_CAP_AVX512 (1 << 1)
#define CPU_CAP_NEON (1 << 2)

// ================= DETECTION =================
cpu_caps_t img_cpu_detect_caps(void);

// ================= FAST CHECKS =================
static inline int img_cpu_has_avx2(cpu_caps_t caps)
{
    return (caps & CPU_CAP_AVX2) != 0;
}

static inline int img_cpu_has_avx512(cpu_caps_t caps)
{
    return (caps & CPU_CAP_AVX512) != 0;
}

static inline int img_cpu_has_neon(cpu_caps_t caps)
{
    return (caps & CPU_CAP_NEON) != 0;
}

#endif
