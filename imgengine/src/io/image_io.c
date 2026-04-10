// ./src/io/image_io.c

#include "imgengine/io.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <stdio.h>

IE_API int img_load(const char *path, img_t *img) {
    // 6-Year Tip: Standardize channel count to 3 (RGB) for the whole pipeline
    img->data = stbi_load(path, &img->width, &img->height, &img->channels, 3);
    if (!img->data) {
        fprintf(stderr, "❌ [IO] Failed to load %s: %s\n", path, stbi_failure_reason());
        return 0;
    }
    img->channels = 3;
    return 1;
}

IE_API int img_save_png(const char *path, const img_t *img) {
    if (!img || !img->data)
        return 0;
    return stbi_write_png(path, img->width, img->height, img->channels, img->data,
                          img->width * img->channels);
}
