// ./include/io/remote_fetch.h

// include/io/remote_fetch.h

#ifndef IMGENGINE_REMOTE_FETCH_H
#define IMGENGINE_REMOTE_FETCH_H

#include <stddef.h>
#include <stdint.h>

// 🔥 fetch partial (range request)
int img_http_fetch_range(
    const char *url,
    size_t start,
    size_t end,
    uint8_t *buffer,
    size_t *received);

// 🔥 full fetch (fallback)
int img_http_fetch_full(
    const char *url,
    uint8_t **data,
    size_t *size);

#endif