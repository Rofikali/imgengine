/* pipeline/jump_table.c */

#include "pipeline/jump_table.h"
#include <string.h>

img_kernel_fn g_jump_table[IMG_MAX_OPS];
img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS];

void img_jump_table_init(void)
{
    memset(g_jump_table, 0, sizeof(g_jump_table));
    memset(g_batch_jump_table, 0, sizeof(g_batch_jump_table));
}

void img_register_op(
    uint32_t opcode,
    img_kernel_fn fn,
    img_batch_kernel_fn batch_fn)
{
    if (opcode >= IMG_MAX_OPS)
        return;

    if (fn)
        g_jump_table[opcode] = fn;

    if (batch_fn)
        g_batch_jump_table[opcode] = batch_fn;
}