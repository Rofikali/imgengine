// api/v1/img_pipeline.h

#ifndef IMGENGINE_PIPELINE_H
#define IMGENGINE_PIPELINE_H

#include "img_types.h"

#ifdef __cplusplus
extern "C" {
#endif

img_pipeline_t *img_pipeline_create(void);
void img_pipeline_destroy(img_pipeline_t *p);

/* 🔥 ADD OPERATION */
int img_pipeline_add(img_pipeline_t *p, img_op_t op, const void *params);

/* 🔥 COMPILE → JUMP TABLE */
int img_pipeline_compile(img_pipeline_t *p);

#ifdef __cplusplus
}
#endif

#endif