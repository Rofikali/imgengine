// ./src/api/api_process_raw_free_buffer.c
#include "api/api_process_raw_internal.h"
#include "runtime/job_exec.h"

void img_api_process_raw_free_buffer(img_engine_t *engine, img_buffer_t *buf) {
    img_runtime_release_raw_buffer(engine, buf);
}
