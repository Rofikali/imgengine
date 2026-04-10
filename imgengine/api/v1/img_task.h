// api/v1/img_task.h

#ifndef IMGENGINE_TASK_H
#define IMGENGINE_TASK_H

#include "img_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct img_task {
    img_buffer_t *input;
    img_buffer_t *output;

    void *user_data; // 🔥 for FastAPI mapping
    int status;
} img_task_t;

#ifdef __cplusplus
}
#endif

#endif