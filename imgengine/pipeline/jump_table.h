/* pipeline/jump_table.h */

#ifndef IMGENGINE_PIPELINE_JUMP_TABLE_H
#define IMGENGINE_PIPELINE_JUMP_TABLE_H

#include "pipeline/pipeline.h"

#define CACHE_LINE 64

typedef struct
{
    img_op_fn ops[MAX_OPCODES];
    img_batch_op_fn batch_ops[MAX_OPCODES];
} img_jump_table_t __attribute__((aligned(CACHE_LINE)));

// 🔥 GLOBAL (fallback)
extern img_jump_table_t g_jump_table_global;

// 🔥 THREAD-LOCAL (L1 HOT)
extern __thread img_jump_table_t *g_jump_table_local;

void img_pipeline_register_op(uint32_t op_code,
                              img_op_fn single_fn,
                              img_batch_op_fn batch_fn);

void img_pipeline_bind_thread(void);

#endif