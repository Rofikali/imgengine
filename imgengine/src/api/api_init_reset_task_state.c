// ./src/api/api_init_reset_task_state.c
#include "api/api_init_internal.h"

#include <string.h>

void img_api_init_reset_task_state(void) { memset(&g_task_queue, 0, sizeof(g_task_queue)); }
