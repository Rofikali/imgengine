// api/v1/img_buffer.h

#ifndef IMGENGINE_BUFFER_H
#define IMGENGINE_BUFFER_H

#include "img_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 🔥 ACCESSORS ONLY (NO STRUCT) */

uint8_t *img_buffer_data(img_buffer_t *buf);
uint32_t img_buffer_width(const img_buffer_t *buf);
uint32_t img_buffer_height(const img_buffer_t *buf);
uint32_t img_buffer_stride(const img_buffer_t *buf);

/* 🔥 REFCOUNT CONTROL */
void img_buffer_retain(img_buffer_t *buf);
void img_buffer_release(img_buffer_t *buf);

#ifdef __cplusplus
}
#endif

#endif