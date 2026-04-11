// ./include/plugins/plugin_internal_removed.h

// plugins/plugin_inhernal.h

#ifndef IMGENGINE_PLUGINS_INTERNAL_H
#define IMGENGINE_PLUGINS_INTERNAL_H

#include <stdint.h>

// 🔥 Forward declarations ONLY
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_batch img_batch_t;

// Function pointer types
typedef void (*img_op_fn)(img_ctx_t *, img_buffer_t *, void *);
typedef void (*img_batch_op_fn)(img_ctx_t *, img_batch_t *, void *);

// Plugin descriptor
typedef struct
{
    uint32_t op_code;
    const char *name;
    img_op_fn single_exec;
    img_batch_op_fn batch_exec;
} img_plugin_info_t;

// Init
void img_plugins_init_all(void);

#endif
