#ifndef IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H
#define IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct img_layout_crop_rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} img_layout_crop_rect_t;

/*
 * Computes the largest centered source rectangle matching the target aspect
 * ratio. Integer division floors the retained dimension; an odd excess pixel
 * is left on the right or bottom, matching the P11 Rust reference.
 *
 * Returns 1 on success. Returns 0 for null output, zero dimensions, or a
 * result that cannot retain at least one source pixel. On every failure a
 * non-null output is reset to all zeroes.
 */
int img_layout_centered_crop(uint32_t source_width, uint32_t source_height,
                             uint32_t target_width, uint32_t target_height,
                             img_layout_crop_rect_t *output);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H */
