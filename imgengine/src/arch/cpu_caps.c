#define _GNU_SOURCE
#include "arch/cpu_caps.h"

#include <string.h>
#include <unistd.h>

#if defined(__x86_64__)
#include <cpuid.h>
#endif

cpu_caps_t img_cpu_detect_caps(void)
{
    cpu_caps_t caps;
    memset(&caps, 0, sizeof(caps));

    caps.logical_cores = sysconf(_SC_NPROCESSORS_ONLN);

#if defined(__x86_64__)
    unsigned int eax, ebx, ecx, edx;

    // SSE4.2
    __cpuid(1, eax, ebx, ecx, edx);
    caps.sse4 = (ecx & bit_SSE4_2) != 0;

    // AVX2
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    caps.avx2 = (ebx & bit_AVX2) != 0;

    // AVX512
#ifdef bit_AVX512F
    caps.avx512 = (ebx & bit_AVX512F) != 0;
#endif

#elif defined(__aarch64__)
    // Assume NEON on modern ARM64
    caps.neon = 1;
#endif

    return caps;
}