// ./include/cold/error.h

/* src/cold/error.h */

#ifndef IMGENGINE_COLD_ERROR_H
#define IMGENGINE_COLD_ERROR_H

#include "api/v1/img_error.h"

// 🔥 branch hint
#if defined(__GNUC__)
#define IMG_LIKELY(x) __builtin_expect(!!(x), 1)
#define IMG_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define IMG_LIKELY(x) (x)
#define IMG_UNLIKELY(x) (x)
#endif

const char *img_error_to_string(img_result_t res);

// 🔥 structured logging
void img_log_error(
    img_result_t res,
    const char *func,
    const char *msg);

#endif