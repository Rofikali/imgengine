// ./src/pipeline/layout_grid_alloc_cells.c
#define _GNU_SOURCE

#include "pipeline/layout_grid_internal.h"

#include <assert.h>

#include <stddef.h>
#include <string.h>

img_result_t img_layout_grid_alloc_cells(const img_job_t *job, img_arena_t *arena,
                                         img_cell_t **cells_out) {
    size_t cols = (size_t)job->cols;
    size_t rows = (size_t)job->rows;
    if (rows > SIZE_MAX / cols)
        return IMG_ERR_NOMEM;

    size_t total = cols * rows;
    if (total == 0 || total > UINT32_MAX)
        return IMG_ERR_SECURITY;
    if (total > SIZE_MAX / sizeof(img_cell_t))
        return IMG_ERR_NOMEM;

    size_t cell_bytes = total * sizeof(img_cell_t);
    img_cell_t *cells = img_arena_alloc_aligned(arena, cell_bytes, 16);
    if (!cells)
        return IMG_ERR_NOMEM;

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_UNDEFINED__)
    assert((((uintptr_t)cells) & 0xF) == 0 && "cells not 16-byte aligned");
#endif

    memset(cells, 0, cell_bytes);
    *cells_out = cells;
    return IMG_SUCCESS;
}
