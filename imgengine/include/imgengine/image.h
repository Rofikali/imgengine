// ./include/imgengine/image.h

#ifndef IMG_IMAGE_H
#define IMG_IMAGE_H

#include "memory_pool.h"

typedef struct {
    int width;
    int height;
    int channels;
    // int stride;          /* Number of bytes per row, 32-byte aligned */
    unsigned char *data; /* 32-byte aligned for AVX2 */
} img_t;

/* Returns 1 on success, 0 on failure */
int img_create(img_t *img, mem_pool_t *mp, int w, int h, int ch);
int img_load_to_pool(const char *path, img_t *img, mem_pool_t *mp);

#endif
