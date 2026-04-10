
// /* pipeline/jump_table.h */

#ifndef IMGENGINE_JUMP_TABLE_H
#define IMGENGINE_JUMP_TABLE_H

#include <stdint.h>
#include "pipeline/kernel_adapter.h"
#include "arch/cpu_caps.h"

#define IMG_MAX_OPS 256

/*
 * 🔥 HOT PATH TABLES (KERNEL ABI ONLY)
 */
extern img_kernel_fn g_jump_table[IMG_MAX_OPS];
extern img_batch_kernel_fn g_batch_jump_table[IMG_MAX_OPS];

/*
 * 🔥 REGISTER (AUTO ADAPT)
 */
void img_register_op(
    uint32_t opcode,
    img_op_fn single_fn,
    img_batch_op_fn batch_fn);

/*
 * 🔥 INIT
 */
void img_jump_table_init(cpu_caps_t caps);

#endif

// #ifndef IMGENGINE_JUMP_TABLE_H
// #define IMGENGINE_JUMP_TABLE_H

// #include <stdint.h>
// #include "core/context_internal.h"
// #include "arch/cpu_caps.h"
// #include "pipeline/kernel_adapter.h"

// /*
//  * 🔥 LIMITS
//  */
// #define IMG_MAX_OPS 256

// /*
//  * 🔥 INTERNAL KERNEL TABLE (HOT PATH)
//  */
// extern img_kernel_fn g_jump_table[IMG_MAX_OPS];

// /*
//  * 🔥 OPTIONAL BATCH TABLE
//  */
// extern img_batch_op_fn g_batch_jump_table[IMG_MAX_OPS];

// /*
//  * 🔥 REGISTER (PLUGIN → KERNEL ADAPT)
//  */
// void img_register_op(
//     uint32_t opcode,
//     img_op_fn single_fn,
//     img_batch_op_fn batch_fn);

// /*
//  * 🔥 INIT (CPU DISPATCH)
//  */
// void img_jump_table_init(cpu_caps_t caps);

// #endif
