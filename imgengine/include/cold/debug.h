
/* src/cold/debug.h */
#ifndef IMGENGINE_COLD_DEBUG_H
#define IMGENGINE_COLD_DEBUG_H

#include <stdint.h>

// forward decl
typedef struct img_ctx img_ctx_t;

// 🔥 debug levels
typedef enum
{
    IMG_DEBUG_NONE = 0,
    IMG_DEBUG_BASIC,
    IMG_DEBUG_VERBOSE
} img_debug_level_t;

// 🔥 runtime control
void img_debug_set_level(img_debug_level_t lvl);

// 🔥 debug APIs
void img_debug_dump_ctx(const img_ctx_t *ctx);
void img_debug_dump_slab_usage(const img_ctx_t *ctx);
void img_debug_print_cpu_info(void);

#endif