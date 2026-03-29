// src/layout/crop_layout.c 

#include "imgengine/image.h"

static void draw_line(img_t *img,
                      int x1, int y1,
                      int x2, int y2,
                      int thickness)
{
    for (int t = 0; t < thickness; t++)
    {
        for (int x = x1; x <= x2; x++)
        {
            int idx = ((y1 + t) * img->width + x) * 3;
            img->data[idx + 0] = 0;
            img->data[idx + 1] = 0;
            img->data[idx + 2] = 0;
        }

        for (int y = y1; y <= y2; y++)
        {
            int idx = (y * img->width + (x1 + t)) * 3;
            img->data[idx + 0] = 0;
            img->data[idx + 1] = 0;
            img->data[idx + 2] = 0;
        }
    }
}

void draw_crop_marks(img_t *canvas,
                     int x, int y,
                     int w, int h,
                     int mark_len,
                     int thickness)
{
    // Top-left
    draw_line(canvas, x - mark_len, y, x, y, thickness);
    draw_line(canvas, x, y - mark_len, x, y, thickness);

    // Top-right
    draw_line(canvas, x + w, y - mark_len, x + w, y, thickness);
    draw_line(canvas, x + w, y, x + w + mark_len, y, thickness);

    // Bottom-left
    draw_line(canvas, x - mark_len, y + h, x, y + h, thickness);
    draw_line(canvas, x, y + h, x, y + h + mark_len, thickness);

    // Bottom-right
    draw_line(canvas, x + w, y + h, x + w + mark_len, y + h, thickness);
    draw_line(canvas, x + w, y + h, x + w, y + h + mark_len, thickness);
}