// ./src/pipeline/jump_table_reset.c
#include "pipeline/jump_table.h"

#include <string.h>

void img_jump_table_reset(void) {
    memset(g_jump_table, 0, sizeof(g_jump_table));
    memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));
}
