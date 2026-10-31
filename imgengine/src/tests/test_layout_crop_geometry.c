#include "pipeline/layout_crop_geometry.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int expect_crop(const char *name, uint32_t sw, uint32_t sh, uint32_t tw, uint32_t th,
                       img_layout_crop_rect_t expected) {
    img_layout_crop_rect_t actual = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
    if (!img_layout_centered_crop(sw, sh, tw, th, &actual) || actual.x != expected.x ||
        actual.y != expected.y || actual.width != expected.width ||
        actual.height != expected.height) {
        fprintf(stderr, "%s: got {%u,%u,%u,%u}, expected {%u,%u,%u,%u}\n", name, actual.x, actual.y,
                actual.width, actual.height, expected.x, expected.y, expected.width,
                expected.height);
        return 0;
    }
    return 1;
}

static int expect_invalid(const char *name, uint32_t sw, uint32_t sh, uint32_t tw, uint32_t th) {
    img_layout_crop_rect_t actual = {1, 2, 3, 4};
    if (img_layout_centered_crop(sw, sh, tw, th, &actual) || actual.x != 0 || actual.y != 0 ||
        actual.width != 0 || actual.height != 0) {
        fprintf(stderr, "%s: invalid geometry was accepted or output was not cleared\n", name);
        return 0;
    }
    return 1;
}

int main(void) {
    int passed = 1;

    /* Same-aspect and square sources retain every source pixel. */
    passed &= expect_crop("equal aspect", 400, 200, 2, 1, (img_layout_crop_rect_t){0, 0, 400, 200});
    passed &= expect_crop("square", 100, 100, 1, 1, (img_layout_crop_rect_t){0, 0, 100, 100});

    /* Wide source into tall cell and tall source into wide cell. */
    passed &=
        expect_crop("wide to square", 400, 200, 1, 1, (img_layout_crop_rect_t){100, 0, 200, 200});
    passed &=
        expect_crop("tall to square", 200, 400, 1, 1, (img_layout_crop_rect_t){0, 100, 200, 200});

    /* Odd excess is deterministically left on the right/bottom. */
    passed &= expect_crop("odd horizontal excess", 101, 100, 1, 1,
                          (img_layout_crop_rect_t){0, 0, 100, 100});
    passed &= expect_crop("odd vertical excess", 100, 101, 1, 1,
                          (img_layout_crop_rect_t){0, 0, 100, 100});

    /* One-pixel and large cross-product cases exercise integer boundaries. */
    passed &= expect_crop("one pixel", 1, 1, 9, 9, (img_layout_crop_rect_t){0, 0, 1, 1});
    passed &= expect_crop("large dimensions", UINT32_MAX, UINT32_MAX, 1, 1,
                          (img_layout_crop_rect_t){0, 0, UINT32_MAX, UINT32_MAX});

    passed &= expect_invalid("zero source width", 0, 10, 1, 1);
    passed &= expect_invalid("zero source height", 10, 0, 1, 1);
    passed &= expect_invalid("zero target width", 10, 10, 0, 1);
    passed &= expect_invalid("zero target height", 10, 10, 1, 0);

    if (!img_layout_centered_crop(10, 10, 1, 1, NULL)) {
        puts("[crop] null output rejected");
    } else {
        fprintf(stderr, "null output was accepted\n");
        passed = 0;
    }

    if (!passed)
        return 1;
    puts("[crop] native centered crop geometry passed");
    return 0;
}
