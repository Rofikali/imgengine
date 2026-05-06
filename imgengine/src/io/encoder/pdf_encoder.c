// ./src/io/encoder/pdf_encoder.c
// src/io/encoder/pdf_encoder.c

// PDF encoder — JPEG-compressed image inside a PDF 1.4 container.
// Ported from pdf_plugin.c, upgraded: JPEG-compressed stream.

#include "core/buffer.h"
#include "core/result.h"
#include <turbojpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * img_encode_pdf()
 *
 * Encode img_buffer_t into a PDF 1.4 file with JPEG-compressed image.
 *
 * PDF structure:
 *   Object 1: Catalog
 *   Object 2: Pages
 *   Object 3: Page (A4 in points: 595×842)
 *   Object 4: Content stream (draw image)
 *   Object 5: Image XObject (JPEG stream)
 *
 * Returns IMG_SUCCESS or IMG_ERR_IO.
 */
img_result_t img_encode_pdf(const img_buffer_t *buf, const char *output_path, uint32_t dpi) {
    if (!buf || !output_path || !buf->data)
        return IMG_ERR_SECURITY;

    /* Compress image to JPEG first */
    tjhandle tj = tjInitCompress();
    if (!tj)
        return IMG_ERR_NOMEM;

    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;

    int rc =
        tjCompress2(tj, buf->data, (int)buf->width, (int)buf->stride, (int)buf->height, TJPF_RGB,
                    &jpeg_buf, &jpeg_size, TJSAMP_444, 92, /* quality — high for print */
                    TJFLAG_FASTDCT);

    tjDestroy(tj);

    if (rc != 0 || !jpeg_buf)
        return IMG_ERR_IO;

    FILE *f = fopen(output_path, "wb");
    if (!f) {
        tjFree(jpeg_buf);
        return IMG_ERR_IO;
    }

    /*
     * A4 page in PDF points (1 point = 1/72 inch).
     * At `dpi` DPI: pixel → point = pixel * 72 / dpi
     */
    float pt_w = (float)buf->width * 72.0f / (float)dpi;
    float pt_h = (float)buf->height * 72.0f / (float)dpi;

    long offsets[8] = {0};
    int obj = 1;

    /* PDF header */
    fprintf(f, "%%PDF-1.4\n%%\xe2\xe3\xcf\xd3\n");

    /* Object 1: Catalog */
    offsets[obj] = ftell(f);
    fprintf(f, "%d 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n", obj++);

    /* Object 2: Pages */
    offsets[obj] = ftell(f);
    fprintf(f, "%d 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n", obj++);

    /* Object 3: Page */
    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Type /Page /Parent 2 0 R\n"
            "   /MediaBox [0 0 %.2f %.2f]\n"
            "   /Contents 4 0 R\n"
            "   /Resources << /XObject << /Im0 5 0 R >> >> >>\n"
            "endobj\n",
            obj++, pt_w, pt_h);

    /* Object 4: Content stream */
    char content[256];
    int content_len =
        snprintf(content, sizeof(content), "q\n%.2f 0 0 %.2f 0 0 cm\n/Im0 Do\nQ\n", pt_w, pt_h);

    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Length %d >>\n"
            "stream\n"
            "%s"
            "endstream\n"
            "endobj\n",
            obj++, content_len, content);

    /* Object 5: JPEG Image XObject */
    offsets[obj] = ftell(f);
    fprintf(f,
            "%d 0 obj\n"
            "<< /Type /XObject /Subtype /Image\n"
            "   /Width %u /Height %u\n"
            "   /ColorSpace /DeviceRGB\n"
            "   /BitsPerComponent 8\n"
            "   /Filter /DCTDecode\n"
            "   /Length %lu >>\n"
            "stream\n",
            obj++, buf->width, buf->height, jpeg_size);

    fwrite(jpeg_buf, 1, jpeg_size, f);
    tjFree(jpeg_buf);

    fprintf(f, "\nendstream\nendobj\n");

    /* xref table */
    long xref_offset = ftell(f);
    fprintf(f, "xref\n0 %d\n0000000000 65535 f \n", obj);

    for (int i = 1; i < obj; i++)
        fprintf(f, "%010ld 00000 n \n", offsets[i]);

    /* trailer */
    fprintf(f,
            "trailer\n<< /Size %d /Root 1 0 R >>\n"
            "startxref\n%ld\n%%%%EOF\n",
            obj, xref_offset);

    fclose(f);
    return IMG_SUCCESS;
}
