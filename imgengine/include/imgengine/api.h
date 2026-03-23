// // include/imgengine/api.h

// #ifndef IMG_API_H
// #define IMG_API_H

// #include "context.h"

// typedef enum
// {
//     IMG_FIT,
//     IMG_FILL
// } img_scale_mode_t;

// typedef struct
// {
//     float photo_w_cm;
//     float photo_h_cm;
//     int dpi;

//     int cols;
//     int rows;
//     int gap;

//     int border_px;

//     int padding;

//     int auto_fit; // 🔥 ADD THIS LINE

//     img_scale_mode_t mode;

//     unsigned char bg_r;
//     unsigned char bg_g;
//     unsigned char bg_b;

// } img_job_t;

// int imgengine_run(
//     img_ctx_t *ctx,
//     const char *input,
//     const char *output,
//     const img_job_t *job);

// #endif

#ifndef IMG_API_H
#define IMG_API_H

#include "context.h"

typedef enum
{
    IMG_FIT,
    IMG_FILL
} img_scale_mode_t;

typedef struct
{
    float photo_w_cm;
    float photo_h_cm;
    int dpi;

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

int imgengine_run(
    img_ctx_t *ctx,
    const char *input,
    const char *output,
    const img_job_t *job);

#endif