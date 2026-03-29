// // plugins/plugin.h

// #ifndef IMG_PLUGIN_H
// #define IMG_PLUGIN_H

// #include "imgengine/image.h"
// #include "imgengine/context.h"
// #include "imgengine/api.h"

// typedef struct img_plugin
// {
//     const char *name;

//     // return 1 = success, 0 = fail
//     int (*process)(img_ctx_t *ctx,
//                    img_t *canvas,
//                    const img_job_t *job);

// } img_plugin_t;

// #endif

#ifndef IMG_PLUGIN_H
#define IMG_PLUGIN_H

#include "imgengine/image.h"
#include "imgengine/api.h"

/* Forward declare context to avoid circular includes */
typedef struct img_ctx img_ctx_t;

typedef struct img_plugin {
    const char *name;
    /* Kernel-style function pointers */
    int (*should_run)(const img_job_t *job);
    int (*process)(img_ctx_t *ctx, img_t *canvas, const img_job_t *job);
} img_plugin_t;

#endif


// #ifndef IMG_PLUGIN_H
// #define IMG_PLUGIN_H

// #include "imgengine/image.h"
// #include "imgengine/context.h"
// #include "imgengine/api.h"

// typedef struct img_plugin
// {
//     const char *name;
//     // Senior move: Check if plugin is needed before entering the loop
//     int (*should_run)(const img_job_t *job);
//     int (*process)(img_ctx_t *ctx, img_t *canvas, const img_job_t *job);
// } img_plugin_t;

// #endif
