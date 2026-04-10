
// src/cold/error.c
#include "cold/error.h"
#include <stdio.h>

const char *img_error_to_string(img_result_t r)
{
    switch (r)
    {
    case IMG_SUCCESS:
        return "SUCCESS";
    case IMG_ERR_NOMEM:
        return "NOMEM";
    case IMG_ERR_FORMAT:
        return "FORMAT";
    case IMG_ERR_SECURITY:
        return "SECURITY";
    default:
        return "UNKNOWN";
    }
}

void img_log_error(
    img_result_t res,
    const char *func,
    const char *msg)
{
    if (IMG_LIKELY(res == IMG_SUCCESS))
        return;

    fprintf(stderr,
            "[ERROR] func=%s code=%s msg=%s\n",
            func,
            img_error_to_string(res),
            msg ? msg : "none");
}