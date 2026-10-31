#ifndef IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H
#define IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H

#include <stdint.h>

#include "core/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMG_LAYOUT_NORMALIZED_CROP_SCALE UINT32_C(1000000)

typedef struct img_layout_normalized_crop {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} img_layout_normalized_crop_t;

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
int img_layout_centered_crop(uint32_t source_width, uint32_t source_height, uint32_t target_width,
                             uint32_t target_height, img_layout_crop_rect_t *output);

/*
 * Converts a normalized half-open crop rectangle to decoded source pixels.
 * Left/top edges use floor; right/bottom edges use ceil. Coordinates use
 * IMG_LAYOUT_NORMALIZED_CROP_SCALE units per full source dimension.
 * The resulting crop must match target aspect ratio within a one-source-pixel
 * rounding envelope (|w*th - h*tw| <= target_width + target_height).
 * Returns 1 on success; on failure a non-null output is cleared.
 */
int img_layout_normalized_crop_to_pixels(uint32_t source_width, uint32_t source_height,
                                         uint32_t target_width, uint32_t target_height,
                                         const img_layout_normalized_crop_t *normalized,
                                         img_layout_crop_rect_t *output);

/*
 * Builds a read-only-by-contract, non-owning view over a validated crop.
 * The view preserves the source stride and channels and has owner_pool == NULL.
 * Its data remains valid only while source remains alive and unchanged.
 *
 * Never pass this view to an ownership/release function or img_buf_hdr().
 * Returns 1 on success and clears output on failure.
 */
int img_layout_crop_borrowed_view(const img_buffer_t *source, const img_layout_crop_rect_t *crop,
                                  img_buffer_t *output);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_PIPELINE_LAYOUT_CROP_GEOMETRY_H */
