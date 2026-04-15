// ./src/pipeline/border.c
// src/pipeline/border.c

// ================================================================
// FILE 2: src/pipeline/border.c  (NEW FILE)
// Draw a solid black border inside the photo boundary.
// border_px pixels thick on all 4 sides, drawn INSIDE the photo
// so it doesn't affect the grid geometry.
// ================================================================

#define _GNU_SOURCE

#include "core/buffer.h"
#include "pipeline/layout.h"
#include "core/result.h"

static inline void fill_rect(
    img_buffer_t *canvas,
    int32_t x, int32_t y,
    int32_t w, int32_t h,
    uint8_t r, uint8_t g, uint8_t b)
{
    const int32_t cw = (int32_t)canvas->width;
    const int32_t cht = (int32_t)canvas->height;

    for (int32_t py = y; py < y + h; py++)
    {
        if (py < 0 || py >= cht)
            continue;
        for (int32_t px = x; px < x + w; px++)
        {
            if (px < 0 || px >= cw)
                continue;
            uint8_t *p = canvas->data +
                         (size_t)py * canvas->stride +
                         (size_t)px * 3;
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
    }
}

/*
 * img_draw_borders()
 *
 * Draws a solid border inside each photo cell.
 * Border is drawn INSIDE the photo boundary — does not affect
 * grid geometry, gap, bleed, or crop marks.
 *
 * Border color: black (0, 0, 0).
 * Thickness: job->border_px pixels on each side.
 *
 * Standard photo lab border — thin black line around each print.
 */
img_result_t img_draw_borders(
    img_buffer_t *canvas,
    const img_layout_t *layout,
    uint32_t border_px)
{
    if (!canvas || !layout || border_px == 0)
        return IMG_SUCCESS;

    const uint8_t r = 0, g = 0, b = 0; /* black */
    const int32_t bp = (int32_t)border_px;

    for (uint32_t i = 0; i < layout->count; i++)
    {
        const img_cell_t *c = &layout->cells[i];

        const int32_t x = (int32_t)c->x;
        const int32_t y = (int32_t)c->y;
        const int32_t w = (int32_t)c->w;
        const int32_t h = (int32_t)c->h;

        /* TOP edge */
        fill_rect(canvas, x, y, w, bp, r, g, b);

        /* BOTTOM edge */
        fill_rect(canvas, x, y + h - bp, w, bp, r, g, b);

        /* LEFT edge (full height including corners) */
        fill_rect(canvas, x, y, bp, h, r, g, b);

        /* RIGHT edge (full height including corners) */
        fill_rect(canvas, x + w - bp, y, bp, h, r, g, b);
    }

    return IMG_SUCCESS;
}
