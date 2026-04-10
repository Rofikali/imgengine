#ifndef IMGENGINE_JUMP_TABLE_H
#define IMGENGINE_JUMP_TABLE_H

#include "pipeline/kernel_types.h"
#include <stdint.h>

#define IMG_MAX_OPS 256

/*
 * 🔥 SINGLE TABLE (MODE DECIDED AT COMPILE)
 */
extern img_kernel_fn g_jump_table[IMG_MAX_OPS];

void img_register_op(uint32_t opcode, img_kernel_fn fn);

void img_jump_table_init(void);

#endif