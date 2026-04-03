
// src/cold/error.c

#include "cold/error.h"

const char *img_error_to_string(img_result_t r)
{
    switch (r)
    {
    case IMG_SUCCESS:
        return "Success";
    case IMG_ERR_NOMEM:
        return "Out of Memory";
    case IMG_ERR_FORMAT:
        return "Invalid Format";
    case IMG_ERR_SECURITY:
        return "Security Violation";
    default:
        return "Unknown";
    }
}