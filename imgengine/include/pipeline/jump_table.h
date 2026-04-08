/* pipeline/jump_table.h */
#ifndef IMGENGINE_JUMP_TABLE_H
#define IMGENGINE_JUMP_TABLE_H

#include <stdint.h>
#include "pipeline/kernel_adapter.h"

#define IMG_MAX_OPS 256

extern img_kernel_fn g_jump_table[IMG_MAX_OPS];
extern img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS];

void img_jump_table_init(void);

#endif
