// ./src/api/api_init_reset_worker_state.c
#include "api/api_init_internal.h"

#include <string.h>

void img_api_init_reset_worker_state(void) {
    memset(g_workers, 0, sizeof(g_workers));
    memset(g_worker_ctxs, 0, sizeof(g_worker_ctxs));
}
