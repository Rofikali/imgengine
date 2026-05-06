// ./src/pipeline/jump_table_select_resize.c
#include "pipeline/jump_table.h"
#include "pipeline/jump_table_init_internal.h"

void img_jump_table_select_resize(cpu_caps_t caps) {
    if (img_cpu_has_avx512(caps)) {
        img_jump_table_select_resize_avx512();
    } else if (img_cpu_has_avx2(caps)) {
        img_jump_table_select_resize_avx2();
    } else {
        img_jump_table_select_resize_scalar();
    }
}
