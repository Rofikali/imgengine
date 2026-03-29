// imgengine/crop.h 

#ifndef IMG_CROP_H
#define IMG_CROP_H

#include "image.h"
#include "memory_pool.h"

int img_center_crop(const img_t *src, img_t *dst,
                    mem_pool_t *mp,
                    int target_w, int target_h);

#endif