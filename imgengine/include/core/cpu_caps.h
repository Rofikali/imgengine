// ./include/core/cpu_caps.h
#ifndef IMGENGINE_CORE_CPU_CAPS_H
#define IMGENGINE_CORE_CPU_CAPS_H

#include <stdint.h>

typedef uint32_t cpu_caps_t;

#define CPU_CAP_AVX2 (1u << 0)
#define CPU_CAP_AVX512 (1u << 1)
#define CPU_CAP_NEON (1u << 2)

cpu_caps_t img_cpu_detect_caps(void);

static inline int img_cpu_has_avx2(cpu_caps_t caps) { return (caps & CPU_CAP_AVX2) != 0; }

static inline int img_cpu_has_avx512(cpu_caps_t caps) { return (caps & CPU_CAP_AVX512) != 0; }

static inline int img_cpu_has_neon(cpu_caps_t caps) { return (caps & CPU_CAP_NEON) != 0; }

#endif /* IMGENGINE_CORE_CPU_CAPS_H */
