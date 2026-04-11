
// api/v1/img_error.h  — FINAL VERSION

#ifndef IMGENGINE_API_ERROR_H
#define IMGENGINE_API_ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        IMG_SUCCESS = 0,
        IMG_ERR_NOMEM = 1,    /* out of memory / slab exhausted        */
        IMG_ERR_FORMAT = 2,   /* malformed or unsupported image format  */
        IMG_ERR_SECURITY = 3, /* input failed security validation       */
        IMG_ERR_IO = 4,       /* I/O read or write failure              */
        IMG_ERR_HW_UNSUP = 5, /* required SIMD not available            */
        IMG_ERR_INTERNAL = 6, /* internal engine error (bug)            */
    } img_result_t;

    /*
     * img_result_name()
     *
     * Returns a human-readable string for an error code.
     * Kernel pattern: printk uses this style for every subsystem.
     * Never returns NULL.
     */
    static inline const char *img_result_name(img_result_t r)
    {
        switch (r)
        {
        case IMG_SUCCESS:
            return "IMG_SUCCESS";
        case IMG_ERR_NOMEM:
            return "IMG_ERR_NOMEM";
        case IMG_ERR_FORMAT:
            return "IMG_ERR_FORMAT";
        case IMG_ERR_SECURITY:
            return "IMG_ERR_SECURITY";
        case IMG_ERR_IO:
            return "IMG_ERR_IO";
        case IMG_ERR_HW_UNSUP:
            return "IMG_ERR_HW_UNSUP";
        case IMG_ERR_INTERNAL:
            return "IMG_ERR_INTERNAL";
        default:
            return "IMG_ERR_UNKNOWN";
        }
    }

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_ERROR_H */