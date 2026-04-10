// ./include/imgengine/error.h


#ifndef IMG_ERROR_H
#define IMG_ERROR_H

typedef enum
{
    IMG_OK = 0,
    IMG_ERR_ALLOC,
    IMG_ERR_IO,
    IMG_ERR_INVALID
} img_err_t;

#endif