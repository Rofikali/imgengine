// include/imgengine/api.h

#ifndef IMG_API_H
#define IMG_API_H

#include <stdint.h>
#include <stddef.h>
#include "common.h" // <--- CLEANER

// #if defined(_WIN32)
// #define IE_API __declspec(dllexport)
// #else
// #define IE_API __attribute__((visibility("default")))
// #endif

/* Opaque Handle: Keeps internal struct changes from breaking external apps */
typedef struct img_ctx img_ctx_t;

typedef enum
{
    IMG_OK = 0,
    IMG_ERR_ALLOC = 1,
    IMG_ERR_IO = 2,
    IMG_ERR_INVALID = 3
} img_status_t;

typedef enum
{
    IMG_FIT,
    IMG_FILL
} img_scale_mode_t;

typedef struct img_job
{
    float photo_w_cm;
    float photo_h_cm;
    int dpi;
    int bleed_px;
    int crop_mark_px;
    int crop_thickness;
    int crop_offset_px;
    int cols;
    int rows;
    int gap;
    int border_px;
    int padding;
    img_scale_mode_t mode;
    unsigned char bg_r;
    unsigned char bg_g;
    unsigned char bg_b;
} img_job_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /* --- Lifecycle Management --- */
    IE_API img_status_t imgengine_create(img_ctx_t **out_ctx, size_t pool_size);
    IE_API void imgengine_destroy(img_ctx_t *ctx);

    /* --- The Main Pipeline --- */
    IE_API img_status_t imgengine_run(
        img_ctx_t *ctx,
        const char *input,
        const char *output,
        const img_job_t *job);

#ifdef __cplusplus
}
#endif

#endif
