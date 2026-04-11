// ./include/plugins/plugin_crop.h

/* plugins/plugin_crop.h */

#ifndef IMGENGINE_PLUGIN_CROP_H
#define IMGENGINE_PLUGIN_CROP_H

#include <stdint.h>

// 🔥 Forward declarations
typedef struct img_buffer img_buffer_t;
typedef struct img_ctx img_ctx_t;

void plugin_crop_single(img_ctx_t *, img_buffer_t *, void *);

#endif
