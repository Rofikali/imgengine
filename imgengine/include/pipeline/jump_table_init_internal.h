// ./include/pipeline/jump_table_init_internal.h
#ifndef IMGENGINE_PIPELINE_JUMP_TABLE_INIT_INTERNAL_H
#define IMGENGINE_PIPELINE_JUMP_TABLE_INIT_INTERNAL_H

#include "arch/cpu_caps.h"

void img_jump_table_reset(void);
void img_jump_table_select_resize_avx512(void);
void img_jump_table_select_resize_avx2(void);
void img_jump_table_select_resize_scalar(void);
void img_jump_table_select_resize(cpu_caps_t caps);

#endif /* IMGENGINE_PIPELINE_JUMP_TABLE_INIT_INTERNAL_H */
