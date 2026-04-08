// #include "pipeline/dispatch/jump_table.h"
// #include <string.h>

// img_kernel_fn g_jump_table[IMG_MAX_OPS];

// void img_register_op(uint32_t opcode, img_kernel_fn fn)
// {
//     if (opcode < IMG_MAX_OPS)
//         g_jump_table[opcode] = fn;
// }

// void img_jump_table_init(cpu_caps_t caps)
// {
//     memset(g_jump_table, 0, sizeof(g_jump_table));

//     /*
//      * 🔥 Register baseline kernels first
//      */
//     extern void img_register_baseline_kernels(void);
//     img_register_baseline_kernels();

//     /*
//      * 🔥 Override with SIMD versions
//      */
//     if (caps.avx512)
//     {
//         extern void img_register_avx512_kernels(void);
//         img_register_avx512_kernels();
//     }
//     else if (caps.avx2)
//     {
//         extern void img_register_avx2_kernels(void);
//         img_register_avx2_kernels();
//     }
// }

#include "pipeline/dispatch/jump_table.h"
#include <string.h>

img_kernel_fn g_jump_table[IMG_MAX_OPS];

/*
 * 🔥 DEFAULT (SAFE NO-OP)
 */
static void noop(img_ctx_t *ctx, img_buffer_t *buf, void *p)
{
    (void)ctx;
    (void)buf;
    (void)p;
}

void img_jump_table_init(void)
{
    for (int i = 0; i < IMG_MAX_OPS; i++)
        g_jump_table[i] = noop;
}

void img_register_op(uint32_t op_code, img_kernel_fn fn)
{
    if (op_code >= IMG_MAX_OPS)
        return;

    g_jump_table[op_code] = fn ? fn : noop;
}