// ./include/imgengine/common.h


#ifndef IMG_COMMON_H
#define IMG_COMMON_H

#if defined(_WIN32)
    #define IE_API __declspec(dllexport)
#else
    #define IE_API __attribute__((visibility("default")))
#endif

#endif
