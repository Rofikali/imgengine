// ./include/pipeline/layout_grid_internal.h

// ================================================================
// FILE 2: include/pipeline/layout_grid_internal.h  (UPDATE)
// Add img_ctx_t* to grid prepare function.
// ================================================================

#ifndef IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H
#define IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H

#include "core/result.h"
#include "core/buffer.h"
#include "core/context_internal.h"
#include "pipeline/canvas.h"
#include "pipeline/layout.h"
#include "pipeline/job.h"
#include "pipeline/render_cache.h"
#include "pipeline/layout_resize_internal.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include <stdint.h>

img_result_t img_layout_grid_prepare_scaled(img_ctx_t *ctx, /* ← added */
                                            const img_buffer_t *photo, const img_job_t *job,
                                            uint32_t pw, uint32_t ph, img_slab_pool_t *pool,
                                            img_buffer_t *scaled, uint8_t *owned_out);

img_result_t img_layout_grid_alloc_cells(const img_job_t *job, img_arena_t *arena,
                                         img_cell_t **cells_out);

uint32_t img_layout_grid_place_cells(img_canvas_t *canvas, const img_buffer_t *scaled,
                                     const img_job_t *job, img_cell_t *cells);

#endif /* IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H */

// // ./include/pipeline/layout_grid_internal.h
// #ifndef IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H
// #define IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H

// #include "pipeline/layout.h"
// #include "pipeline/layout_resize.h"

// img_result_t img_layout_grid_prepare_scaled(
//     const img_buffer_t *photo,
//     const img_job_t *job,
//     uint32_t pw,
//     uint32_t ph,
//     img_slab_pool_t *pool,
//     img_buffer_t *scaled);

// img_result_t img_layout_grid_alloc_cells(
//     const img_job_t *job,
//     img_arena_t *arena,
//     img_cell_t **cells_out);

// uint32_t img_layout_grid_place_cells(
//     img_canvas_t *canvas,
//     const img_buffer_t *scaled,
//     const img_job_t *job,
//     img_cell_t *cells);

// #endif /* IMGENGINE_PIPELINE_LAYOUT_GRID_INTERNAL_H */
