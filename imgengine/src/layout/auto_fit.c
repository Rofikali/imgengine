// src/layout/auto_fit.c
#include "imgengine/layout.h"

void layout_autofit(int canvas_w, int canvas_h,
                    int pw, int ph,
                    int gap, int padding,
                    int *out_cols, int *out_rows)
{
    int usable_w = canvas_w - 2 * padding;
    int usable_h = canvas_h - 2 * padding;

    int cols = (usable_w + gap) / (pw + gap);
    int rows = (usable_h + gap) / (ph + gap);

    if (cols < 1)
        cols = 1;
    if (rows < 1)
        rows = 1;

    *out_cols = cols;
    *out_rows = rows;
}