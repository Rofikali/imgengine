// /* pipeline/jump_table.h */

#ifndef IMGENGINE_JUMP_TABLE_H
#define IMGENGINE_JUMP_TABLE_H

#include <stdint.h>
#include "core/context_internal.h"
#include "api/v1/img_types.h"
#include "arch/cpu_caps.h"
#include "pipeline/batch.h"

// ================= FUNCTION TYPES =================

typedef void (*img_op_fn)(img_ctx_t *, img_buffer_t *, void *);
typedef void (*img_batch_op_fn)(img_ctx_t *, void *, void *);

// ================= CONSTANTS =================

#define IMG_MAX_OPS 256

// ================= GLOBAL TABLES =================

extern img_op_fn g_jump_table[IMG_MAX_OPS];
extern img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS];

// ================= API =================

// Register operation
void img_register_op(uint32_t opcode, img_op_fn fn, img_batch_op_fn batch_fn);

// CPU-aware init (MAIN ENTRY)
void img_jump_table_init(cpu_caps_t caps);

#endif