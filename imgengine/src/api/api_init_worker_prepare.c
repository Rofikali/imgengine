// ./src/api/api_init_worker_prepare.c
#include "api/api_init_internal.h"
#include "runtime/worker.h"

int img_api_init_prepare_worker(uint32_t worker_id) {
    g_worker_ctxs[worker_id].thread_id = worker_id;
    g_worker_ctxs[worker_id].caps = g_engine.caps;
    g_worker_ctxs[worker_id].local_pool = g_engine.global_pool;

    return img_worker_init(&g_workers[worker_id], worker_id, NULL, &g_worker_ctxs[worker_id]);
}
