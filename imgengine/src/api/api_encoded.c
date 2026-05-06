// ./src/api/api_encoded.c
#include "api/v1/img_api.h"
#include "api/api_internal.h"
#include "runtime/job_exec.h"

#include <stdlib.h>

void img_api_release_raw_buffer(img_engine_t *engine, img_buffer_t *buf) {
    img_runtime_release_raw_buffer(engine, buf);
}

void img_encoded_free(uint8_t *ptr) {
    if (ptr)
        free(ptr);
}
