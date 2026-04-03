#include "arch/cpu_caps.h"
#include <stddef.h>

#if defined(__x86_64__)
#include <cpuid.h>

cpu_arch_t img_detect_cpu(void)
{
    unsigned int eax, ebx, ecx, edx;

    // Check AVX2
    if (__get_cpuid_max(0, NULL) >= 7)
    {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);

        if (ebx & (1 << 5)) // AVX2
            return ARCH_AVX2;

        if (ebx & (1 << 16)) // AVX512F
            return ARCH_AVX512;
    }

    return ARCH_SCALAR;
}
#else

cpu_arch_t img_detect_cpu(void)
{
#if defined(__aarch64__)
    return ARCH_NEON;
#else
    return ARCH_SCALAR;
#endif
}

#endif