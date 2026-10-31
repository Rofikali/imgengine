#include "pipeline/layout_crop_geometry.h"

#include <stddef.h>
#include <string.h>

static int img_layout_crop_mul_size(size_t left, size_t right, size_t *output) {
    if (!output || (left != 0 && right > SIZE_MAX / left))
        return 0;
    *output = left * right;
    return 1;
}

static int img_layout_crop_add_size(size_t left, size_t right, size_t *output) {
    if (!output || right > SIZE_MAX - left)
        return 0;
    *output = left + right;
    return 1;
}

int img_layout_centered_crop(uint32_t source_width, uint32_t source_height, uint32_t target_width,
                             uint32_t target_height, img_layout_crop_rect_t *output) {
    if (!output)
        return 0;
    memset(output, 0, sizeof(*output));

    if (source_width == 0 || source_height == 0 || target_width == 0 || target_height == 0)
        return 0;

    /*
     * A uint32_t-by-uint32_t product fits in uint64_t. Keeping the comparison
     * and retained-dimension products in this width avoids floating-point
     * aspect-ratio decisions and matches Rust's checked integer reference.
     */
    const uint64_t source_cross = (uint64_t)source_width * (uint64_t)target_height;
    const uint64_t target_cross = (uint64_t)target_width * (uint64_t)source_height;

    uint32_t crop_width = source_width;
    uint32_t crop_height = source_height;

    if (source_cross > target_cross) {
        const uint64_t retained =
            ((uint64_t)source_height * (uint64_t)target_width) / (uint64_t)target_height;
        if (retained == 0 || retained > source_width)
            return 0;
        crop_width = (uint32_t)retained;
    } else if (source_cross < target_cross) {
        const uint64_t retained =
            ((uint64_t)source_width * (uint64_t)target_height) / (uint64_t)target_width;
        if (retained == 0 || retained > source_height)
            return 0;
        crop_height = (uint32_t)retained;
    }

    if (crop_width == 0 || crop_height == 0 || crop_width > source_width ||
        crop_height > source_height)
        return 0;

    output->x = (source_width - crop_width) / 2u;
    output->y = (source_height - crop_height) / 2u;
    output->width = crop_width;
    output->height = crop_height;
    return 1;
}

static uint64_t img_layout_crop_ceil_div_u64(uint64_t numerator, uint64_t denominator) {
    return numerator / denominator + (numerator % denominator != 0u);
}

int img_layout_normalized_crop_to_pixels(uint32_t source_width, uint32_t source_height,
                                         uint32_t target_width, uint32_t target_height,
                                         const img_layout_normalized_crop_t *normalized,
                                         img_layout_crop_rect_t *output) {
    if (!output)
        return 0;
    memset(output, 0, sizeof(*output));

    const uint64_t scale = IMG_LAYOUT_NORMALIZED_CROP_SCALE;
    if (!normalized || source_width == 0 || source_height == 0 || target_width == 0 ||
        target_height == 0 || normalized->width == 0 || normalized->height == 0 ||
        normalized->x > scale || normalized->y > scale || normalized->width > scale ||
        normalized->height > scale)
        return 0;

    const uint64_t right = (uint64_t)normalized->x + normalized->width;
    const uint64_t bottom = (uint64_t)normalized->y + normalized->height;
    if (right > scale || bottom > scale)
        return 0;

    /* All products are bounded by UINT32_MAX * 1,000,000 and fit uint64_t. */
    const uint64_t left_px = ((uint64_t)normalized->x * source_width) / scale;
    const uint64_t top_px = ((uint64_t)normalized->y * source_height) / scale;
    uint64_t right_px = img_layout_crop_ceil_div_u64(right * source_width, scale);
    uint64_t bottom_px = img_layout_crop_ceil_div_u64(bottom * source_height, scale);

    if (right_px > source_width)
        right_px = source_width; /* only the normalized right-edge rounding */
    if (bottom_px > source_height)
        bottom_px = source_height;

    if (left_px >= right_px || top_px >= bottom_px || right_px > source_width ||
        bottom_px > source_height)
        return 0;

    const uint64_t crop_width = right_px - left_px;
    const uint64_t crop_height = bottom_px - top_px;
    const uint64_t width_cross = crop_width * target_height;
    const uint64_t height_cross = crop_height * target_width;
    const uint64_t difference =
        width_cross > height_cross ? width_cross - height_cross : height_cross - width_cross;
    const uint64_t rounding_envelope = (uint64_t)target_width + target_height;
    if (difference > rounding_envelope)
        return 0;

    output->x = (uint32_t)left_px;
    output->y = (uint32_t)top_px;
    output->width = (uint32_t)crop_width;
    output->height = (uint32_t)crop_height;
    return 1;
}

int img_layout_crop_borrowed_view(const img_buffer_t *source, const img_layout_crop_rect_t *crop,
                                  img_buffer_t *output) {
    if (!output)
        return 0;
    memset(output, 0, sizeof(*output));

    if (!source || !source->data || !crop || source->width == 0 || source->height == 0 ||
        source->channels == 0 || crop->width == 0 || crop->height == 0 || crop->x > source->width ||
        crop->y > source->height || crop->width > source->width - crop->x ||
        crop->height > source->height - crop->y)
        return 0;

    size_t source_row_bytes = 0;
    size_t source_prefix_bytes = 0;
    size_t source_span_bytes = 0;
    size_t x_offset = 0;
    size_t y_offset = 0;
    size_t data_offset = 0;
    size_t crop_row_bytes = 0;
    size_t crop_prefix_bytes = 0;
    size_t crop_span_bytes = 0;

    if (!img_layout_crop_mul_size((size_t)source->width, (size_t)source->channels,
                                  &source_row_bytes) ||
        (size_t)source->stride < source_row_bytes ||
        !img_layout_crop_mul_size((size_t)(source->height - 1u), (size_t)source->stride,
                                  &source_prefix_bytes) ||
        !img_layout_crop_add_size(source_prefix_bytes, source_row_bytes, &source_span_bytes) ||
        !img_layout_crop_mul_size((size_t)crop->x, (size_t)source->channels, &x_offset) ||
        !img_layout_crop_mul_size((size_t)crop->y, (size_t)source->stride, &y_offset) ||
        !img_layout_crop_add_size(y_offset, x_offset, &data_offset) ||
        !img_layout_crop_mul_size((size_t)crop->width, (size_t)source->channels, &crop_row_bytes) ||
        !img_layout_crop_mul_size((size_t)(crop->height - 1u), (size_t)source->stride,
                                  &crop_prefix_bytes) ||
        !img_layout_crop_add_size(crop_prefix_bytes, crop_row_bytes, &crop_span_bytes) ||
        data_offset > source_span_bytes || crop_span_bytes > source_span_bytes - data_offset)
        return 0;

    output->data = source->data + data_offset;
    output->owner_pool = NULL;
    output->width = crop->width;
    output->height = crop->height;
    output->channels = source->channels;
    output->stride = source->stride;
    return 1;
}
