// ./include/pipeline/canvas_internal.h
#ifndef IMGENGINE_PIPELINE_CANVAS_INTERNAL_H
#define IMGENGINE_PIPELINE_CANVAS_INTERNAL_H

#include "pipeline/canvas.h"

void img_canvas_compute_geometry(img_canvas_t *canvas, const img_job_t *job, uint32_t pw,
                                 uint32_t ph);

img_result_t img_canvas_alloc_buffer(img_canvas_t *canvas, img_slab_pool_t *pool, uint32_t pw,
                                     uint32_t ph);

void img_canvas_fill_background(img_canvas_t *canvas, const img_job_t *job);

#endif /* IMGENGINE_PIPELINE_CANVAS_INTERNAL_H */
