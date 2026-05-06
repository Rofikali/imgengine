// ./src/api/api_init_task_queue.c
#include "api/api_init_internal.h"

int img_api_init_prepare_task_queue(void) { return img_mpmc_init(&g_task_queue, 1024); }
