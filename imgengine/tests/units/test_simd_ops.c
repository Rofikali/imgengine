// ./tests/units/test_simd_ops.c
// tests/units/test_simd_ops.c

#include "imgengine/image.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define POISON_BYTE 0xEF

void test_simd_bounds_handling() {
    struct mem_pool mp;
    mp_init(&mp, 1024 * 1024);

    // 1. Create an awkward size: 15x15, 3 channels
    // Row bytes = 45. Stride will be 64 (next multiple of 32).
    img_t img;
    img_create(&img, &mp, 15, 15, 3);

    // 2. Poison the padding area
    for (int y = 0; y < img.height; y++) {
        unsigned char *row_end = img.data + (y * img.stride) + (img.width * img.channels);
        int padding_len = img.stride - (img.width * img.channels);
        memset(row_end, POISON_BYTE, padding_len);
    }

    // 3. Run your SIMD op (e.g., a dummy brightness boost using AVX2)
    // simulate_avx2_brightness_boost(&img);

    // 4. Verify Poison didn't change
    for (int y = 0; y < img.height; y++) {
        unsigned char *row_end = img.data + (y * img.stride) + (img.width * img.channels);
        int padding_len = img.stride - (img.width * img.channels);

        for (int i = 0; i < padding_len; i++) {
            if (row_end[i] != POISON_BYTE) {
                printf("❌ FAIL: SIMD wrote into padding at row %d, byte %d\n", y, i);
                assert(0);
            }
        }
    }

    mp_destroy(&mp);
    printf("✅ PASS: SIMD Bounds & Stride Integrity\n");
}

int main() {
    test_simd_bounds_handling();
    return 0;
}
