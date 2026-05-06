// ./include/core/result.h
// include/core/result.h

#ifndef IMGENGINE_CORE_RESULT_H
#define IMGENGINE_CORE_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * img_result_t — canonical error type for imgengine.
 *
 * Lives in core/ because every layer needs it.
 * core/ has no dependencies — safe to include anywhere.
 *
 * Previously in api/v1/img_error.h — that caused pipeline,
 * security, and memory to depend upward on api/.
 */
typedef enum {
    IMG_SUCCESS = 0,
    IMG_ERR_NOMEM = 1,
    IMG_ERR_FORMAT = 2,
    IMG_ERR_SECURITY = 3,
    IMG_ERR_IO = 4,
    IMG_ERR_HW_UNSUP = 5,
    IMG_ERR_INTERNAL = 6,
} img_result_t;

static inline const char *img_result_name(img_result_t r) {
    switch (r) {
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

#endif /* IMGENGINE_CORE_RESULT_H */