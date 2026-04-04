/* api/v1/img_types.h */

#ifndef IMGENGINE_API_TYPES_H
#define IMGENGINE_API_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // 🔥 Core buffer (ONLY place with full definition)
    typedef struct img_buffer
    {
        uint32_t width;
        uint32_t height;
        uint32_t channels;
        uint32_t stride;
        uint8_t *data;
    } img_buffer_t;

    // Operation descriptor
    typedef struct
    {
        uint32_t op_code;
        void *params;
    } img_op_desc_t;

#define IMG_MAX_PIPELINE_OPS 16

    typedef struct
    {
        img_op_desc_t ops[IMG_MAX_PIPELINE_OPS];
        uint32_t count;
    } img_pipeline_desc_t;

#ifdef __cplusplus
}
#endif

#endif
