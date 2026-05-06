// ./src/pipeline/jump_table_init.c
#include "pipeline/jump_table.h"
#include "pipeline/jump_table_init_internal.h"

void img_jump_table_init(cpu_caps_t caps) {
    img_jump_table_reset();
    img_jump_table_select_resize(caps);
}
