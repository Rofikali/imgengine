// ./include/pipeline/layout.h

// Grid layout engine — places resized photos on canvas.

#ifndef IMGENGINE_PIPELINE_LAYOUT_H
#define IMGENGINE_PIPELINE_LAYOUT_H

#include <stdint.h>
#include "core/buffer.h"
#include "pipeline/canvas.h"
// #include "api/v1/img_error.h"
// #include "api/v1/img_job.h"
// #include "pipeline/job.h"
// #include "core/result.h"
#include "memory/arena.h"

/*
 * img_cell_t
 *
 * Position and size of one photo cell on the canvas.
 * All values in pixels.
 */
typedef struct
{
    uint32_t x, y; /* top-left corner */
    uint32_t w, h; /* width, height   */
} img_cell_t;

/*
 * img_layout_t
 *
 * Result of a grid layout pass.
 * Cells array is arena-allocated — valid for the duration of the job.
 */
typedef struct
{
    img_cell_t *cells;
    uint32_t count;
} img_layout_t;

/*
 * img_layout_grid()
 *
 * Resize photo to fit each cell, blit all cells onto canvas.
 * Populates layout->cells with each cell's position.
 *
 * photo:  decoded source image (slab-owned)
 * canvas: initialized A4 canvas
 * layout: output — arena-allocated cells array
 * arena:  scratch allocator for cells array
 */
img_result_t img_layout_grid(
    img_canvas_t *canvas,
    const img_buffer_t *photo,
    const img_job_t *job,
    img_layout_t *layout,
    img_arena_t *arena,
    img_slab_pool_t *pool);

#endif /* IMGENGINE_PIPELINE_LAYOUT_H */
