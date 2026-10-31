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

static int expect_borrowed_view(void) {
    /* Three RGB rows, each with five padding bytes after the 4-pixel payload. */
    uint8_t pixels[3u * 17u];
    for (size_t i = 0; i < sizeof(pixels); i++)
        pixels[i] = (uint8_t)i;

    img_buffer_t source = {
        .data = pixels,
        .owner_pool = (img_slab_pool_t *)(uintptr_t)1u,
        .width = 4,
        .height = 3,
        .channels = 3,
        .stride = 17,
    };
    img_layout_crop_rect_t crop = {.x = 1, .y = 1, .width = 2, .height = 2};
    img_buffer_t view = {0};

    if (!img_layout_crop_borrowed_view(&source, &crop, &view)) {
        fprintf(stderr, "padded-stride crop view was rejected\n");
        return 0;
    }

    if (view.data != pixels + 20u || view.owner_pool != NULL || view.width != 2 ||
        view.height != 2 || view.channels != 3 || view.stride != 17) {
        fprintf(stderr,
                "crop view did not preserve pointer, non-ownership, dimensions, or stride\n");
        return 0;
    }

    /* A borrowed view must not change the source's metadata or pixel bytes. */
    if (source.data != pixels || source.owner_pool == NULL || pixels[20] != view.data[0]) {
        fprintf(stderr, "crop view modified or detached the source\n");
        return 0;
    }
    return 1;
}

static int expect_invalid_view(const char *name, const img_buffer_t *source,
                               const img_layout_crop_rect_t *crop) {
    img_buffer_t output = {.data = (uint8_t *)(uintptr_t)1u,
                           .owner_pool = (img_slab_pool_t *)(uintptr_t)1u,
                           .width = 1,
                           .height = 1,
                           .channels = 1,
                           .stride = 1};
    if (img_layout_crop_borrowed_view(source, crop, &output) || output.data != NULL ||
        output.owner_pool != NULL || output.width != 0 || output.height != 0 ||
        output.channels != 0 || output.stride != 0) {
        fprintf(stderr, "%s: invalid crop view accepted or output not cleared\n", name);
        return 0;
    }
    return 1;
}


static int expect_normalized_crop(const char *name, uint32_t sw, uint32_t sh, uint32_t tw,
                                  uint32_t th, img_layout_normalized_crop_t input,
                                  img_layout_crop_rect_t expected) {
    img_layout_crop_rect_t actual = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
    if (!img_layout_normalized_crop_to_pixels(sw, sh, tw, th, &input, &actual) ||
        actual.x != expected.x || actual.y != expected.y || actual.width != expected.width ||
        actual.height != expected.height) {
        fprintf(stderr, "%s: normalized crop got {%u,%u,%u,%u}, expected {%u,%u,%u,%u}\\n",
                name, actual.x, actual.y, actual.width, actual.height, expected.x, expected.y,
                expected.width, expected.height);
        return 0;
    }
    return 1;
}

static int expect_invalid_normalized_crop(const char *name, uint32_t sw, uint32_t sh,
                                          uint32_t tw, uint32_t th,
                                          const img_layout_normalized_crop_t *input) {
    img_layout_crop_rect_t actual = {1, 2, 3, 4};
    if (img_layout_normalized_crop_to_pixels(sw, sh, tw, th, input, &actual) ||
        actual.x != 0 || actual.y != 0 || actual.width != 0 || actual.height != 0) {
        fprintf(stderr, "%s: invalid normalized crop accepted or output not cleared\\n", name);
        return 0;
    }
    return 1;
}

static int test_normalized_crop(void) {
    int passed = 1;
    passed &= test_normalized_crop();
    const uint32_t s = IMG_LAYOUT_NORMALIZED_CROP_SCALE;
    passed &= expect_normalized_crop("center square", 400, 200, 1, 1,
                                     (img_layout_normalized_crop_t){s / 4, 0, s / 2, s},
                                     (img_layout_crop_rect_t){100, 0, 200, 200});
    passed &= expect_normalized_crop("right edge square", 400, 200, 1, 1,
                                     (img_layout_normalized_crop_t){s / 2, 0, s / 2, s},
                                     (img_layout_crop_rect_t){200, 0, 200, 200});
    passed &= expect_normalized_crop("full 2:1", 400, 200, 2, 1,
                                     (img_layout_normalized_crop_t){0, 0, s, s},
                                     (img_layout_crop_rect_t){0, 0, 400, 200});
    passed &= expect_invalid_normalized_crop(
        "aspect mismatch", 400, 200, 1, 1,
        &(img_layout_normalized_crop_t){0, 0, s, s});
    passed &= expect_invalid_normalized_crop(
        "out of bounds", 400, 200, 1, 1,
        &(img_layout_normalized_crop_t){s / 2, 0, s / 2 + 1, s});
    passed &= expect_invalid_normalized_crop(
        "zero extent", 400, 200, 1, 1,
        &(img_layout_normalized_crop_t){0, 0, 0, s});
    passed &= expect_invalid_normalized_crop("null normalized", 400, 200, 1, 1, NULL);
    passed &= expect_invalid_normalized_crop(
        "zero source", 0, 200, 1, 1,
        &(img_layout_normalized_crop_t){0, 0, s, s});
    return passed;
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

    uint8_t backing[3u * 17u];
    img_buffer_t source = {
        .data = backing, .owner_pool = NULL, .width = 4, .height = 3, .channels = 3, .stride = 17};
    img_layout_crop_rect_t valid_crop = {.x = 1, .y = 1, .width = 2, .height = 2};
    passed &= expect_borrowed_view();
    passed &= expect_invalid_view("null source", NULL, &valid_crop);
    passed &= expect_invalid_view(
        "null data",
        &(img_buffer_t){.data = NULL, .width = 4, .height = 3, .channels = 3, .stride = 17},
        &valid_crop);
    passed &= expect_invalid_view(
        "short stride",
        &(img_buffer_t){.data = backing, .width = 4, .height = 3, .channels = 3, .stride = 11},
        &valid_crop);
    passed &=
        expect_invalid_view("crop outside source", &source,
                            &(img_layout_crop_rect_t){.x = 3, .y = 1, .width = 2, .height = 1});
    passed &= expect_invalid_view(
        "zero crop", &source, &(img_layout_crop_rect_t){.x = 0, .y = 0, .width = 0, .height = 1});
    passed &= expect_invalid_view("null crop", &source, NULL);
    passed &= expect_invalid_view(
        "zero channels",
        &(img_buffer_t){.data = backing, .width = 4, .height = 3, .channels = 0, .stride = 17},
        &valid_crop);

    if (!passed)
        return 1;
    puts("[crop] centered and normalized crop geometry plus borrowed view passed");
    return 0;
}
