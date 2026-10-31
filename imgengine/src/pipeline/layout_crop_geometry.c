#include "pipeline/layout_crop_geometry.h"

#include <stddef.h>
#include <string.h>

int img_layout_centered_crop(uint32_t source_width, uint32_t source_height,
                             uint32_t target_width, uint32_t target_height,
                             img_layout_crop_rect_t *output) {
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
