// api/v1/img_types.h

#ifndef IMGENGINE_TYPES_H
#define IMGENGINE_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 🔥 OPAQUE TYPES (KERNEL STYLE) */
typedef struct img_engine img_engine_t;
typedef struct img_pipeline img_pipeline_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_task img_task_t;

/* 🔥 SCALAR TYPES */
typedef uint32_t img_op_t;

/* 🔥 SCALE MODE */
typedef enum { IMG_SCALE_FIT = 0, IMG_SCALE_FILL = 1 } img_scale_mode_t;

#ifdef __cplusplus
}
#endif

#endif