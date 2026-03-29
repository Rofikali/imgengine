// imgengine/border.h 

#ifndef IMG_BORDER_H
#define IMG_BORDER_H

#include "image.h"
#include "memory_pool.h"

int img_add_border(const img_t *src, img_t *dst,
                   mem_pool_t *mp,
                   int border);

#endif