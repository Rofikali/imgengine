
/* api/v1/img_error.h */

#ifndef IMGENGINE_API_ERROR_H
#define IMGENGINE_API_ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        IMG_SUCCESS = 0,

        // Memory / Resource
        IMG_ERR_NOMEM,

        // Input Issues
        IMG_ERR_FORMAT,
        IMG_ERR_SECURITY,

        // Runtime
        IMG_ERR_IO,
        IMG_ERR_HW_UNSUP,

        // Unknown
        IMG_ERR_INTERNAL

    } img_result_t;

#ifdef __cplusplus
}
#endif

#endif
