// ./include/plugins/plugin_resize.h

// ================================================================
// FILE: include/plugins/plugin_resize.h  (UPDATE)
// Now includes from arch/ instead of defining its own.
// ================================================================

#ifndef IMGENGINE_PLUGIN_RESIZE_H
#define IMGENGINE_PLUGIN_RESIZE_H

/*
 * resize_params_t is an arch-level type.
 * Plugins that implement resize use it from arch/.
 */
#include "arch/resize_params.h"

/* Forward declarations only — no heavy deps */
typedef struct img_buffer img_buffer_t;
typedef struct img_ctx img_ctx_t;

#endif /* IMGENGINE_PLUGIN_RESIZE_H */
