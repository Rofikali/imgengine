// pipeline/kernel_types.h  🔥 NEW CORE FILE

#ifndef IMGENGINE_KERNEL_TYPES_H
#define IMGENGINE_KERNEL_TYPES_H

#include <stdint.h>

/* Forward declarations ONLY */
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_batch img_batch_t;

/*
 * 🔥 UNIFIED KERNEL ABI (L10 RULE)
 */
typedef void (*img_kernel_fn)(img_ctx_t *ctx,
                              void *restrict data, // buffer OR batch
                              void *restrict params);

/*
 * 🔥 KERNEL MODE
 */
typedef enum { IMG_KERNEL_SINGLE = 0, IMG_KERNEL_BATCH = 1 } img_kernel_mode_t;

#endif