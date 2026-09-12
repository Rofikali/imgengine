// ./src/arch/x86_64/cpu_detect.c

#include "arch/cpu_caps.h"
#include <stddef.h>

#if defined(IMGENGINE_PORTABLE_BASELINE)

cpu_caps_t img_cpu_detect_caps(void) { return 0; }

#elif defined(__x86_64__)

#include <cpuid.h>

static uint64_t img_xgetbv0(void) {
    unsigned int eax = 0;
    unsigned int edx = 0;

    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((uint64_t)edx << 32) | eax;
}

cpu_caps_t img_cpu_detect_caps(void) {
    cpu_caps_t caps = 0;

    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid_max(0, NULL) < 1)
        return caps;

    __cpuid(1, eax, ebx, ecx, edx);
    if ((ecx & bit_OSXSAVE) == 0 || (ecx & bit_AVX) == 0)
        return caps;

    const uint64_t xcr0 = img_xgetbv0();
    const uint64_t avx_state = (UINT64_C(1) << 1) | (UINT64_C(1) << 2);
    if ((xcr0 & avx_state) != avx_state)
        return caps;

    // Ensure CPUID leaf 7 is supported
    if (__get_cpuid_max(0, NULL) >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);

        // AVX2 (bit 5 of EBX)
        if (ebx & (1 << 5))
            caps |= CPU_CAP_AVX2;

        // AVX512F (bit 16 of EBX)
        const uint64_t avx512_state = avx_state | (UINT64_C(1) << 5) |
                                      (UINT64_C(1) << 6) | (UINT64_C(1) << 7);
        if ((ebx & bit_AVX512F) != 0 && (xcr0 & avx512_state) == avx512_state)
            caps |= CPU_CAP_AVX512;
    }

    return caps;
}

#elif defined(__aarch64__)

cpu_caps_t img_cpu_detect_caps(void) {
    // On ARM64, NEON is mandatory
    return CPU_CAP_NEON;
}

#else

cpu_caps_t img_cpu_detect_caps(void) {
    // Fallback: no SIMD guarantees
    return 0;
}

#endif
