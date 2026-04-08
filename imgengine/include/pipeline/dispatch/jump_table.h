#ifndef IMGENGINE_JUMP_TABLE_H
#define IMGENGINE_JUMP_TABLE_H

#include <stdint.h>

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

typedef void (*img_kernel_fn)(
    img_ctx_t *,
    img_buffer_t *,
    void *params);

/*
 * 🔥 MAX OPS
 */
#define IMG_MAX_OPS 256

extern img_kernel_fn g_jump_table[IMG_MAX_OPS];

/*
 * 🔥 REGISTRATION
 */
void img_register_op(
    uint32_t op_code,
    img_kernel_fn fn);

/*
 * 🔥 INIT
 */
void img_jump_table_init(void);

#endif