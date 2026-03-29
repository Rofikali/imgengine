// src/plugins/pdf_plugin.c

#include "imgengine/image.h"
#include <stdio.h>

// VERY SIMPLE PDF (RGB, no compression)
int img_save_pdf(const char *filename, const img_t *img)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
        return 0;

    int w = img->width;
    int h = img->height;

    // PDF HEADER
    fprintf(f, "%%PDF-1.4\n");

    // OBJECT OFFSETS
    long offsets[10];
    int obj = 1;

    // 1: Catalog
    offsets[obj] = ftell(f);
    fprintf(f, "%d 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n", obj++);

    // 2: Pages
    offsets[obj] = ftell(f);
    fprintf(f, "%d 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n", obj++);

    // 3: Page
    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Type /Page /Parent 2 0 R "
            "/MediaBox [0 0 %d %d] "
            "/Contents 4 0 R "
            "/Resources << /XObject <</Im0 5 0 R>> >> >>\n"
            "endobj\n",
            obj++, w, h);

    // 4: Contents
    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Length 44 >>\n"
            "stream\n"
            "q\n"
            "%d 0 0 %d 0 0 cm\n"
            "/Im0 Do\n"
            "Q\n"
            "endstream\n"
            "endobj\n",
            obj++, w, h);

    // 5: Image
    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Type /XObject\n"
            "/Subtype /Image\n"
            "/Width %d\n"
            "/Height %d\n"
            "/ColorSpace /DeviceRGB\n"
            "/BitsPerComponent 8\n"
            "/Length %d >>\n"
            "stream\n",
            obj++, w, h, w * h * 3);

    fwrite(img->data, 1, w * h * 3, f);

    fprintf(f, "\nendstream\nendobj\n");

    int xref_offset = ftell(f);

    // XREF
    fprintf(f, "xref\n0 %d\n0000000000 65535 f \n", obj);

    for (int i = 1; i < obj; i++)
        fprintf(f, "%010ld 00000 n \n", offsets[i]);

    // TRAILER
    fprintf(f,
            "trailer\n<< /Size %d /Root 1 0 R >>\nstartxref\n%d\n%%%%EOF",
            obj, xref_offset);

    fclose(f);
    return 1;
}