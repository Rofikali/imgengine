// // api/v1/img_api.h

// #ifndef IMGENGINE_API_H
// #define IMGENGINE_API_H

// #include "img_error.h"
// #include "img_types.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// /* 🔥 ENGINE LIFECYCLE */
// img_engine_t *img_engine_create(uint32_t workers, size_t memory_pool);
// void img_engine_destroy(img_engine_t *engine);

// /* 🔥 TASK SUBMISSION (NON-BLOCKING) */
// int img_submit(img_engine_t *engine, img_pipeline_t *pipeline, img_task_t *task);

// /* 🔥 COMPLETION (OPTIONAL POLL MODEL) */
// int img_poll(img_engine_t *engine, img_task_t **completed_task);

// #ifdef __cplusplus
// }
// #endif

// #endif

// api/v1/img_api.h

#ifndef IMGENGINE_API_H
#define IMGENGINE_API_H

#include "img_types.h"

#ifdef __cplusplus
extern "C" {
#endif

img_engine_t *img_engine_create(uint32_t workers, size_t mem);
void img_engine_destroy(img_engine_t *e);

/* 🔥 submission model */
int img_submit(img_engine_t *e, img_pipeline_t *p, img_task_t *t);

int img_wait(img_engine_t *e, img_task_t *t);

#ifdef __cplusplus
}
#endif

#endif