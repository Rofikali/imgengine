// ./src/pipeline/jump_table_select_resize_avx2.c
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

extern void resize_avx2(img_ctx_t *, img_buffer_t *, void *);
extern void img_batch_resize_fused_avx2(img_ctx_t *, img_batch_t *, void *);

void img_jump_table_select_resize_avx2(void) {
    g_jump_table[OP_RESIZE] = resize_avx2;
    g_batch_jump_table[OP_RESIZE] = img_batch_resize_fused_avx2;
}
