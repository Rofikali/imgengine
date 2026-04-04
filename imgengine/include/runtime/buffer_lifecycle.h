// include/runtime/buffer_lifecycle.h

#ifndef IMGENGINE_BUFFER_LIFECYCLE_H
#define IMGENGINE_BUFFER_LIFECYCLE_H

#include <stdint.h>

typedef struct img_buffer img_buffer_t;

// 🔥 Ownership model
typedef enum
{
    IMG_BUF_OWNED = 0,
    IMG_BUF_SHARED = 1,
    IMG_BUF_EXTERNAL = 2
} img_buf_owner_t;

typedef struct
{
    img_buf_owner_t owner;
    uint32_t ref_count;
} img_buf_meta_t;

// Attach metadata
void img_buffer_acquire(img_buffer_t *buf);
void img_buffer_release(img_buffer_t *buf);

#endif