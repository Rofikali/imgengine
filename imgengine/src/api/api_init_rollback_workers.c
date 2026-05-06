// ./src/api/api_init_rollback_workers.c
#include "api/api_init_internal.h"
#include "runtime/worker.h"

void img_api_init_rollback_workers(uint32_t initialized_workers) {
    for (uint32_t i = 0; i < initialized_workers; i++) {
        img_worker_stop(&g_workers[i]);
        img_worker_join(&g_workers[i]);
        img_queue_destroy(g_workers[i].queue);
        g_workers[i].queue = NULL;
    }
}
