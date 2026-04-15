// ./src/pipeline/crop_marks.c
// src/pipeline/crop_marks.c
//
// FIX: Crop marks were drawing ON the photos because the mark
// coordinates placed them starting AT the photo edge going INWARD.
//
// Industry standard: crop marks are in the WHITE SPACE OUTSIDE
// the photo boundary. They indicate where to cut — never drawn
// over the image content.
//
// Correct geometry (per ISO 15930 / print industry standard):
//
//   Photo cell at (x, y, w, h)
//   crop_offset = distance from photo edge to start of mark
//   crop_mark   = length of the mark line
//
//   TOP LEFT corner:
//     Horizontal: from (x - offset - mark_len) to (x - offset)
//     Vertical:   from (y - offset - mark_len) to (y - offset)
//
//   TOP RIGHT corner:
//     Horizontal: from (x + w + offset) to (x + w + offset + mark_len)
//     Vertical:   from (y - offset - mark_len) to (y - offset)
//
//   All marks are OUTSIDE the photo. They draw in the gap/padding
//   white space. offset pushes them away from the photo edge.
//
// In your image, the red lines crossed through the photos because
// the draw_h/draw_v functions were starting at the photo edge
// (offset=0) or inside it (negative offset direction was wrong).

#define _GNU_SOURCE

#include "core/buffer.h"
#include "pipeline/layout.h"
#include "core/result.h"

static inline void set_px(img_buffer_t *c, int32_t x, int32_t y,
                          uint8_t r, uint8_t g, uint8_t b)
{
    if (x < 0 || y < 0 ||
        (uint32_t)x >= c->width ||
        (uint32_t)y >= c->height)
        return;

    uint8_t *p = c->data + (size_t)y * c->stride + (size_t)x * 3;
    p[0] = r;
    p[1] = g;
    p[2] = b;
}

/* Draw horizontal line from (x, y) rightward, length len, thickness th */
static void draw_h(img_buffer_t *c,
                   int32_t x, int32_t y, int32_t len, uint32_t th,
                   uint8_t r, uint8_t g, uint8_t b)
{
    for (uint32_t t = 0; t < th; t++)
        for (int32_t i = 0; i < len; i++)
            set_px(c, x + i, y + (int32_t)t, r, g, b);
}

/* Draw vertical line from (x, y) downward, length len, thickness th */
static void draw_v(img_buffer_t *c,
                   int32_t x, int32_t y, int32_t len, uint32_t th,
                   uint8_t r, uint8_t g, uint8_t b)
{
    for (uint32_t t = 0; t < th; t++)
        for (int32_t i = 0; i < len; i++)
            set_px(c, x + (int32_t)t, y + i, r, g, b);
}

/*
 * img_draw_crop_marks()
 *
 * Draws ISO-standard crop marks in the white space OUTSIDE each photo.
 *
 * Mark color: black (0, 0, 0).
 * Marks never overlap the photo content.
 *
 *  Diagram for TOP-LEFT corner of one photo cell:
 *
 *  ←mark_len→←offset→|←── photo ──
 *  ═══════════════════|             ← horizontal mark (above photo)
 *                     |
 *  ═══════════════════|             ← horizontal mark (gap between thickness)
 *         ↑
 *         vertical mark (left of photo)
 *         goes from (y - offset - mark_len) upward to (y - offset)
 */
img_result_t img_draw_crop_marks(
    img_buffer_t *canvas,
    const img_layout_t *layout,
    const img_job_t *job)
{
    if (!canvas || !layout || job->crop_mark_px == 0)
        return IMG_SUCCESS;

    const int32_t off = (int32_t)job->crop_offset_px;
    const int32_t len = (int32_t)job->crop_mark_px;
    const uint32_t th = job->crop_thickness ? job->crop_thickness : 1;

    /* black crop marks */
    const uint8_t r = 0, g = 0, b = 0;

    for (uint32_t i = 0; i < layout->count; i++)
    {
        const img_cell_t *c = &layout->cells[i];

        const int32_t x = (int32_t)c->x;
        const int32_t y = (int32_t)c->y;
        const int32_t x2 = x + (int32_t)c->w; /* right edge */
        const int32_t y2 = y + (int32_t)c->h; /* bottom edge */

        /*
         * TOP-LEFT corner
         *   H mark: leftward from (x - off), length len
         *   V mark: upward   from (y - off), length len
         */
        draw_h(canvas, x - off - len, y - off - (int32_t)th, len, th, r, g, b);
        draw_v(canvas, x - off - (int32_t)th, y - off - len, len, th, r, g, b);

        /*
         * TOP-RIGHT corner
         *   H mark: rightward from (x2 + off), length len
         *   V mark: upward    from (y  - off), length len
         */
        draw_h(canvas, x2 + off, y - off - (int32_t)th, len, th, r, g, b);
        draw_v(canvas, x2 + off, y - off - len, len, th, r, g, b);

        /*
         * BOTTOM-LEFT corner
         *   H mark: leftward from (x - off), length len
         *   V mark: downward from (y2 + off), length len
         */
        draw_h(canvas, x - off - len, y2 + off, len, th, r, g, b);
        draw_v(canvas, x - off - (int32_t)th, y2 + off, len, th, r, g, b);

        /*
         * BOTTOM-RIGHT corner
         *   H mark: rightward from (x2 + off), length len
         *   V mark: downward  from (y2 + off), length len
         */
        draw_h(canvas, x2 + off, y2 + off, len, th, r, g, b);
        draw_v(canvas, x2 + off, y2 + off, len, th, r, g, b);
    }

    return IMG_SUCCESS;
}
