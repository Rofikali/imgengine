// imgengine/layout.h

#ifndef IMG_LAYOUT_H
#define IMG_LAYOUT_H

#include "common.h" // <--- ADD THIS
#include "image.h"

/* Forward declare the EXACT typedefs used in api.h and context.h */
typedef struct img_ctx img_ctx_t;
typedef struct img_job img_job_t;

typedef struct img_cell {
    int x, y;
    int w, h;
} img_cell_t;

typedef struct img_layout_info {
    img_cell_t *cells;
    int count;
} img_layout_info_t;

/* 6-Year Engineer: ABI-compliant signatures */
void layout_store(img_ctx_t *ctx, img_cell_t *cells, int count);

IE_API img_layout_info_t *layout_get(img_ctx_t *ctx);

// img_layout_info_t *layout_get(img_ctx_t *ctx);

int layout_grid(img_t *canvas, const img_t *photo, const img_job_t *job, img_ctx_t *ctx);

#endif
