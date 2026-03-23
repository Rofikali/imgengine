// imgengine/layout.h
#include "image.h"
#include "imgengine/context.h"

#ifndef IMG_LAYOUT_H
#define IMG_LAYOUT_H

int layout_grid(img_t *canvas,
                const img_t *photo,
                int cols, int rows,
                int gap, int padding,
                img_ctx_t *ctx);

#endif