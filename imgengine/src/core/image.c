// // src/core/image.c

// #include "imgengine/image.h"

// #include "stb/stb_image.h"
// #include "stb/stb_image_write.h"

// #include <stdio.h>

// int img_load(const char *path, img_t *img)
// {
//     printf("Loading image: %s\n", path);

//     img->data = stbi_load(path, &img->width, &img->height, &img->channels, 3);

//     if (!img->data)
//     {
//         printf("❌ stbi_load failed: %s\n", stbi_failure_reason());
//         return 0;
//     }

//     img->channels = 3;
//     return 1;
// }

// int img_save_png(const char *path, const img_t *img)
// {
//     return stbi_write_png(path, img->width, img->height, img->channels,
//                           img->data, img->width * img->channels);
// }

// int img_create(img_t *img, mem_pool_t *mp, int w, int h, int ch)
// {

//     size_t size = (size_t)w * h * ch;

//     unsigned char *data = (unsigned char *)mp_alloc(mp, size);
//     if (!data)
//         return 0;

//     img->width = w;
//     img->height = h;
//     img->channels = ch;
//     img->data = data;

//     return 1;
// }

#include "imgengine/image.h"
#include "stb/stb_image.h"
#include <string.h>

int img_create(img_t *img, mem_pool_t *mp, int w, int h, int ch)
{
    size_t size = (size_t)w * h * ch;
    /* mp_alloc handles the MEM_ALIGN (32 bytes) for AVX2 safety */
    unsigned char *data = (unsigned char *)mp_alloc(mp, size);
    if (!data)
        return 0;

    img->width = w;
    img->height = h;
    img->channels = ch;
    img->data = data;
    return 1;
}

int img_load_to_pool(const char *path, img_t *img, mem_pool_t *mp)
{
    int w, h, ch;
    /* stbi_load uses heap; we copy it to our Pool to prevent fragmentation */
    unsigned char *temp = stbi_load(path, &w, &h, &ch, 3);
    if (!temp)
        return 0;

    if (!img_create(img, mp, w, h, 3))
    {
        stbi_image_free(temp);
        return 0;
    }

    memcpy(img->data, temp, (size_t)w * h * 3);
    stbi_image_free(temp);
    return 1;
}
