
// pi/v1/img_pipeline.h

#ifndef IMGENGINE_API_PIPELINE_H
#define IMGENGINE_API_PIPELINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct img_pipeline img_pipeline_t;

    img_pipeline_t *img_pipeline_create(void);
    void img_pipeline_destroy(img_pipeline_t *pipe);

    int img_pipeline_add_op(
        img_pipeline_t *pipe,
        uint32_t op_code,
        void *params);

#ifdef __cplusplus
}
#endif

#endif
