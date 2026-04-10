// ./include/imgengine/resize.h


#ifndef IMG_RESIZE_H
#define IMG_RESIZE_H

#include "image.h"
#include "memory_pool.h"

int img_resize(const img_t *src, img_t *dst,
               mem_pool_t *mp,
               int new_w, int new_h);

#endif