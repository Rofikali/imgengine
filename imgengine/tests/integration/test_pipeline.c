// ./tests/integration/test_pipeline.c
// tests/integration/test_pipeline.py

#include "imgengine/api.h"
#include "imgengine/crop.h"
#include "imgengine/resize.h"
#include <assert.h>
#include <stdio.h>

/**
 * test_full_chain - Manual walk-through of the processing pipeline
 */
void test_full_chain(void) {
    struct mem_pool mp;
    mp_init(&mp, 10 * 1024 * 1024); // 10MB Pool

    img_t src, cropped, resized;

    /* 1. Create Dummy Source (1000x1000) */
    img_create(&src, &mp, 1000, 1000, 3);

    /* 2. Center Crop to 500x500 */
    int ok = img_center_crop(&src, &cropped, &mp, 500, 500);
    assert(ok && cropped.width == 500);
    /* Verify Kernel-style stride: 500*3=1500, next 32-byte aligned is 1504 */
    assert(cropped.stride == 1504);

    /* 3. Resize to Passport Size (e.g. 413x531) */
    ok = img_resize(&cropped, &resized, &mp, 413, 531);
    assert(ok && resized.height == 531);

    /* 4. Validate Memory Safety */
    // Ensure resized data pointer is within pool bounds
    assert(resized.data >= mp.buffer && resized.data < mp.buffer + mp.size);

    mp_destroy(&mp);
    printf(" [PASS] Integration: Crop -> Resize Chain\n");
}

int main(void) {
    test_full_chain();
    return 0;
}
