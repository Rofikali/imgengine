// ./src/pipeline/layout_grid_place_cells.c
#define _GNU_SOURCE

#include "pipeline/layout_grid_internal.h"

#include <assert.h>

extern void img_blit_avx2(img_buffer_t *, const img_buffer_t *,
                          uint32_t, uint32_t);

uint32_t img_layout_grid_place_cells(
    img_canvas_t *canvas,
    const img_buffer_t *scaled,
    const img_job_t *job,
    img_cell_t *cells)
{
    uint32_t pw = canvas->photo_w_px;
    uint32_t ph = canvas->photo_h_px;
    uint32_t step_x = pw + job->gap;
    uint32_t step_y = ph + job->gap;
    uint32_t placed = 0;

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_UNDEFINED__)
    assert(((uintptr_t)cells & 0xF) == 0 && "cells not 16-byte aligned");
#endif

    for (uint32_t row = 0; row < job->rows; row++)
    {
        for (uint32_t col = 0; col < job->cols; col++)
        {
            size_t x = (size_t)canvas->start_x + (size_t)col * step_x;
            size_t y = (size_t)canvas->start_y + (size_t)row * step_y;

            if (x > UINT32_MAX || y > UINT32_MAX)
                continue;
            if (x > canvas->buf.width || pw > canvas->buf.width - (uint32_t)x)
                continue;
            if (y > canvas->buf.height || ph > canvas->buf.height - (uint32_t)y)
                continue;

            img_blit_avx2(&canvas->buf, scaled, (uint32_t)x, (uint32_t)y);

            size_t idx = (size_t)row * (size_t)job->cols + (size_t)col;
            /* sanitizer-only alignment check is done at function entry */
            cells[idx].x = x;
            cells[idx].y = y;
            cells[idx].w = pw;
            cells[idx].h = ph;
            placed++;
        }
    }

    return placed;
}
