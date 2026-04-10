// ./include/imgengine/io.h

#ifndef IMG_IO_H
#define IMG_IO_H

#include "common.h" // 6-year tip: Always include your visibility macros
#include "image.h"

IE_API int img_load(const char *path, img_t *img);
IE_API int img_save_png(const char *path, const img_t *img);
IE_API int img_save_pdf(const char *filename, const img_t *img);

#endif
