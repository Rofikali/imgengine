// ./src/api/api_init_rollback.c
#include "api/api_init_internal.h"

void img_api_init_rollback(uint32_t initialized_workers) {
    img_api_init_rollback_workers(initialized_workers);
    img_api_init_reset_engine();
}
