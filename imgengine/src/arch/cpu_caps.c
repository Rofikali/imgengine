// // arch/cpu_caps.c

// #include "arch/cpu_caps.h"

// #if defined(__x86_64__) || defined(__i386__)

// #include <cpuid.h>

// // Check if OS supports AVX registers
// static int os_supports_avx(void)
// {
//     uint32_t eax, edx;

//     __asm__ volatile(
//         "xgetbv"
//         : "=a"(eax), "=d"(edx)
//         : "c"(0));

//     return (eax & 0x6) == 0x6; // XMM + YMM enabled
// }

// cpu_caps_t img_cpu_detect_caps(void)
// {
//     cpu_caps_t caps = 0;

//     unsigned int eax, ebx, ecx, edx;

//     // ---------------- BASIC FEATURES ----------------
//     __cpuid(1, eax, ebx, ecx, edx);

//     int avx_supported = (ecx & bit_AVX) != 0;

//     if (avx_supported && os_supports_avx())
//     {
//         // ---------------- EXTENDED FEATURES ----------------
//         __cpuid_count(7, 0, eax, ebx, ecx, edx);

//         if (ebx & bit_AVX2)
//             caps |= CPU_CAP_AVX2;

//         if (ebx & bit_AVX512F)
//             caps |= CPU_CAP_AVX512;
//     }

//     return caps;
// }

// #elif defined(__aarch64__)

// #include <sys/auxv.h>
// #include <asm/hwcap.h>

// cpu_caps_t img_cpu_detect_caps(void)
// {
//     cpu_caps_t caps = 0;

// #ifdef AT_HWCAP
//     unsigned long hwcaps = getauxval(AT_HWCAP);

// #ifdef HWCAP_ASIMD
//     if (hwcaps & HWCAP_ASIMD)
//         caps |= CPU_CAP_NEON;
// #endif

// #endif

//     return caps;
// }

// #else

// // Fallback (no SIMD)
// cpu_caps_t img_cpu_detect_caps(void)
// {
//     return 0;
// }

// #endif

#include "arch/cpu_caps.h"

cpu_caps_t img_detect_cpu(void)
{
    return img_cpu_detect_caps(); // 🔥 redirect
}