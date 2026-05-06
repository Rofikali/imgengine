// ./src/pipeline/jump_table_register.c
#include "pipeline/jump_table.h"

img_kernel_fn g_jump_table[IMG_MAX_OPS];
img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS];

void img_register_op(uint32_t opcode, img_op_fn single_fn, img_batch_op_fn batch_fn) {
    if (opcode >= IMG_MAX_OPS)
        return;

    if (single_fn)
        g_jump_table[opcode] = single_fn;

    if (batch_fn)
        g_batch_jump_table[opcode] = batch_fn;
}
