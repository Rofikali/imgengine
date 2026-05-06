// ./src/api/api_state.c
#include "api/api_internal.h"

img_engine_t g_engine;
img_worker_t g_workers[64];
img_ctx_t g_worker_ctxs[64];
img_mpmc_queue_t g_task_queue;
