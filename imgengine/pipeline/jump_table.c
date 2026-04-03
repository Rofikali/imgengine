/* pipeline/jump_table.c */

#include "pipeline/jump_table.h"
#include <string.h>
#include <stdlib.h>

// Global template
img_jump_table_t g_jump_table_global = {0};

// Thread-local pointer
__thread img_jump_table_t *g_jump_table_local = NULL;

void img_pipeline_register_op(uint32_t op_code,
                              img_op_fn single_fn,
                              img_batch_op_fn batch_fn)
{
    if (__builtin_expect(op_code < MAX_OPCODES, 1))
    {
        g_jump_table_global.ops[op_code] = single_fn;
        g_jump_table_global.batch_ops[op_code] = batch_fn;
    }
}

void img_pipeline_bind_thread(void)
{
    // Allocate per-thread jump table
    g_jump_table_local = aligned_alloc(CACHE_LINE, sizeof(img_jump_table_t));

    if (!g_jump_table_local)
        return;

    // Copy global → local (warm L1)
    memcpy(g_jump_table_local, &g_jump_table_global, sizeof(img_jump_table_t));
}