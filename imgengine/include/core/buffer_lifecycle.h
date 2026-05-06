// ./include/core/buffer_lifecycle.h
#ifndef IMGENGINE_CORE_BUFFER_LIFECYCLE_H
#define IMGENGINE_CORE_BUFFER_LIFECYCLE_H

#include "core/buffer.h"

void img_buffer_acquire(img_buffer_t *buf);
void img_buffer_release(img_buffer_t *buf);

#endif /* IMGENGINE_CORE_BUFFER_LIFECYCLE_H */
