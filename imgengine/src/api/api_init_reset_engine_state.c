// ./src/api/api_init_reset_engine_state.c
#include "api/api_init_internal.h"

#include <string.h>

void img_api_init_reset_engine_state(void) { memset(&g_engine, 0, sizeof(g_engine)); }
