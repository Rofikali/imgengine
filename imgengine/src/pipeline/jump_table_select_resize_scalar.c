// ./src/pipeline/jump_table_select_resize_scalar.c
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

extern void resize_scalar(img_ctx_t *, img_buffer_t *, void *);

void img_jump_table_select_resize_scalar(void) { g_jump_table[OP_RESIZE] = resize_scalar; }
