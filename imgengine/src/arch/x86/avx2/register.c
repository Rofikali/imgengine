#include "pipeline/dispatch/jump_table.h"

extern void resize_avx2(img_ctx_t *, img_buffer_t *, void *);

void img_register_avx2_kernels(void)
{
    // example opcode 1 = resize
    img_register_op(1, resize_avx2);
}