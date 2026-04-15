// ./include/pipeline/canvas.h

// Canvas = the A4 output buffer. Allocated from slab.

#ifndef IMGENGINE_PIPELINE_CANVAS_H
#define IMGENGINE_PIPELINE_CANVAS_H

#include <stdint.h>
#include "core/buffer.h"
#include "memory/slab.h"
#include "pipeline/job.h"
#include "core/result.h"

/*
 * img_canvas_t
 *
 * The output A4 canvas.
 * Wraps img_buffer_t with layout metadata.
 * All measurements in pixels at job->dpi.
 */
typedef struct
{
    img_buffer_t buf; /* raw pixel buffer (slab-owned) */

    uint32_t page_w_px; /* A4 width  at DPI (2480 @ 300dpi) */
    uint32_t page_h_px; /* A4 height at DPI (3508 @ 300dpi) */

    /* grid geometry (computed by img_canvas_init) */
    uint32_t photo_w_px; /* each photo cell width  */
    uint32_t photo_h_px; /* each photo cell height */
    uint32_t start_x;    /* grid origin x */
    uint32_t start_y;    /* grid origin y */

} img_canvas_t;

/*
 * img_canvas_init()
 *
 * Allocate canvas from pool, fill with background color,
 * compute grid geometry from job parameters.
 *
 * Returns IMG_SUCCESS or IMG_ERR_NOMEM.
 */
img_result_t img_canvas_init(
    img_canvas_t *canvas,
    img_slab_pool_t *pool,
    const img_job_t *job);

/*
 * img_canvas_free()
 *
 * Return canvas buffer to pool.
 */
void img_canvas_free(img_canvas_t *canvas, img_slab_pool_t *pool);

/*
 * CM to pixels conversion
 */
static inline uint32_t img_cm_to_px(float cm, uint32_t dpi)
{
    return (uint32_t)(cm * (float)dpi / 2.54f + 0.5f);
}

/* A4 dimensions in cm */
#define IMG_A4_W_CM 21.0f
#define IMG_A4_H_CM 29.7f

#endif /* IMGENGINE_PIPELINE_CANVAS_H */
