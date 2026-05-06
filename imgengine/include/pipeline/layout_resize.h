// ./include/pipeline/layout_resize.h
#ifndef IMGENGINE_PIPELINE_LAYOUT_RESIZE_H
#define IMGENGINE_PIPELINE_LAYOUT_RESIZE_H

#include <stdint.h>
#include "core/buffer.h"
#include "core/result.h"
#include "memory/slab.h"

img_result_t img_layout_resize_bilinear(const img_buffer_t *src, img_buffer_t *dst, uint32_t new_w,
                                        uint32_t new_h, img_slab_pool_t *pool);

img_result_t img_layout_prepare_fill(const img_buffer_t *src, img_buffer_t *dst, uint32_t cell_w,
                                     uint32_t cell_h, img_slab_pool_t *pool);

img_result_t img_layout_prepare_fit(const img_buffer_t *src, img_buffer_t *dst, uint32_t cell_w,
                                    uint32_t cell_h, img_slab_pool_t *pool, uint8_t bg_r,
                                    uint8_t bg_g, uint8_t bg_b);

#endif /* IMGENGINE_PIPELINE_LAYOUT_RESIZE_H */
